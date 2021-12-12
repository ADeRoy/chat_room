#ifndef _COMMON_H_
#define _COMMON_H_
#include<pthread.h>
//网络相关
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include <assert.h>
//c++
#include <iostream>
#include <map>
using namespace std;

#define RET_OK 0
#define RET_ERROR -1
#define RET_AGAIN -2    //重新读取
#define RET_EXIT -3     //客户端退出

//De 协议包头▄︻┻┳═一…… ☆(>○<)
struct DeMessageHead{
    char mark[2];   // "DE" 认证deroy的协议
    char version;
    char encoded;   //0 不加密，1 加密
    int length;
};

struct DeMessagePacket
{
    /* data */
    int mode;  //1 请求,2 应答,3 消息通知
    int error; //0 成功,非0,对应的错误码

    int sequence;   //序列号
    int command;    //命令号
};

enum
{
    CommandEnum_Registe,
    CommandEnum_Login,
    CommandEnum_Logout,
    CommandEnum_GroupChat,
    CommandEnum_AddFriend,
    CommandEnum_delFriend,
    CommandEnum_PrivateChat,
    CommandEnum_CreateGroup,
    CommandEnum_GetGroupList,
    CommandEnum_GetGroupInfo,
    CommandEnum_GetFriendList
};

typedef enum
{
  RECV_HEAD = 0,
  RECV_BODY
}recvType;

struct RegistInfoReq{
    char m_userName[32];
    char m_password[32];
};

struct BaseInfo{
    int sessionId;  //会话ID
    char name[32];  //名称
    int socket;
};

struct RegistInfoResp{
    int m_account;
};

struct LoginInfoReq{
    int m_account;  //账号
    char m_password[32];
};

struct GroupChatInfo
{
    /* data */
    char m_groupName[32]; //群名称
    int  m_account;       //群账号
};

struct GroupChatReq
{
    int m_UserAccount;  //发送方
    int m_msgLen;
    int m_type;         //数据类型 0:文本,1:图片 ...
    int m_GroupAccount;    //发送群号 0:广播   
};

struct PrivateChatReq
{
    int m_UserAccount;      //发送的账号
    int m_msgLen;
    int m_type;             //数据类型 0:文本,1:图片 ...
    int m_FriendAccount;    //发送好友账号
};

struct GetGroupInfoReq
{
    /* data */
    int m_Group;    //群号 0:广播   
};

struct GetGroupInfoResp
{
    /* data */
    char m_groupName[32]; //群名称
    int  m_GroupAccount;     //群号 0:广播   
    int  m_size;          //群成员大小
};

struct GroupUserInfo{
    char m_userName[32];
    int  m_account;  //账号
    int  m_right;    //权限 0:群成员 1:群管 2:群主
};

/*  好友请求接口封装  */
struct GetFriendInfoResp
{
    int m_size;             //群成员大小
};

struct AddFriendInfoReq
{
    int m_friendAccount;    //好友账号
    int m_senderAccount;    //发送端账号
    char m_reqInfo[64];    //请求信息 例如我是xxx
};

struct AddFriendInfoResp
{
    int m_friendAccount;    //好友账号
    int m_senderAccount;    //发送端账号
    int status;             //同意0，不同意-1
};

struct FriendInfo{
    char m_userName[32];//好友用户名
    int  m_account;     //账号
    int  m_status;      //是否添加成功  0:等待 1:成功
};
//好友信息表，属于Session会话对象，即每个客户端一个好友信息表
typedef std::map<int,FriendInfo*>   mapFriendInfo;    //好友信息表

typedef std::map<int,GroupChatInfo*>   mapGroupChatInfo;
// 用户账号 +  群用户信息
typedef std::map<int,GroupUserInfo*>   mapGroupUserInfo;
// 群号 + 群成员表
typedef std::map<int,mapGroupUserInfo>   mapGroupInfo;
static mapGroupInfo g_GroupUserInfoMap;         //群成员总表
static mapGroupChatInfo g_GroupCharInfoMap;     //群总表


typedef enum Menu
{
    Exit, Registe, Login, GroupChat, PrivateChat, GroupManage, FriendManage
}MENU;

// typedef enum
// {
//   MSG_REQ = 0,  //请求消息
//   MSG_RESP      //响应消息
// }MsgType;

// class MessageManage{
// public:
//     MessageManage();
//     ~MessageManage();

// public:
//     recvType m_type;					//接收消息类型
//     char *m_head;							//接收到的头信息
//     int m_bufLen;							//需要接收消息的长度
//     int m_readPos;						//当前读取到的长度
//     char *m_body;							//接收到的body
//     MsgType m_msgType;		    //消息类型
//     bool m_isFinish;          //如果是接收回复消息。用来判断是否接收完全，如果是请求，则该变量没有什么作用
// };

static int getSeqNum()
{
  static int num = 0;
  if (num++ >= 0xEFFFFFFF - 1)
  {
    num = 0;
  }
  return num;
}

static int getAccountNum()
{
    static int account = 10000;
    if (account++ >= 0xEFFFFFFF - 1)
    {
        account = 10000;
    }
    return account;
}

#endif