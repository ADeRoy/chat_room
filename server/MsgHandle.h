#ifndef _MSGHANDLE_H_
#define _MSGHANDLE_H_
#include "common.h"
#include "chatBase.h"
#include <stdio.h>

typedef enum
{
  RECV_HEAD = 0,
  RECV_BODY
}recvType;

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

class Session{
public:
    Session(int socket);
    ~Session();

public:
    int readEvent();
    int recvHead();
    int recvBody();
    int handleMsgBase();
    int handleMsg(recvMsg *rMsg);
    static void sendMsg(int socket,void*buf,int bufLen,int type,int error = 0,int mode = 2);
public:
    static int handleRegiste(void*arg,void*msg);
    static int handleLogin(void*arg,void*msg);
    static int handleLogout(void*arg,void*msg);
    static int handleGroupChat(void*arg,void*msg);
    int handlePrivateChat(void*msg);
    int handleGetGroupList(void*msg);
    int handleGetGroupInfo(void*msg);
    int handleGetFriendInfo(void* msg);
    int handleAddFriendReq(void*msg);
    int handleAddFriendResp(void*msg);
private:
    void cleanSession();
    void noticeUserLogin(UserInfo*userInfo);
private:
    int m_socket;
    int m_account;          //会话账号
    char m_userName[64];    //用户名
    int m_isLogin;
    recvType m_type;		//接收消息类型
    char *m_head;			//接收到的头信息
    int m_bufLen;			//需要接收消息的长度
    int m_readPos;			//当前读取到的长度
    char *m_body;			//接收到的body
    bool m_isFinish;        //接收消息,用来判断是否接收完全
    // mapFriendInfo m_friendInfoMap;
};
#endif  