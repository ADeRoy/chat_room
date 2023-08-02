#include "common.h"
#include "EventPool.h"  
#include "MsgHandle.h"
#define USE_EVENTPOLL
using namespace std;
using namespace DE;

int handleFdReadable(SocketHandle fd);

int main(int argc, char *argv[])
{
    int default_port = 8000;
    int optch = 0;
    while ((optch = getopt(argc, argv, "s:p:")) != -1)
    {
        switch (optch)
        {
        case 'p':
            default_port = atoi(optarg);
            LOGINFO("port: %s\n", optarg);
            break;
        case '?':
            LOGINFO("Unknown option: %c\n", (char)optopt);
            break;
        default:
            break;
        }
    }
    pthread_mutex_init(&_mxMessage, NULL);
    // pthread_mutex_lock(&_mxMessage);    // lock the mutex

    // pthread_mutex_unlock(&_mxMessage);  // unlock the mutex
    /*声明服务器地址和客户链接地址*/
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    /*声明服务器监听套接字和客户端链接套接字*/
    int listen_fd, connect_fd;

    /*(1) 初始化监听套接字listenfd*/
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("Socket Error:");
        return 0;
    }

    /*(2) 设置服务器sockaddr_in结构*/
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //任意地址
    server_addr.sin_port = htons(default_port);

    // 设置允许socket立即重用
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&listen_fd, sizeof(listen_fd));

    /*(3) 绑定套接字和端口*/
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind error:");
        return 0;
    }

    /*(4) 监听客户请求*/
    if (listen(listen_fd, 4) == -1)
    {
        perror("Listen error:");
        return 0;
    }
    /*(5) 接受客户请求*/
    cout << "server start lister:" << "0.0.0.0" << ":" << default_port << endl;

#if defined USE_EVENTPOLL
        EventPoll m_eventPoll;
        m_eventPoll.init(BCF_POLL_EPOLL); // use epoll BCF_POLL_SELECT
        m_eventPoll.addEvent(listen_fd, BCF_POLLIN);
        PollEventSet m_events; //就绪的事件

        int timeOut;
        while (true)
        {
            timeOut = 2 * 1000; //超时单位为毫秒
            StatusType ret = m_eventPoll.poll(m_events, &timeOut);
            if (ret == ST_OK)
            {
                for (unsigned int i = 0; i < m_events.size(); i++)
                {
                    //监听句柄
                    if (m_events[i].rtEvents && m_events[i].sock == listen_fd)
                    {
                        /*  处理可读事件  */
                        int newConnect = handleFdReadable(m_events[i].sock);
                        m_eventPoll.addEvent(newConnect, BCF_POLLIN);
                        cout << "listen sock:" << m_events[i].sock << ",accept socket:" << newConnect << endl;
                        SessionPtr session = std::make_shared<Session>(newConnect);
                        int ret = SessionMng::Instance()->addSession(session, newConnect);

                    }
                    else
                    {
                        //普通IO句柄    输入和close都会触发，如果对端 close 掉之后未移除事件将会循环触发
                        if (m_events[i].rtEvents & BCF_POLLIN)
                        {
                            int ret = SessionMng::Instance()->handleSession(m_events[i].sock);
                            if(ret<0){
                                m_eventPoll.removeEvent(m_events[i].sock);
                                cout << "client close fd:" << m_events[i].sock << ".." << endl;
                            }
                        }
                    }
                }
            }

            //处理超时
            if (ret == ST_TIMEOUT)
            {
                continue;
            }
            else if (ret != ST_OK)
            {
                if (ret == ST_SYSERROR)
                {
                    if (errno != EINTR)
                    {
                        cout << "event poll failed, errno:" <<errno << "error:" << strerror(errno) << endl;
                        break;
                    }
                    else
                    {
                        cout << "event poll failed ERROR:" << ret << endl;
                    }
                }
                else
                {
                    cout << "event poll failed " << ret << endl;
                    // break;
                }
                continue;
            }
        }
#else
    for (;;)
    {
        client_len = sizeof(client_addr);
        connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        if (connect_fd < 0)
        {
            perror("accept error");
            return 0;
        }
        LOGINFO("Connect from %s:%u...\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        int *argv = (int *)malloc(sizeof(int));
        *argv = connect_fd;
        CREATE_THREAD(NULL, 1024 * 4, true, taskThread, (void *)argv, NULL);
    }
#endif
    close(listen_fd);
    return 0;
}

// 处理监听 
int handleFdReadable(SocketHandle fd)
{
    #if 0
    SocketHandle tmpSockFd;
    BCFSocket::TransAddr trashy;
    tmpSockFd = BCFSocket::accept(fd, trashy, NULL);
    return tmpSockFd;
    #endif
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    SocketHandle sconnFd;

    sconnFd = ::accept(fd, (struct sockaddr *)&clientAddr, &addrLen);
    if (sconnFd != -1 && clientAddr.sin_family == AF_INET)
    {
        std::string ip = inet_ntoa(clientAddr.sin_addr);
        unsigned short port = ntohs(clientAddr.sin_port);
    }
    return sconnFd;
    return 0;
}

//修身，齐家