#include "chatTask.h"

void *taskThread(void *arg){
    printf("-------------------- new connect --------------------\n");
    // pthread_mutex_lock(&_mxMessage);   //lock the mutex
    // pthread_mutex_unlock(&_mxMessage);  //unlock the mutex
    int clientFd = *((int *)arg);
	char recvMsg[1024*2];
	int recvLen;
    int head_len = sizeof(DeMessageHead);

	free(arg);
    arg = NULL;

    Session session(clientFd);
	while (1)
	{
        if(session.readEvent() == RET_EXIT)
        {
			close(clientFd);
			return NULL;
        }
#if 0
        memset(recvMsg,'\0',sizeof(recvMsg));
		recvLen = read(clientFd, recvMsg, sizeof(DeMessageHead));
		if (recvLen <= 0)
		{   //下线
			printf("offline\n");
			close(clientFd);
			return NULL;
		}
        printf("readLen:%d,HeadLen:%d\n",recvLen,sizeof(DeMessageHead));
        memmove((void*)&headTmp,recvMsg,sizeof(DeMessageHead));
        printf("mark:%s,encoded:%d,length:%d,version:%d\n",headTmp.mark,headTmp.encoded,headTmp.length,headTmp.version);

        memset(recvMsg,'\0',sizeof(recvMsg));
        recvLen = read(clientFd, recvMsg, sizeof(DeMessagePacket));
        printf("readLen:%d,PacketLen:%d\n",recvLen,sizeof(DeMessagePacket));
		if (recvLen <= 0)
		{   //下线
			printf("offline\n");
			close(clientFd);
			return NULL;
		}
        memmove((void*)&packetTmp,recvMsg,sizeof(DeMessagePacket));
        printf("command:%d,error:%d,,mode:%d,sequence:%d\n",packetTmp.command,packetTmp.error,packetTmp.mode,packetTmp.sequence);

        memset(recvMsg,'\0',sizeof(recvMsg));
        recvLen = read(clientFd, recvMsg, headTmp.length - sizeof(DeMessagePacket));
        printf("readLen:%d,HeadLen:%d\n",recvLen,sizeof(DeMessageHead));
		if (recvLen <= 0)
		{   //下线
			printf("offline\n");
			close(clientFd);
			return NULL;
		}
        printf("msg:%s\n",recvMsg);
#endif
    }
    return NULL;
}