#ifndef _EVENTPOLL_H_
#define _EVENTPOLL_H_

#include <list>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include "common.h"
#include "MySysClock.h"
#include "MyStatusType.h"


namespace DE
{
    #define BCF_POLLIN    0x001     /**< Can read without blocking */
    #define BCF_POLLPRI   0x002     /**< Priority data available */
    #define BCF_POLLOUT   0x004     /**< Can write without blocking */
    #define BCF_POLLERR   0x010     /**< Pending error */
    #define BCF_POLLHUP   0x020     /**< Hangup occurred */
    #define BCF_POLLNVAL  0x040     /**< Descriptor invalid */

    enum PollMethod             /**< 轮询方法定义 */
    {
        BCF_POLL_DEFAULT,        /**< Platform default poll method */
        BCF_POLL_SELECT,         /**< Poll uses select method */
        BCF_POLL_EPOLL,          /**< Poll uses epoll method */
        BCF_POLL_POLL            /**< Poll uses poll method */
    };

    struct PollEventType
    {
        SocketHandle sock;      /**< socket句柄 */
        unsigned int reqEvents; /**< 关注的事件集合 */
        unsigned int rtEvents;  /**< 触发时的事件集合,即哪些事件被触发 */
    };

    typedef std::vector<PollEventType> PollEventSet;    /**< 事件集合类型定义 */

    /** 查询集合接口类定义\n
    *  所有查询集合的父类，所有查询集合应继承此类，并实现所有接口\n
    *  \n\n
    */
    class PollSet
    {
    public:
        virtual ~PollSet(){};

        virtual int initSet() = 0;
        virtual int clearSet() = 0;
        virtual int addFd(const SocketHandle& sock, unsigned int reqEvents) = 0;
        virtual int deleteFd(const SocketHandle& sock) = 0;
        virtual int doSetPoll(std::list<PollEventType>& reqEvents, PollEventSet& rtEvents, int* timeout) = 0;

    protected:
        PollSet(){};
    };

    /** select查询集合类定义\n
    *  \n
    *  \n\n
    */
    class PollFdSet : public PollSet
    {
    public:
        PollFdSet() :PollSet() {};
        ~PollFdSet(){};

        virtual int initSet();
        virtual int clearSet();
        virtual int addFd(const SocketHandle& sock, unsigned int reqEvents);
        virtual int deleteFd(const SocketHandle& sock);
        virtual int doSetPoll(std::list<PollEventType>& reqEvents, PollEventSet& rtEvents, int* timeout);

    private:
        fd_set _rset;
        fd_set _wset;
        fd_set _eset;
        int _maxFd;
    };

    /** epoll查询集合类定义\n
    *  \n
    *  \n\n
    */
    class PollEpollSet : public PollSet
    {
    public:
        PollEpollSet():PollSet(),_epollFd(-1), _evCount(0) {};
        ~PollEpollSet(){};

        virtual int initSet();
        virtual int clearSet();
        virtual int addFd(const SocketHandle& sock, unsigned int reqEvents);
        virtual int deleteFd(const SocketHandle& sock);
        virtual int doSetPoll(std::list<PollEventType>& reqEvents, PollEventSet& rtEvents, int* timeout);

    private:
        int _epollFd;
        // struct epoll_event* _epollEvents;
        int _evCount;

        unsigned int getEpollEvent(unsigned int event);
        unsigned int getBCFEvent(unsigned int event);
    };

    /** IO多路复用组件接口定义\n
    *  \n
    *  \n\n
    */
    class EventPoll
    {
        public:
        EventPoll();
        ~EventPoll();
        StatusType init(PollMethod method = BCF_POLL_DEFAULT);
        StatusType destory();
        StatusType poll(PollEventSet& events, int* timeout);
        StatusType addEvent(const SocketHandle& sock, unsigned int reqEvents);
        StatusType removeEvent(const SocketHandle& sock);
        StatusType removeAllEvent();
        int getEventCount();

        static void setCurrentPoll(PollMethod m);
        static PollMethod getCurrentPoll();
    private:
        std::list<PollEventType> _events;
        PollMethod _method;
        PollSet* _pollSet;
        bool _initFlag;
    
        static PollMethod curMethod;
    };
}

#endif