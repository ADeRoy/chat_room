#ifndef WIDGET_H
#define WIDGET_H

#include <QListWidget>
#include <QWidget>
#include "common.h"
#include "logindlg.h"
#include "addfrienddialog.h"
#include "custom/chatmessage.h"
#include <map>
#include <list>
#include <QListWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

typedef enum{
    TYPE_GROUP_CHAT,TYPE_PRIVATE_CHAT
}EChatType;

struct chatWidgetInfo{
    int m_account;        // 聊天窗口对应的聊天号
    EChatType m_type;     // 聊天窗口对应的类型 群聊/私聊
};

typedef std::map<int,QListWidget*> mapChatWidget;
typedef std::list<chatWidgetInfo*> listChatWidgetInfo;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
protected:
    // Event handlers
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event); // 双击
public:
    QPoint mouseWindowTopLeft; //鼠标相对窗口左上角的坐标         在mousePressEvent 得到
    QPoint mouseDeskTopLeft;   //鼠标相对于桌面左上角坐标         在mouseMoveEvent实时获取
    QPoint windowDeskTopLeft;  //窗口左上角相对于桌面左上角坐标    在mouseMoveEvent实时计算(矢量)获得

private slots:
    void disconnectedSlot();
    void readyReadSlot();
    void on_pushBtn_send_clicked();

    void SlotUserGroupButtonClicked(int index); //这个是组按钮按下后响应槽函数
private slots:
    int handleMsg(recvMsg *rMsg);
    void on_pushButton_addFriend_clicked();

//    void on_listWidget_info_itemClicked(QListWidgetItem *item);

    void on_pushBtn_close_clicked();

    void on_pushBtn_hide_clicked();

    void on_pushBtn_max_clicked();

private:
    void Init();
    void InitUI();
    int  login();
    void Init_Group_Info(GroupChatInfo* groupInfo);
    void Init_Friend_Info(FriendInfo* friendInfo);
    void writeMsg(void*buf,int bufLen,int type, int error = 0,int mode = 1);
private:
    void getGroupList();
    void getFriendList();
public:
    int getLoginStatus(){
        return m_isLogin;
    }
#if 0
    GroupUserInfo* findUserInfo(int account);
#endif
private:
    int handleRegiste(void*msg);
    int handleLogin(void*msg);
    int handleLogout(void*msg);
    int handleGroupChat(void*msg);
    int handlePrivateChat(void*msg);
    int getGroupList(void*msg);
    int getGroupInfo(void* msg);
    int getFriendInfo(void* msg);


    int handleAddFriendReq(void*msg);
    int handleAddFriendResp(void*msg);
private:
    void dealMessage(QListWidget* listWidget,ChatMessage *messageW, QListWidgetItem *item, QString text, QString time, QString ip ,ChatMessage::User_Type type); //用户发送文本
    void dealMessageTime(QListWidget* listWidget,QString curMsgTime); //处理时间
private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    bool        socketState;
    Session*    m_session;
    UserInfo    m_userInfo;
    bool        m_isLogin;
    bool        m_isfull;
    QRect       m_rect;

    mapChatWidget           m_chatWigetMap;     //聊天列表
    listChatWidgetInfo      m_chatWidgetInfoList;
    int m_curCheckedUser;
};
#endif // WIDGET_H
