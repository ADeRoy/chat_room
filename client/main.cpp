#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "common.h"
#include "MsgHandle.h"

int g_runEnable = 1;
int g_account = -1;
int g_loginStatus = -1;
const int MAX_LINE = 2048;
void writeMsg(int socket,void*buf,int bufLen,int type);
static int menu_proxy_stu()
{
    printf("\033[0m***欢迎来到梦凡聊天室***\n");
    char op;
    do
    {
        printf("╭═════════════════════════════════○●○●═══╮\n");
        printf("│        ChatRoom powered by deroy       │\n");
        printf("╰═══○●○●═════════════════════════════════╯\n");
        printf("┌───────────────────────────────────────────-┐\n");
        printf("│                                            │\n");
        printf("│ 1. 注册                        2. 登录     │\n");
        printf("│                                            │\n");
        printf("│ 3. 群聊                        4. 私聊     │\n");
        printf("│                                            │\n");
        printf("│ 5. 好友管理                    6. 群聊管理 │\n");
        printf("│                                            │\n");
        printf("└────────────────────────────────────────────┘\n");
        printf("请您选择(0-6):\n");
        op = getchar();
    } while (op < '0' || op > '6');
    return op - '0';
}

static int handleRegiste(int socket){
    /* 注册 */

}

void chatGroup(int socket)
{
    char buf[1024];
    int ret = -1;
    do
    {
        memset(buf, '\0', 1024);
        ret = read(0, buf, sizeof(buf));
        if (ret <= 0)
        {
            perror("read error..\n");
            close(socket);
            return;
        }
        if(strncmp(buf,"#Q",2) == 0){
            return;
        }
        //发送消息
        GroupChatReq *groupChatReq;
        char *p = (char *)malloc(sizeof(GroupChatReq) + strlen(buf));
        groupChatReq = (GroupChatReq *)p;
        groupChatReq->m_UserAccount = g_account;
        groupChatReq->m_type = 0;
        groupChatReq->m_GroupAccount = 0;
        groupChatReq->m_msgLen = strlen(buf);
        memmove(p + sizeof(GroupChatReq), buf, strlen(buf));

        writeMsg(socket, p, sizeof(GroupChatReq) + strlen(buf), CommandEnum_GroupChat);
        free(p);
    } while (g_runEnable);
}
void* readThread(void*arg){
    int socket = *((int*)arg);

    while (g_runEnable)
    {
        switch (menu_proxy_stu())	//菜单选择
        {
            case Exit: 
                g_runEnable = 0;
                return NULL;
                break;
            case Registe: 
                handleRegiste(socket);
                break;
            case Login:
                break;
            case GroupChat: 
                chatGroup(socket);
                break;
            case PrivateChat: 
                break;
            case GroupManage: 
                break;
            case FriendManage:
                break;
            default:
                break;
        }
    }
    
}
int main(int argc ,char**argv)
{
    /*声明套接字和链接服务器地址*/
    int sockfd;
    int optch,ret = -1;
    const char*server_addr;
    int default_port = 8000;

    struct sockaddr_in servaddr;
    /*判断是否为合法输入 必须传入一个参数：服务器Ip*/
    if(argc<3)
    {
        printf("user default addr:0.0.0.0 and port 8000\n");
        server_addr = "0.0.0.0";
    }
    while((optch = getopt(argc, argv, "s:p:")) != -1)
	{
		switch (optch)
		{
        case 's':
            server_addr = optarg;
            break;
        case 'p':
            default_port = atoi(optarg);
            printf("port: %s\n", optarg);
            break;
        case '?':
            printf("Unknown option: %c\n",(char)optopt);    
            break;
        default:
            break;
		}
	}
    recvType type = RECV_HEAD;
    /*(1) 创建套接字*/
    sockfd =socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1)
    {
        perror("socket error");
        return 0;
    }
    /*(2) 设置连接服务器地址结构*/
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family =AF_INET;
    servaddr.sin_port = htons(default_port);
    if(inet_pton(AF_INET , server_addr , &servaddr.sin_addr) < 0)
	{
		printf("inet_pton error for %s\n",server_addr);
		return 0;
	}
    /*  (3) 发送连接服务器请求  */
    if( (ret = connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr))) < 0)
	{
		perror("connect error");
		return 0;
	}
    printf("connect seccess,ret:%d..\n",ret);
    struct sockaddr_in c_addr;
    memset(&c_addr, 0, sizeof(c_addr));
    socklen_t len = sizeof(c_addr);

    if(getsockname(sockfd, (struct sockaddr *)&c_addr, &len) == 0)
    {
        printf("getsockname:%s\n",inet_ntoa(c_addr.sin_addr));
    }
    else{
        printf("getsockname: error\n");
    }

    
    pthread_t readThdId = 0;
    pthread_create(&readThdId,NULL,readThread,&sockfd);

    Session session(sockfd);

    char buf[MAX_LINE];
    while (g_runEnable)
    {
        char msg[1024] = "hello world";
        /* 注册 */
        DeMessageHead header;
        RegistInfoReq info;
        memset(&info,'\0',sizeof(RegistInfoReq));
        strncpy(info.m_userName,"test",strlen("test"));
        strncpy(info.m_password,"password",strlen("password"));
        writeMsg(sockfd,&info,sizeof(RegistInfoReq),CommandEnum_Registe);

        /*接收注册响应*/
        memset(&header,'\0',sizeof(DeMessageHead));
        read(sockfd,&header,sizeof(DeMessageHead));
        char *p = (char*)malloc(header.length);
        DeMessagePacket* pPacket = (DeMessagePacket *)p;
        read(sockfd,pPacket,header.length);
        RegistInfoResp* resp = (RegistInfoResp*)(p+sizeof(DeMessagePacket));
        printf("account:%d\n",resp->m_account);
        g_account = resp->m_account;
        free(p);

        //登录
        LoginInfoReq loginInfo;
        memset(&loginInfo,'\0',sizeof(LoginInfoReq));
        loginInfo.m_account = g_account;
        strncpy(loginInfo.m_password,"password",strlen("password"));
        writeMsg(sockfd,&loginInfo,sizeof(LoginInfoReq),CommandEnum_Login);
        
        GetGroupInfoReq getGroupInfoReq;
        getGroupInfoReq.m_Group = 0;
        writeMsg(sockfd,&getGroupInfoReq,sizeof(GetGroupInfoReq),CommandEnum_GetGroupInfo);
        
        while (g_runEnable)
        {
            if (session.readEvent() == RET_EXIT)
            {
                printf("offline..\n");
                close(sockfd);
                return 0;
            }
        }
    }
    close(sockfd);
    return 0;
}

void writeMsg(int socket,void*buf,int bufLen,int type)
{
    DeMessageHead header;
    memcpy(header.mark, "DE", sizeof(header.mark));
    header.encoded = '0';
    header.version = '0';
    header.length = sizeof(DeMessagePacket) + bufLen;

    char *p = (char *)malloc(header.length);
    DeMessagePacket *pPacket = (DeMessagePacket *)p;
    pPacket->mode = 2;
    pPacket->sequence = getSeqNum();
    pPacket->command = type;
    pPacket->error = 0;
    if(buf)
        memcpy(p + sizeof(DeMessagePacket), buf, bufLen);

    char *sendMsg = new char[sizeof(DeMessageHead) + header.length];
    memset(sendMsg, 0, sizeof(DeMessageHead) + header.length);
    memcpy(sendMsg, &header, sizeof(DeMessageHead));
    memcpy(sendMsg + sizeof(DeMessageHead), p, header.length);
    free(p);
    write(socket, sendMsg, sizeof(DeMessageHead) + header.length);
    delete sendMsg;
}
