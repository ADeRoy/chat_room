#ifndef COMMON_H
#define COMMON_H

#include "Session.h"
#include "chatinfomanage.h"
#include <QTcpSocket>
#include <qDebug>
#include <QDateTime>

#include <iostream>
#include <map>
using namespace std;

#define RET_OK 0
#define RET_ERROR -1
#define RET_AGAIN -2    //重新读取
#define RET_EXIT -3     //客户端退出
#define RET_END -4      //读取结束

#define MAX_SEND_LENGTH 6144

#ifndef FILENAME
#define FILENAME (__FILE__)
#endif

#ifndef FILEFUNCTION
#define FILEFUNCTION  (__FUNCTION__)
#endif

#ifndef FILELINE
#define FILELINE   (__LINE__)
#endif

#define ChatLog qDebug().noquote()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz")\
                                     <<"["<<FILENAME<<":"<<FILELINE<<"]["<<FILEFUNCTION<<"]"

#define ChatLogInfo(INFO) qDebug().noquote()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz")\
                                     <<"["<<FILENAME<<":"<<FILELINE<<"]["<<FILEFUNCTION<<"]"

#define LOGINFO(format, ...)                                                         \
    {                                                                              \
        qDebug("%s [ %s : %d] [%s]>>" format, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz").toStdString().c_str()\
                        , FILENAME,FILELINE, FILEFUNCTION ,##__VA_ARGS__); \
    }
/**
  * 暂时用宏定义代替，后续通过读取配置文件获取
  */
#define SERVER_ADDR "172.19.0.51" //这是我的服务器地址，需要修改为你启动服务器的地址
#define SERVER_PORT 8000

//De 协议包头
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

#define CMD_CHAT_GET(intCmd, stringCmd) \
    case intCmd:                          \
        return stringCmd;

#define DA_PROTOBUFCMD_MAP(CHATCMD)                        \
    CHATCMD(CommandEnum_Registe, "UserRegiste")   \
    CHATCMD(CommandEnum_Login, "UserLogin") \
    CHATCMD(CommandEnum_Logout, "UserLogout")  \
    CHATCMD(CommandEnum_GroupChat, "GroupChat")    \
    CHATCMD(CommandEnum_AddFriend, "AddFriend")      \
    CHATCMD(CommandEnum_delFriend, "delFriend")       \
    CHATCMD(CommandEnum_PrivateChat, "PrivateChat")   \
    CHATCMD(CommandEnum_CreateGroup, "CreateGroup") \
    CHATCMD(CommandEnum_GetGroupList, "GetGroupList")  \
    CHATCMD(CommandEnum_GetGroupInfo, "GetGroupInfo")    \
    CHATCMD(CommandEnum_GetFriendInfo, "GetFriendInfo")      \

enum{
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
    CommandEnum_GetFriendInfo
};

static const char *getChatCmdString(int cmd)
{
    switch (cmd)
    {
        DA_PROTOBUFCMD_MAP(CMD_CHAT_GET)
        default:
            return "no define cmd";
    }
}

typedef enum Menu
{
    Exit, Registe, Login, GroupChat, PrivateChat, GroupManage, FriendManage
}MENU;

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

#endif // COMMON_H
