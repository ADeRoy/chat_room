#ifndef CHATINFOMANAGE_H
#define CHATINFOMANAGE_H
#include "common.h"
#include <map>

struct RegistInfoReq{
    char m_userName[32];
    char m_password[32];
};

struct BaseInfo{
    int  sessionId;  //会话ID
    char name[32];   //名称
    int  socket;
};

/* 用来传递给 Login 界面的数据 */
struct UserInfo{
    char m_userName[32];
    char m_password[32];
    int  m_account;
};

struct RegistInfoResp{
    int m_account;
};

struct LoginInfoReq{
    int m_account;
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
    int m_UserAccount;  //发送方
    int m_msgLen;
    int m_type;         //数据类型 0:文本,1:图片 ...
    int m_GroupAccount; //发送群号 0:广播
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
    int m_Group;    //群号 0:广播
};

struct GetGroupInfoResp
{
    char m_groupName[32]; //群名称
    int  m_GroupAccount;  //群号 0:广播
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
    int m_size;             //好友大小
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
typedef std::map<int,FriendInfo*>   mapFriendInfo;    //好友信息表

// 用户账号 +  群用户信息
typedef std::map<int,GroupUserInfo*>   mapGroupUserInfo;
// 群号 + 群成员表
typedef std::map<int,mapGroupUserInfo>   mapGroupInfo;
// 群号 + 群聊信息
typedef std::map<int,GroupChatInfo*>   mapGroupChatInfo;

class ChatInfoManage
{
public:
    ChatInfoManage();
public:
    static ChatInfoManage* getInstance(){
        if(NULL == m_pInstance)
        {
            m_pInstance = new ChatInfoManage();
        }
        return m_pInstance;
    }
public:
    //获取在线用户信息
    GroupUserInfo* getUserInfo(int account);
    //群聊信息操作
    int addGroupChatInfo(GroupChatInfo* pGroupChatInfo);
    GroupChatInfo* getGroupChatInfo(int groupAccount);
    //群成员信息操作
    int addGroupUserInfo(int groupAccount,GroupUserInfo* groupUserInfo);
    //获取群成员信息 当群为 0 的时候相当于获取在线用户信息
    GroupUserInfo* getGroupUserInfo(int groupAccount,int userAccount);
    //添加好友信息
    int addFriendInfo(FriendInfo* friendInfo);
    FriendInfo* getFriendInfo(int account);
private:
    static ChatInfoManage* m_pInstance;
    mapFriendInfo m_FriendInfoMap;                      //好友信息表
    /*static */mapGroupInfo     g_GroupUserInfoMap;     //群成员信息表
    /*static */mapGroupChatInfo g_GroupCharInfoMap;     //群信息表
};

#endif // CHATINFOMANAGE_H
