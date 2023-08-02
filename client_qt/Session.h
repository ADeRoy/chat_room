#ifndef SESSION_H
#define SESSION_H
#include "common.h"
#include <QObject>
#include <assert.h>
#include <QTcpSocket>

//用于保存收到的MA的消息
typedef struct _recvMsg
{
  _recvMsg()
  {
    head = NULL;
    body = NULL;
    bodyLen = 0;
  }
  ~_recvMsg()
  {
    if (head != NULL)
    {
      delete[] head;
      head = NULL;
    }

    if (body != NULL)
    {
      delete[] body;
      body = NULL;
    }
  }
  char *head;
  char *body;
  int bodyLen;
}recvMsg;

typedef enum
{
  RECV_HEAD = 0,
  RECV_BODY
}recvType;


class Session:public QObject
{
    Q_OBJECT
public:
    Session(QTcpSocket* socket);
    ~Session();
public:
    int readEvent();
    int recvHead();
    int recvBody();
    int handleMsgBase();
//    int handleMsg(recvMsg *rMsg);
//    static void sendMsg(QTcpSocket* socket,void*buf,int bufLen,int type,int error=0);
signals:
    void signal_handleMsg(recvMsg *rMsg);
public:
//    static int handleRegiste(void*arg,void*msg);
//    static int handleLogin(void*arg,void*msg);
//    static int handleLogout(void*arg,void*msg);
//    static int handleGroupChat(void*arg,void*msg);
//    int getGroupList(void*msg);
//    int getGroupInfo(void* msg);
private:
    void cleanSession();
private:
    QTcpSocket* m_socket;
    recvType m_type;		//接收消息类型
    char *m_head;			//接收到的头信息
    int m_bufLen;			//需要接收消息的长度
    int m_readPos;			//当前读取到的长度
    char *m_body;			//接收到的body
    bool m_isFinish;        //接收消息,用来判断是否接收完全
};

#endif // SESSION_H
