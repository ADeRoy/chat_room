#include "common.h"

using namespace std;
int main(int argc,char* argv[])
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
    // pthread_mutex_lock(&_mxMessage);   //lock the mutex

    // pthread_mutex_unlock(&_mxMessage);  //unlock the mutex
    /*声明服务器地址和客户链接地址*/
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    /*声明服务器监听套接字和客户端链接套接字*/
    int listen_fd,connect_fd;

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
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&listen_fd, sizeof(listen_fd));  

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
        int* argv = (int *)malloc(sizeof(int));
        *argv = connect_fd;
        CREATE_THREAD(NULL,1024*4,true,taskThread,(void*)argv,NULL);
    }
    close(listen_fd);
    return 0;
}