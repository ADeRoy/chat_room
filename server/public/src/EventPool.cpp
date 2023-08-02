#include "EventPool.h"

using namespace DE;

PollMethod EventPoll::curMethod = BCF_POLL_EPOLL;

/**
 *@fn           EventPoll::EventPoll()
 *@brief        EventPoll构造函数
 */
EventPoll::EventPoll()
{
    _method = EventPoll::curMethod;
    _initFlag = false;
    _pollSet = NULL;
}

/**
 *@fn           EventPoll::~EventPoll()
 *@brief        EventPoll析构函数
 */
EventPoll::~EventPoll()
{
    if(_initFlag)
        destory();
}

/**
 *@fn           EventPoll::init(PollMethod method)
 *@brief        初始化轮询事件对象
 *@param[in] method      轮询事件对象所采用的轮询方式，见PollMethod定义
 *
 *@return          见StatusType定义
 */
StatusType EventPoll::init(PollMethod method)
{
  if(_initFlag)
    return ST_REINIT;

  /// 初始化轮询方式
  /// BCF_POLL_DEFAULT按照平台默认方式初始化
  if(method == BCF_POLL_DEFAULT)
  {
    _method = EventPoll::getCurrentPoll(); // 如果指定默认,则使用全局预设置
  }
  // 如果指定方式,则使用现在指定的
  else
  {
    switch(method)
    {
      case BCF_POLL_SELECT:
        _method = BCF_POLL_SELECT;
        break;
      case BCF_POLL_EPOLL:
#ifdef linux
        _method = BCF_POLL_EPOLL;
#else
        return ST_NOTIMPL;
#endif
        break;
      default:
        return ST_NOTIMPL;
        break;
    }
  }

  /// 清空事件数组
  _events.clear();

  /// 初始化方法的私有数据
  if(_method == BCF_POLL_SELECT)
    _pollSet = new PollFdSet;
  else if(_method == BCF_POLL_EPOLL)
    _pollSet = new PollEpollSet;
  else
    return ST_NOTIMPL;

  if (_pollSet->initSet() < 0)
  {
    ChatLog << "_pollSet->initSet() ERROR...";
    return ST_SYSERROR;
  }
  /// 设置初始化标志位
  _initFlag = true;

  return ST_OK;
}

/**
 *@fn           EventPoll::destory()
 *@brief        销毁对象使用的资源，调用该方法后，需要重新初始化
 *
 *@return          见StatusType定义
 */
StatusType EventPoll::destory()
{
    _pollSet->clearSet();
    delete _pollSet;
    _pollSet = NULL;
    _initFlag = false;
    return ST_OK;
}

/**
 *@fn           EventPoll::addEvent(const SocketHandle& sock, unsigned int reqEvents)
 *@brief        添加一个轮询事件
 *@param[in] sock      事件关注的socket句柄
 *@param[in] reqEvents 关注事件类型，可以是多个事件的组合
 * 
 *@return          见StatusType定义
 */
StatusType EventPoll::addEvent(const SocketHandle& sock, unsigned int reqEvents)
{
  if(!_initFlag)
    return ST_UNINIT;

  if(sock <= 0)
    return ST_PARAMERROR;

  if (_pollSet->addFd(sock, reqEvents) < 0)
  {
    ChatLog << "_pollSet->addFd err, fd:" << sock;
    return ST_SYSERROR;
  }
  PollEventType ev;
  ev.sock = sock;
  ev.reqEvents = reqEvents;
  ev.rtEvents = 0;
  _events.push_back(ev);

  return ST_OK;
}

/**
 *@fn           EventPoll::removeEvent(const SocketHandle& sock)
 *@brief        移除一个轮询事件，将移除关注句柄的所有事件
 *@param[in] sock      事件关注的socket句柄
 * 
 *@return          见StatusType定义
 */
StatusType EventPoll::removeEvent(const SocketHandle& sock)
{
  if(!_initFlag)
    return ST_UNINIT;

  if(sock <= 0)
    return ST_PARAMERROR;

  std::list<PollEventType>::iterator i;
  for(i=_events.begin(); i!=_events.end();)
  {
    if(i->sock == sock)
    {
      i = _events.erase(i);
      if(_pollSet->deleteFd(sock) < 0)
        return ST_SYSERROR;
    }
    else
    {
      ++i;
    }
  }

  return ST_OK;
}

/**
 *@fn           EventPoll::removeAllEvent()
 *@brief        移除所有轮询事件，该方法不同于destory()\n
 *              该方法仅将当前关注的事件集合全部清空
 * 
 *@return          见StatusType定义
 */
StatusType EventPoll::removeAllEvent()
{
  if(!_initFlag)
    return ST_UNINIT;

  std::list<PollEventType>::iterator i=_events.begin();
  for(; i!=_events.end();)
  {
    _pollSet->deleteFd(i->sock);
    i = _events.erase(i);
  }

  return ST_OK;
}

/**
 *@fn           EventPoll::poll(PollEventSet& events, int* timeout)
 *@brief        阻塞等待事件
 *@param[out] events      激活的事件数组
 *@param[in] timeout      阻塞等待的时间,毫秒\n
 *                        如该指针为NULL,或值<0,则一直阻塞\n
 *                        如该值=0,则立即返回.
 * 
 *@return          见StatusType定义
 */
StatusType EventPoll::poll(PollEventSet& events, int* timeout)
{
  if(!_initFlag)
    return ST_UNINIT;

  int ret = _pollSet->doSetPoll(_events, events, timeout);
  if(ret == 0)
    return ST_TIMEOUT;
  if(ret < 0)
    return ST_SYSERROR;

  return ST_OK;
}

/**
 *@fn           EventPoll::getEventCount()
 *@brief        返回当前对象中保存的关注事件数量
 * 
 *@return       事件个数
 */
int EventPoll::getEventCount()
{
  return (int)_events.size();
}

/**
*@fn           EventPoll::setCurrentPoll(PollMethod m)
*@brief        设置默认的多路复用方式
*/
void EventPoll::setCurrentPoll(PollMethod m)
{
  EventPoll::curMethod = m; 
}

/**
 *@fn           EventPoll::getCurrentPoll()
 *@brief        获取当前的多路复用方式
 */
PollMethod EventPoll::getCurrentPoll()
{
  return EventPoll::curMethod; 
}



/**
 *@fn           PollFdSet::initSet()
 *@brief        select方式sock集合初始化
 * 
 *@return       成功返回0
 */
int PollFdSet::initSet()
{
  FD_ZERO(&_rset);
  FD_ZERO(&_wset);
  FD_ZERO(&_eset);
  _maxFd = 0;
  return 0;
}

/**
 *@fn           PollFdSet::clearSet()
 *@brief        select方式sock集合清空
 * 
 *@return       成功返回0
 */
int PollFdSet::clearSet()
{
  FD_ZERO(&_rset);
  FD_ZERO(&_wset);
  FD_ZERO(&_eset);
  _maxFd = 0;
  return 0;
}

/**
 *@fn           PollFdSet::addFd(const SocketHandle& sock, unsigned int reqEvents)
 *@brief        select方式根据事件类型添加一个sock句柄
 *@param[in] sock      事件关注的socket句柄
 *@param[in] reqEvents 关注事件类型，可以是多个事件的组合 
 * 
 *@return       成功返回0，sock超出范围返回-1
 */
int PollFdSet::addFd(const SocketHandle& sock, unsigned int reqEvents)
{
  if((int)sock > FD_SETSIZE - 1)
  {
    return -1;
  }

  if(reqEvents & BCF_POLLIN)
    FD_SET(sock, &_rset);
  if(reqEvents & BCF_POLLOUT)
    FD_SET(sock, &_wset);
  if(reqEvents & (BCF_POLLPRI | BCF_POLLERR | BCF_POLLHUP | BCF_POLLNVAL))
    FD_SET(sock, &_eset);

  if((int)sock > _maxFd)
    _maxFd = (int)sock;

  return 0;
}

/**
 *@fn           PollFdSet::deleteFd(const SocketHandle& sock)
 *@brief        select方式删除一个sock句柄
 *@param[in] sock      事件关注的socket句柄
 * 
 *@return       成功返回0
 */
int PollFdSet::deleteFd(const SocketHandle& sock)
{
  FD_CLR(sock, &_rset);
  FD_CLR(sock, &_wset);
  FD_CLR(sock, &_eset);
  return 0;
}

/**
 *@fn           PollFdSet::doSetPoll(std::list<PollEventType>& reqEvents, PollEventSet& rtEvents, int* timeout)
 *@brief        select方式阻塞轮询 
 *@param[in] reqEvents      关注的事件列表 
 *@param[out] rtEvents      激活的事件数组 
 *@param[in] timeout        阻塞等待的时间 
 * 
 *@return       成功返回0，失败返回-1，置errno
 */
int PollFdSet::doSetPoll(std::list<PollEventType>& reqEvents, PollEventSet& rtEvents, int* timeout)
{
  struct timeval* ptv = NULL;
  struct timeval tv;
  if(timeout != NULL && *timeout >= 0)
  {
    tv.tv_sec = *timeout / 1000;
    tv.tv_usec = (*timeout % 1000) * 1000;
    ptv = &tv;
  }

  fd_set readset, writeset, exceptset;
  memcpy(&readset, &_rset, sizeof(fd_set));
  memcpy(&writeset, &_wset, sizeof(fd_set));
  memcpy(&exceptset, &_eset, sizeof(fd_set));

  int ret = select(_maxFd + 1, &readset, &writeset, &exceptset, ptv);
  if(ret <= 0)
  {
    if(timeout != NULL && *timeout > 0)
    {
      *timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
    return ret;
  }

  rtEvents.clear();
  std::list<PollEventType>::iterator iter = reqEvents.begin();
  for(; iter!=reqEvents.end(); ++iter)
  {
    iter->rtEvents = 0;
    if(FD_ISSET(iter->sock, &readset))
      iter->rtEvents |= BCF_POLLIN;
    if(FD_ISSET(iter->sock, &writeset))
      iter->rtEvents |= BCF_POLLOUT;
    if(FD_ISSET(iter->sock, &exceptset))
      iter->rtEvents |= BCF_POLLERR;

    if(iter->rtEvents != 0)
      rtEvents.push_back(*iter);
  }

  if(timeout != NULL && *timeout > 0)
  {
    *timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  }
  return ret;
}





/**
 *@fn           PollEpollSet::initSet()
 *@return       成功返回_epollFd>0,失败返回-1
 *@brief        epoll方式sock集合初始化
 * 
 */
int PollEpollSet::initSet()
{
  _epollFd = epoll_create(1024);
  if ( -1==_epollFd )
  {
    ChatLog << "epoll_create err,errno:"<<errno;
  }
  return _epollFd;
}

/**
 *@fn           PollEpollSet::clearSet()
 *@brief        epoll方式sock集合清空
 * 
 *@return       成功返回0
 */
int PollEpollSet::clearSet()
{
  if(_epollFd != -1)
  {
    close(_epollFd);
    _epollFd = -1;
  }
  return 0;
}

/**
 *@fn           PollEpollSet::addFd(const SocketHandle& sock, unsigned int reqEvents)
 *@brief        epoll方式根据事件类型添加一个sock句柄
 *@param[in] sock      事件关注的socket句柄
 *@param[in] reqEvents 关注事件类型，可以是多个事件的组合 
 * 
 *@return       成功返回0，失败返回-1，置errno
 */
int PollEpollSet::addFd(const SocketHandle& sock, unsigned int reqEvents)
{
  if (sock <=0 )
  {
    ChatLog << "addFd sock <=0.";
    return -1;
  }
  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.data.fd = sock;
  ev.events = getEpollEvent(reqEvents);
  if ( 0 == ev.events)
  {
    ChatLog << "addFd getEpollEvent err..";
    return -1;
  }
  int ret = epoll_ctl(_epollFd, EPOLL_CTL_ADD, sock, &ev);
  if ( -1 == ret )
  {
    ChatLog << "addFd epoll_ctl errno:"<<errno<<",fd:"<<sock<<",reqEvents:"<< ev.events<<",_epollFd:"<< _epollFd;
  }
  return ret;
}

/**
 *@fn           PollEpollSet::deleteFd(const SocketHandle& sock)
 *@brief        epoll方式删除一个sock句柄
 *@param[in] sock      事件关注的socket句柄
 * 
 *@return       成功返回0，失败返回-1，置errno
 */
int PollEpollSet::deleteFd(const SocketHandle& sock)
{
  struct epoll_event ev = {0};
  return epoll_ctl(_epollFd, EPOLL_CTL_DEL, sock, &ev);
}

/**
 *@fn           PollEpollSet::doSetPoll(std::list<PollEventType>& reqEvents, PollEventSet& rtEvents, int* timeout)
 *@brief        epoll方式阻塞轮询 
 *@param[in] reqEvents      关注的事件列表 
 *@param[out] rtEvents      激活的事件数组 
 *@param[in] timeout        阻塞等待的时间,毫秒
 * 
 *@return       成功返回0，失败返回-1，置errno
 *@attention    在linux下,阻塞在epoll_wait()时,如当前进程被其它进程使用strace跟踪,\n
 *              则epoll_wait()会返回,errno为EINTR
 */
int PollEpollSet::doSetPoll(std::list<PollEventType>& reqEvents, PollEventSet& rtEvents, int* timeout)
{
  int tv = -1;
  if(timeout != NULL && *timeout >= 0)
  {
    tv = *timeout;
  }
  int maxevents = (int)reqEvents.size();
  epoll_event epollEvents[maxevents]; // _epollEvents = new struct epoll_event[maxevents];
  _evCount = 0;
  
  TimeDifference t1;
  _evCount = epoll_wait(_epollFd, epollEvents, maxevents, tv);
  if (_evCount < 0 || maxevents <=0 )
  {
    ChatLog << "epoll_wait errno:" << errno << ",epollfd:" << _epollFd << ",_evCount:" << _evCount <<",maxevents:"<< maxevents<<",tv:"<<tv;
  }
  TimeDifference t2;
  if(_evCount > 0)
  {
    rtEvents.clear();
    std::list<PollEventType>::iterator iter = reqEvents.begin();
    for(; iter!=reqEvents.end(); ++iter)
    {
      for(int i=0; i<_evCount; ++i)
      {
        if(iter->sock == epollEvents[i].data.fd)
        {
          iter->rtEvents = getBCFEvent(epollEvents[i].events);
          rtEvents.push_back(*iter);
        }
      }
    }
  }
  
  if(timeout != NULL && *timeout > 0)
  {
    // 如果epoll_wait返回0,则认为超时,置timeout为0
    if(_evCount == 0)
    {
      *timeout = 0;
    }
    else
    {
      *timeout = tv - (t2 - t1);  // 用tv值减去消耗的时间
      if(*timeout < 0)
        *timeout = 0;
    }
  }
  
  return _evCount;
}

/**
 *@fn           PollEpollSet::getEpollEvent(unsigned int event)
 *@brief        将BCF事件类型转换成epoll事件类型
 *@param[in] event        关注的事件类型，可以是多种事件的或集合
 * 
 *@return       返回epoll事件类型集合
 */
unsigned int PollEpollSet::getEpollEvent(unsigned int event)
{
  unsigned int rv = 0;
  if(event & BCF_POLLIN)
    rv |= EPOLLIN;
  if(event & BCF_POLLPRI)
    rv |= EPOLLPRI;
  if(event & BCF_POLLOUT)
    rv |= EPOLLOUT;

  return rv;
}

/**
 *@fn           PollEpollSet::getBCFEvent(unsigned int event)
 *@brief        将BCF事件类型转换成epoll事件类型
 *@param[in] event        关注的事件类型，可以是多种事件的或集合
 * 
 *@return       返回BCF事件类型集合
 */
unsigned int PollEpollSet::getBCFEvent(unsigned int event)
{
  unsigned int rv = 0;
  if(event & EPOLLIN)
    rv |= BCF_POLLIN;
  if(event & EPOLLPRI)
    rv |= BCF_POLLPRI;
  if(event & EPOLLOUT)
    rv |= BCF_POLLOUT;
  if(event & EPOLLERR)
    rv |= BCF_POLLERR;
  if(event & EPOLLHUP)
    rv |= BCF_POLLHUP;

  return rv;
}
