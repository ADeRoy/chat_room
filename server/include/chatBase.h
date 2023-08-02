#ifndef _CHATBASE_H_
#define _CHATBASE_H_
#include "common.h"
#include <map>
#include <pthread.h>
using namespace std;
static int g_idIndex = 10000;
/**
 * @brief 注册用户信息
 */
struct RegistInfoReq{
    char m_userName[32];
    char m_password[32];
};

struct RegistInfoResp{
    int m_account;
};

/**
 * @brief 登录用户信息
 */
struct UserInfo{
    char m_userName[32];
    char m_password[32];
    int  m_account;     //账号
    int  m_socket;      //句柄
};

struct LoginInfoReq{
    int m_account;      //账号
    char m_password[32];
};

struct GroupChatInfo
{
    char m_groupName[32]; //群名称
    int  m_account;       //群账号
    int  m_size;          //群大小
};

struct GetGroupListResp
{
    int m_size;             //群数量大小
};

struct GroupChatReq
{
    int m_UserAccount;      //发送的账号
    int m_msgLen;
    int m_type;             //数据类型 0:文本,1:图片 ...
    int m_GroupAccount;     //发送群号 0:广播
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
    int m_GroupAccount;    //群号 0:广播   
};

struct GetGroupInfoResp
{
    char m_groupName[32];   //群名称
    int m_GroupAccount;     //群号 0:广播   
    int m_size;             //群成员大小
};

struct GroupUserInfo{
    char m_userName[32];
    int  m_account;     //账号
    int  m_right;       //权限 0:群成员 1:群管 2:群主
};

/*  好友请求接口封装  */
struct GetFriendInfoResp
{
    int m_size;         //群成员大小
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
    int  m_status;      //是否添加成功 0:等待添加   1：同意
};

//好友信息表，属于Session会话对象，即每个客户端一个好友信息表
typedef std::map<int,FriendInfo*>       mapFriendInfo;    //好友信息表
typedef std::map<int,mapFriendInfo>     mapUserFriendInfo;    //用户好友信息表

//account,other
typedef std::map<int,GroupChatInfo*>    mapGroupChatInfo;    //群信息表
typedef std::map<int,RegistInfoReq*>    mapAccountInfo;      //注册用户表
typedef std::map<int,UserInfo*>         mapUserInfo;          //在线用户表


static mapGroupChatInfo g_GroupCharInfoMap; //群信息表
static mapAccountInfo   g_AccountInfoMap;   //注册账户信息表
static mapUserInfo      g_UserInfoMap;      //在线用户信息表
static mapUserFriendInfo g_UserFriendInfoMap;   //用户好友信息表

static pthread_mutex_t _mxMessage;

#endif