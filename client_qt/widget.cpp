#include "widget.h"
#include "ui_widget.h"
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <windows.h>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_curCheckedUser(0)
{
    ui->setupUi(this);
    InitUI();
    Init();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    mouseWindowTopLeft = event->pos();
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    //窗口移动
    if (event->buttons() & Qt::LeftButton)
    {
        mouseDeskTopLeft = event->globalPos();
        windowDeskTopLeft = mouseDeskTopLeft - mouseWindowTopLeft;  //矢量计算
        this->move(windowDeskTopLeft);     //移动到目的地
    }
}

void Widget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if(m_isfull){
        //取消全屏
        m_isfull = false;
        ui->centerWidget->setGeometry(m_rect);

        ui->centerWidget->move(QApplication::desktop()->screen()->rect().center() - ui->centerWidget->rect().center());
    }
    else {
        m_isfull = true;
        m_rect = ui->centerWidget->rect();
        setGeometry(QGuiApplication::primaryScreen()->availableGeometry()); // 不包含windows任务栏区域
        ui->centerWidget->setGeometry(this->rect());
    }
}


void Widget::on_pushBtn_max_clicked()
{
//    this->showFullScreen(); //全屏
    if(m_isfull){
        //取消全屏
        m_isfull = false;
        ui->centerWidget->setGeometry(640,480,m_rect.width(),m_rect.height());
        ui->centerWidget->move(QApplication::desktop()->screen()->rect().center() - ui->centerWidget->rect().center());
    }
    else {
        m_isfull = true;
        m_rect = ui->centerWidget->rect();
        setGeometry(QGuiApplication::primaryScreen()->availableGeometry()); // 不包含windows任务栏区域
        ui->centerWidget->setGeometry(this->rect());
    }
//    ui->centerWidget->showMaximized();
//    this->showMaximized();
}

void Widget::disconnectedSlot()
{
    ChatLogInfo()<<"Socket close";
}

void Widget::readyReadSlot()
{
    if(m_session->readEvent()!=RET_END){
        ChatLogInfo()<<"读取出错了";
    }
}

void Widget::InitUI()
{
    this->setWindowTitle("WeChat");
//    int width = this->width()-10;
//    int height = this->height()-10;
//    ui->centerWidget->setGeometry(5,5,width,height);
    ui->centerWidget->setStyleSheet("QWidget#centerWidget{ border-radius:4px; background:rgba(255,255,255,1); }");

    this->setWindowFlags(Qt::FramelessWindowHint);          //去掉标题栏无边框
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    //实例阴影shadow
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    //设置阴影距离
    shadow->setOffset(0, 0);
    //设置阴影颜色
    shadow->setColor(QColor(39,40,43,100));
    //设置阴影圆角
    shadow->setBlurRadius(10);
    //给嵌套QWidget设置阴影
    ui->centerWidget->setGraphicsEffect(shadow);


    m_isfull = false;
}

void Widget::Init()
{
    socket = new QTcpSocket;
    socketState = false;
    m_isLogin = false;

    QString ipAddressStr = SERVER_ADDR;
    quint16 port = SERVER_PORT;

    if(!socketState)
    {
        socket->connectToHost(ipAddressStr, port);
        if(socket->waitForConnected(3000))
        {
            ChatLogInfo() << "Connect2Server OK";
            socketState = true;
        }
        else
        {
            ChatLogInfo() << socket->errorString();
            return;
        }
    }
    else
    {
        socket->close();    //触发disconnected()信号
        socketState = false;
    }

    m_session = new Session(socket);
    connect(m_session,SIGNAL(signal_handleMsg(recvMsg *)),this,SLOT(handleMsg(recvMsg *)));

    LoginDlg loginDlg;  //这个loginDlog必须是父窗口
    do{
        loginDlg.show();
        int status = loginDlg.exec();
        if(status == QDialog::Accepted)
        {
            ChatLogInfo()<<"will loadin..";
            UserInfo* userInfo = loginDlg.getUserInfo();
            memmove(&m_userInfo,userInfo,sizeof(UserInfo));
            ChatLogInfo()<<"get account:"<<m_userInfo.m_account;
            ChatLogInfo()<<"get password:"<<m_userInfo.m_password;
            login();
        }
        else if(status == QDialog::Rejected){
            ChatLogInfo()<<"close..";
            this->m_isLogin = false;
            return;
        }
    }while(this->m_isLogin == false);

    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnectedSlot()));     //客户端断开连接
    connect(socket, SIGNAL(readyRead()),this, SLOT(readyReadSlot()));           //接收消息

    getGroupList();
    getFriendList();

    connect(ui->userListWidget,SIGNAL(SignalUserGroupButtonClicked(int)),this,SLOT(SlotUserGroupButtonClicked(int)));

}


int Widget::login()
{
    //登录
    LoginInfoReq loginInfo;
    memset(&loginInfo,'\0',sizeof(LoginInfoReq));
    loginInfo.m_account = m_userInfo.m_account;
    ChatLogInfo()<<"m_account:"<<loginInfo.m_account<<",password:"<<m_userInfo.m_password;
    strncpy(loginInfo.m_password,m_userInfo.m_password,strlen(m_userInfo.m_password));
    writeMsg((char*)&loginInfo,sizeof(LoginInfoReq),CommandEnum_Login);

    if(this->socket->waitForReadyRead()==false){
        return -1;
    }

    DeMessageHead header;
    memset(&header,'\0',sizeof(DeMessageHead));
    int len = socket->read((char*)&header,sizeof(DeMessageHead));
    char *p = (char*)malloc(header.length);
    DeMessagePacket* pPacket = (DeMessagePacket *)p;
    socket->read((char*)pPacket,header.length);
    if(pPacket->error == 0){
        ChatLogInfo()<<"登录成功..";
        m_isLogin = true;
    }
    free(p);
    return true;
}

static int stackWidgetIndex = 0;

void Widget::Init_Group_Info(GroupChatInfo* groupInfo)
{
//    ui->listWidget_info->addItem(QString("[群][%1]").arg(groupInfo->m_groupName));
    ui->userListWidget->AddWXWidget(groupInfo->m_groupName,":/src/头像/bianchenxuexijidi.jpg");
    ui->stackedWidget_Msg->setCurrentIndex(stackWidgetIndex++);
    if(stackWidgetIndex >=ui->stackedWidget_Msg->count())
    {
        QWidget* page = new QWidget();
        page->setObjectName(QString("page%1").arg(stackWidgetIndex));
        ui->stackedWidget_Msg->addWidget(page);
        ChatLogInfo()<<"stackedWidget_Msg account:"<<ui->stackedWidget_Msg->count()<<",stackWidgetIndex:"<<stackWidgetIndex;
    }
    QListWidget* listWidget = new QListWidget;
    QGridLayout* mainLayout = new QGridLayout;
    ui->stackedWidget_Msg->currentWidget()->setLayout(mainLayout);
    listWidget->setStyleSheet("QListWidget{"\
                              "background-color: rgb(247,247,247);"\
                              "border-style: none;"\
                              "}");
    mainLayout->addWidget(listWidget);
    mainLayout->setMargin(0); //设置外边距
    mainLayout->setSpacing(0);//设置内边距

    m_chatWigetMap.insert(std::make_pair(groupInfo->m_account,listWidget));
    chatWidgetInfo* chatInfo = new chatWidgetInfo;
    chatInfo->m_account = groupInfo->m_account;
    chatInfo->m_type = TYPE_GROUP_CHAT;

    m_chatWidgetInfoList.push_back(chatInfo);
    m_curCheckedUser = stackWidgetIndex - 1;
//    ui->listWidget_info->setCurrentRow(stackWidgetIndex - 1);
}

void Widget::Init_Friend_Info(FriendInfo *friendInfo)
{
//    ui->listWidget_info->addItem(QString("[好友][%1]").arg(friendInfo->m_userName));
    ui->userListWidget->AddWXWidget(friendInfo->m_userName,":/src/头像/icebear.jpg");

    ui->stackedWidget_Msg->setCurrentIndex(stackWidgetIndex++);
    if(stackWidgetIndex >=ui->stackedWidget_Msg->count())
    {
        QWidget* page = new QWidget();
        page->setObjectName(QString("page%1").arg(stackWidgetIndex));
        ui->stackedWidget_Msg->addWidget(page);
        ChatLogInfo()<<"stackedWidget_Msg account:"<<ui->stackedWidget_Msg->count()<<",stackWidgetIndex:"<<stackWidgetIndex;
    }
    QListWidget* listWidget = new QListWidget;
    QGridLayout* mainLayout = new QGridLayout;
    ui->stackedWidget_Msg->currentWidget()->setLayout(mainLayout);
    listWidget->setStyleSheet("QListWidget{"\
                              "background-color: rgb(247,247,247);"\
                              "border-style: none;"\
                              "border-top:1px solid #D6D6D6;"\
                              "}");
    mainLayout->addWidget(listWidget);
    mainLayout->setMargin(0); //设置外边距
    mainLayout->setSpacing(0);//设置内边距

    m_chatWigetMap.insert(std::make_pair(friendInfo->m_account,listWidget));
    chatWidgetInfo* chatInfo = new chatWidgetInfo;
    chatInfo->m_account = friendInfo->m_account;
    chatInfo->m_type = TYPE_PRIVATE_CHAT;

    m_chatWidgetInfoList.push_back(chatInfo);
    m_curCheckedUser = stackWidgetIndex - 1;
//    ui->listWidget_info->setCurrentRow(stackWidgetIndex - 1);
}

void Widget::writeMsg(void *buf, int bufLen, int type, int error,int mode)
{
    DeMessageHead header;
    memcpy(header.mark, "DE", sizeof(header.mark));
    header.encoded = '0';
    header.version = '0';
    header.length = sizeof(DeMessagePacket) + bufLen;

    char *p = (char *)malloc(header.length);
    DeMessagePacket *pPacket = (DeMessagePacket *)p;
    pPacket->mode = mode;
    pPacket->sequence = getSeqNum();
    pPacket->command = type;
    pPacket->error = error;
    if(buf)
        memcpy(p + sizeof(DeMessagePacket), buf, bufLen);

    char *sendMsg = new char[sizeof(DeMessageHead) + header.length];
    memset(sendMsg, 0, sizeof(DeMessageHead) + header.length);
    memcpy(sendMsg, &header, sizeof(DeMessageHead));
    memcpy(sendMsg + sizeof(DeMessageHead), p, header.length);
    if(p!=NULL){
        free(p);
        p = NULL;
    }
    if (socket->isWritable()) {
        socket->write(sendMsg, sizeof(DeMessageHead) + header.length);
    }
    else {
        ChatLogInfo()<<"socket is unWritable";
    }
    if(sendMsg!=NULL){
        delete[] sendMsg;
        sendMsg = NULL;
    }
}

/**
 * @brief 发送请求
 */
void Widget::getGroupList()
{
    /*获取群聊列表*/
    writeMsg(NULL,0,CommandEnum_GetGroupList);
    /*获取群成员信息*/
    GetGroupInfoReq getGroupInfoReq;
    getGroupInfoReq.m_Group = 0;
    writeMsg(&getGroupInfoReq,sizeof(GetGroupInfoReq),CommandEnum_GetGroupInfo);
}

void Widget::getFriendList()
{
    writeMsg(NULL,0,CommandEnum_GetFriendInfo);
}


int Widget::handleRegiste(void *msg)
{
    RegistInfoReq* registInfo = (RegistInfoReq*)msg;
    printf("username:%s,password:%s\n",registInfo->m_userName,registInfo->m_password);
    return 0;
}

int Widget::handleLogin(void *msg)
{
    GroupUserInfo* loginInfo = (GroupUserInfo*)msg;
    ChatLogInfo()<<"account:"<<loginInfo->m_account<<",userName:"<<loginInfo->m_userName;

    GroupUserInfo* newGroupUserInfo = (GroupUserInfo*)malloc(sizeof(GroupUserInfo));
    memmove(newGroupUserInfo,msg,sizeof(GroupUserInfo));

    int ret = ChatInfoManage::getInstance()->addGroupUserInfo(0,newGroupUserInfo);
    if(ret == 0){
        ChatLogInfo()<<"新用户上线了，保存用户信息..";
    }
    else {
        ChatLogInfo()<<"Insert new userInfo fail";
    }
    return 0;
}

int Widget::handleLogout(void *msg)
{
    Q_UNUSED(msg)
    return 0;
}

int Widget::handleGroupChat(void *msg)
{
    GroupChatReq* groupChatReq = (GroupChatReq*)msg;
    ChatLogInfo()<<"群聊账号:"<<groupChatReq->m_GroupAccount<<"msg size:"<<groupChatReq->m_msgLen;
    //获取聊天群信息
    GroupChatInfo* groupChatInfo =ChatInfoManage::getInstance()->getGroupChatInfo(groupChatReq->m_GroupAccount);
    if(groupChatInfo == NULL)
    {
        ChatLogInfo()<<"not find chatgroup indo";
    }
    if(groupChatReq->m_msgLen <= 0){
        ChatLogInfo()<<"msgLen:"<<groupChatReq->m_msgLen;
        return 0;
    }
    //从群用户信息总表中获取用户信息
    char*buf = (char*)malloc(groupChatReq->m_msgLen + 1);
    memmove(buf,(char*)msg + sizeof(GroupChatReq),groupChatReq->m_msgLen);
    buf[groupChatReq->m_msgLen] = '\0';

    GroupUserInfo* userInfo = ChatInfoManage::getInstance()->getGroupUserInfo(groupChatReq->m_GroupAccount,groupChatReq->m_UserAccount);
    if(groupChatInfo!=NULL&& userInfo!=NULL)
    {
        mapChatWidget::iterator iter = m_chatWigetMap.find(groupChatReq->m_GroupAccount);
        if(iter!=m_chatWigetMap.end()){
            QListWidget* chatWidget = iter->second;
//            QListWidgetItem* item = new QListWidgetItem;

//            //这里转码，中文名称显示乱码
//            item->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + QString("[%1][%2]:%3").arg(userInfo->m_account)
//                          .arg(QString(userInfo->m_userName)).arg(buf));
//            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//            chatWidget->addItem(item);

            // 用户自己发送的消息
            QString time = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
            ChatMessage *message = new ChatMessage(chatWidget);
            QListWidgetItem *item = new QListWidgetItem();
            dealMessageTime(chatWidget,time);
            dealMessage(chatWidget,message, item, buf, time, userInfo->m_userName ,ChatMessage::User_She);
            // 设置消息发送成功
            message->setTextSuccess();

            ChatLogInfo()<<"["<<groupChatInfo->m_groupName<<"]"<<"["<<userInfo->m_userName<<"]"<<buf;
        }
    }
    return 0;
}

int Widget::handlePrivateChat(void *msg)
{
    int ret = 0;
    ChatLogInfo()<<"recv a privateChat Msg..";
    PrivateChatReq* privateChatReq = (PrivateChatReq*)msg;
    //获取聊天群信息
    FriendInfo* friendInfo = ChatInfoManage::getInstance()->getFriendInfo(privateChatReq->m_UserAccount);
    if(friendInfo==NULL){
        printf("not find friend\n");
    }

    if(privateChatReq->m_msgLen <= 0){
        ChatLogInfo()<<"msgLen:"<<privateChatReq->m_msgLen;
        return 0;
    }

    char*buf = (char*)malloc(privateChatReq->m_msgLen + 1);
    memset(buf,'\0',privateChatReq->m_msgLen + 1);
    memmove(buf,(char*)msg + sizeof(PrivateChatReq),privateChatReq->m_msgLen);

    //从群用户信息总表中获取用户信息
    GroupUserInfo* userInfo = ChatInfoManage::getInstance()->getUserInfo(privateChatReq->m_UserAccount);
    if(userInfo!=NULL){

        mapChatWidget::iterator iter = m_chatWigetMap.find(userInfo->m_account);
        if(iter!=m_chatWigetMap.end()){
            QListWidget* chatWidget = iter->second;
//            QListWidgetItem* item = new QListWidgetItem;
            //这里转码，中文名称显示乱码
//            item->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + QString("[%1][%2]:%3").arg(friendInfo->m_account)
//                          .arg(QString(friendInfo->m_userName)).arg(buf));
//            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//            chatWidget->addItem(item);

            QString time = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
            ChatMessage *message = new ChatMessage(chatWidget);
            QListWidgetItem *item = new QListWidgetItem();
            dealMessageTime(chatWidget,time);
            dealMessage(chatWidget,message, item, buf, time, friendInfo->m_userName ,ChatMessage::User_She);
            // 设置消息发送成功
            message->setTextSuccess();

            ChatLogInfo()<<"["<<friendInfo->m_userName<<"]"<<buf;
        }
    }
    else{
        printf("chatgroup not find..\n");
    }
    return ret;
}

int Widget::getGroupList(void *msg)
{
    int ret = -1;
    GetGroupListResp* pResp = (GetGroupListResp*)msg;
    ChatLogInfo()<<"getGroupList:"<<pResp->m_size;
    for(int i=0,index = 0; i<pResp->m_size; i++,index++){
        GroupChatInfo* curGroupChatInfo =(GroupChatInfo *)((char*)msg + sizeof(GetGroupListResp) + sizeof(GroupChatInfo) * index);
        //添加群信息到群管理
        GroupChatInfo* groupChatInfo = (GroupChatInfo*)malloc(sizeof(GroupChatInfo));
        groupChatInfo->m_account = curGroupChatInfo->m_account;
        groupChatInfo->m_size = curGroupChatInfo->m_size;
        strncpy(groupChatInfo->m_groupName,curGroupChatInfo->m_groupName,sizeof(curGroupChatInfo->m_groupName));
        Init_Group_Info(groupChatInfo);
        ChatLogInfo()<<"group:"<<groupChatInfo->m_account<<"name:"<<groupChatInfo->m_groupName<<",size:"<<groupChatInfo->m_size;
        ret = ChatInfoManage::getInstance()->addGroupChatInfo(groupChatInfo);
        if(ret == 0){
            ChatLogInfo()<<"addGroupChatInfo success...";
        }
    }
    return 0;
}

int Widget::getGroupInfo(void *msg)
{
    int ret = -1;
    GetGroupInfoResp* pResp = (GetGroupInfoResp*)msg;
    mapGroupUserInfo groupUserInfoMap;
    //添加群成员信息到群成员管理
    ChatLogInfo()<<"group:"<<pResp->m_GroupAccount<<",size:"<<pResp->m_size;
    for(int i=0;i<pResp->m_size;i++){
        GroupUserInfo* pUserInfo = (GroupUserInfo*)((char*)msg + sizeof(GetGroupInfoResp) + sizeof(GroupUserInfo)*i);
        GroupUserInfo* pNewUserInfo = (GroupUserInfo*)malloc(sizeof(GroupUserInfo));
        memmove(pNewUserInfo,pUserInfo,(sizeof(GroupUserInfo)));

        if(pNewUserInfo->m_account == m_userInfo.m_account){
            strncpy(m_userInfo.m_userName,pNewUserInfo->m_userName,sizeof(pNewUserInfo->m_userName));
        }

        ret = ChatInfoManage::getInstance()->addGroupUserInfo(pResp->m_GroupAccount,pNewUserInfo);
        if(ret == 0){
            ChatLogInfo()<<"account:"<<pUserInfo->m_account<<",name:"<<pUserInfo->m_userName<<"right:"<<pUserInfo->m_right<<",size:"<<groupUserInfoMap.size();
        }
    }
    return 0;
}

int Widget::getFriendInfo(void *msg)
{
    GetFriendInfoResp* pResp = (GetFriendInfoResp*)msg;
    ChatLogInfo()<<"friend size:"<<pResp->m_size;
    for(int i=0;i<pResp->m_size;i++){
        FriendInfo* pFriendInfo = (FriendInfo*)((char*)msg + sizeof(GetFriendInfoResp) + sizeof(FriendInfo)*i);
        FriendInfo* pNewFriendInfo = (FriendInfo*)malloc(sizeof(FriendInfo));
        memmove(pNewFriendInfo,pFriendInfo,(sizeof(FriendInfo)));
        //好友 map 表中插入好友数据
        ChatInfoManage::getInstance()->addFriendInfo(pNewFriendInfo);
        ChatLogInfo()<<"Friend account:"<<pNewFriendInfo->m_account<<",name:"<<pNewFriendInfo->m_userName;
        Init_Friend_Info(pNewFriendInfo);
    }
    return 0;
}

//其他客户端给你发送添加好友请求
int Widget::handleAddFriendReq(void *msg)
{
    AddFriendInfoReq* pAddFriendInfoReq = (AddFriendInfoReq*)msg;
    LOGINFO("get a add friend request..\n");

    GroupUserInfo *pUserInfo = ChatInfoManage::getInstance()->getUserInfo(pAddFriendInfoReq->m_senderAccount);
    if(pUserInfo!=NULL){
        ChatLogInfo()<<"account:"<< pAddFriendInfoReq->m_senderAccount <<",m_reqInfo:"<< pAddFriendInfoReq->m_reqInfo;
        //将等待添加的好友添加到 map 表中
        FriendInfo *pFriendInfo = (FriendInfo *)malloc(sizeof(FriendInfo));
        pFriendInfo->m_account = pUserInfo->m_account;
        pFriendInfo->m_status = 1;  //Linux命令行客户端直接同意
        strncpy(pFriendInfo->m_userName, pUserInfo->m_userName, sizeof(pUserInfo->m_userName));
        ChatInfoManage::getInstance()->addFriendInfo(pFriendInfo);
        Init_Friend_Info(pFriendInfo);

        //给等待添加的好友发送添加好友的响应
        AddFriendInfoResp resp;
        resp.m_friendAccount = pAddFriendInfoReq->m_friendAccount;
        resp.m_senderAccount = pAddFriendInfoReq->m_senderAccount;
        resp.status = 0;   //同意
        writeMsg(&resp, sizeof(AddFriendInfoResp), CommandEnum_AddFriend,0,2);
    }
    else if(pUserInfo == NULL){
        ChatLogInfo()<<"该用户不存在";
    }
}

int Widget::handleAddFriendResp(void *msg)
{
    //处理好友请求后对方给的响应
    AddFriendInfoResp* pResp = (AddFriendInfoResp*)msg;
    GroupUserInfo* userInfo = ChatInfoManage::getInstance()->getUserInfo(pResp->m_friendAccount);
    if(pResp->status == 0){
        //对方同意添加好友
        FriendInfo*friendInfo = (FriendInfo*)malloc(sizeof(FriendInfo));
        friendInfo->m_account = pResp->m_friendAccount;
        strncpy(friendInfo->m_userName,userInfo->m_userName,sizeof(userInfo->m_userName));
        friendInfo->m_status = 1;   //1同意
        ChatInfoManage::getInstance()->addFriendInfo(friendInfo);
        Init_Friend_Info(friendInfo);
        ChatLogInfo()<<"account:"<<friendInfo->m_account<<",name:"<<friendInfo->m_userName;
    }
    else {
        ChatLogInfo()<<"对方拒绝了..";
        return -1;
    }
    return 0;
}

void Widget::dealMessage(QListWidget* listWidget,ChatMessage *messageW, QListWidgetItem *item, QString text, QString time, QString ip ,ChatMessage::User_Type type) //用户发送文本
{
    listWidget->addItem(item);
    messageW->setFixedWidth(listWidget->width());
    QSize size = messageW->fontRect(text);
    item->setSizeHint(size);
    messageW->setText(text, time, size, ip, type);
    listWidget->setItemWidget(item, messageW);
    return;
}
void Widget::dealMessageTime(QListWidget* listWidget,QString curMsgTime) //处理时间
{
    bool isShowTime = false;
    if(listWidget->count() > 0) {
        QListWidgetItem* lastItem = listWidget->item(listWidget->count() - 1);
        ChatMessage* messageW = (ChatMessage *)listWidget->itemWidget(lastItem);
        int lastTime = messageW->time().toInt();
        int curTime = curMsgTime.toInt();
        qDebug() << "curTime lastTime:" << curTime - lastTime;
        isShowTime = ((curTime - lastTime) > 60); // 两个消息相差一分钟
//        isShowTime = true;
    } else {
        isShowTime = true;
    }
    if(isShowTime) {
        ChatMessage* messageTime = new ChatMessage(listWidget);
        QListWidgetItem* itemTime = new QListWidgetItem();
        listWidget->addItem(itemTime);
        QSize size = QSize(listWidget->width() , 40);
        messageTime->resize(size);
        itemTime->setSizeHint(size);
        messageTime->setText(curMsgTime, curMsgTime, size);
        listWidget->setItemWidget(itemTime, messageTime);
    }
    return;
}

void Widget::on_pushBtn_send_clicked()
{
    ChatLogInfo()<<"on_pushBtn_send_clicked";
    if(ui->textEdit->toPlainText().isEmpty()){
        ChatLogInfo()<<"Msg is null";
        return;
    }
//    int currentRow = ui->listWidget_info->currentRow();
    int currentRow = this->m_curCheckedUser;
    if(currentRow < 0){
        ChatLogInfo()<<"未选择聊天窗口";
        return;
    }
    int isfind = 0;
    int currentIndex = 0;
    QListWidget* chatWidget = NULL;
    chatWidgetInfo* chatInfo = NULL;
    for(listChatWidgetInfo::iterator iter = m_chatWidgetInfoList.begin();iter!=m_chatWidgetInfoList.end();iter++,currentIndex++){
        if(currentIndex == currentRow){
            chatInfo = *iter;
            isfind = 1;
        }
        else {

        }
    }
    if(isfind){
    }
    else {
        ChatLogInfo()<<"---------notfind----------";
        return ;
    }
    mapChatWidget::iterator iter = m_chatWigetMap.find(chatInfo->m_account);
    if(iter!=m_chatWigetMap.end()){
        chatWidget = iter->second;
    }
    else {
        ChatLogInfo()<<"not find..";
        return;
    }

    QString str = ui->textEdit->toPlainText();
    if(str.toStdString().size() > MAX_SEND_LENGTH){
        ChatLogInfo()<<"超出最大发送大小..";
        return;
    }
    ui->textEdit->clear();
//    QString msg;
//    msg.sprintf("[%d][%s]%s",m_userInfo.m_account,m_userInfo.m_userName,str.toStdString().c_str());
//    chatWidget->addItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + msg);

    // 用户自己发送的消息
    QString time = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
    ChatMessage *message = new ChatMessage(chatWidget);
    QListWidgetItem *item = new QListWidgetItem();
    dealMessageTime(chatWidget,time);
    dealMessage(chatWidget,message, item, str, time, m_userInfo.m_userName ,ChatMessage::User_Me);
    // 设置消息发送成功
    message->setTextSuccess();


    ChatLogInfo()<<"["<<m_userInfo.m_userName<<"]["<<m_userInfo.m_account<<"]"<<str<<",size:"<<str.toStdString().size();
    //发送消息
    if(chatInfo->m_type == TYPE_GROUP_CHAT){
        //群聊
        GroupChatReq *groupChatReq;
        char *p = (char *)malloc(sizeof(GroupChatReq) + str.toStdString().size());
        groupChatReq = (GroupChatReq *)p;
        groupChatReq->m_UserAccount = m_userInfo.m_account;
        groupChatReq->m_type = 0;
        groupChatReq->m_GroupAccount = 0;
        groupChatReq->m_msgLen = str.toStdString().size();
        memmove(p + sizeof(GroupChatReq), str.toStdString().c_str(), str.toStdString().size());

        writeMsg(p, sizeof(GroupChatReq) + str.toStdString().size(), CommandEnum_GroupChat);
        if(p != NULL){
            free(p);
            p = NULL;
        }
    }
    else if(chatInfo->m_type == TYPE_PRIVATE_CHAT){
        //私聊
        PrivateChatReq *privateChatReq;
        char *p = (char *)malloc(sizeof(PrivateChatReq) + str.toStdString().size());
        privateChatReq = (PrivateChatReq *)p;

        privateChatReq->m_UserAccount = m_userInfo.m_account;
        privateChatReq->m_type = 0;
        privateChatReq->m_FriendAccount = chatInfo->m_account;
        privateChatReq->m_msgLen = str.toStdString().size();
        memmove(p + sizeof(PrivateChatReq), str.toStdString().c_str(), str.toStdString().size());
        writeMsg(p, sizeof(PrivateChatReq) + str.toStdString().size(), CommandEnum_PrivateChat);
        if(p != NULL){
            free(p);
            p = NULL;
        }
    }
}

void Widget::SlotUserGroupButtonClicked(int index)
{
    m_curCheckedUser = index;
    qDebug()<<"checked index:"<<index;
    ui->stackedWidget_Msg->setCurrentIndex(m_curCheckedUser);
    ChatLogInfo()<<"current Row chicked.."<<m_curCheckedUser;
}

int Widget::handleMsg(recvMsg *rMsg)
{
    DeMessagePacket *packet = (DeMessagePacket *)(rMsg->body);

    DeMessageHead *head = (DeMessageHead *)(rMsg->head);
//    ChatLogInfo()<<"mark:"<<head->mark<<",encoded:"<<head->encoded<<",length:"<<head->length<<",version:"<<head->version;
//    ChatLogInfo()<<"command:"<<packet->command<<",error:"<<packet->error<<",mode:"<<packet->mode<<",sequence:"<<packet->sequence;

//    void *reqData = NULL;
//    int reqDataLen = -1;
//    short ret = RET_OK;

    ChatLogInfo()<<"handleMsg:"<<packet->command<<",str:"<<getChatCmdString(packet->command);
    switch (packet->command)
    {
        case CommandEnum_Registe:
            // handleRegiste(rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_Login:
            if(packet->mode == 3){
                handleLogin(rMsg->body + sizeof(DeMessagePacket));
            }else if(packet->mode == 2 && packet->error == 0){
                m_isLogin = 1;
                ChatLogInfo()<<"登陆成功..";
            }
            break;
        case CommandEnum_Logout:
            // handleLogout(this, rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_GroupChat:
            handleGroupChat(rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_AddFriend:
            if(packet->mode == 1){
                //其他客户端给你发送添加好友请求
                handleAddFriendReq(rMsg->body + sizeof(DeMessagePacket));
            }else if(packet->mode == 2){
                if(head->length - sizeof(DeMessagePacket) == 0)
                {
                    //这个是发送好友请求后服务器返回的状态
                    if(packet->error == 0){
                        ChatLogInfo()<<"addFriend Request success..";
                    }
                    else {
                        ChatLogInfo()<<"addFriend Request fail,error:"<<packet->error;
                    }
                }
                else
                    handleAddFriendResp(rMsg->body + sizeof(DeMessagePacket));
            }
            break;
        case CommandEnum_delFriend:
            break;
        case CommandEnum_PrivateChat:
            handlePrivateChat(rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_CreateGroup:
            break;
        case CommandEnum_GetGroupList:
            getGroupList(rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_GetGroupInfo:
            getGroupInfo(rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_GetFriendInfo:
            getFriendInfo(rMsg->body + sizeof(DeMessagePacket));
            break;
    }
    return 0;
}

void Widget::on_pushButton_addFriend_clicked()
{
    AddFriendDialog* addFriendDlg = new AddFriendDialog();

    addFriendDlg->show();
    if(addFriendDlg->exec() == QDialog::Accepted)
    {
        AddFriendInfoReq addFriendInfoReq = addFriendDlg->getAddFriendInfoReq();
        addFriendInfoReq.m_senderAccount = m_userInfo.m_account;
        ChatLogInfo()<<"friend account:"<<addFriendInfoReq.m_friendAccount;
        ChatLogInfo()<<"Req Info:"<<addFriendInfoReq.m_reqInfo;
        writeMsg((char*)&addFriendInfoReq,sizeof(AddFriendInfoReq),CommandEnum_AddFriend);
    }
}

//void Widget::on_listWidget_info_itemClicked(QListWidgetItem *item)
//{
//    int currentRow = ui->listWidget_info->currentRow();
//    ui->stackedWidget_Msg->setCurrentIndex(currentRow);
//    ChatLogInfo()<<item->text()<<"current Row chicked.."<<currentRow;

//}

void Widget::on_pushBtn_close_clicked()
{
    this->close();
}

void Widget::on_pushBtn_hide_clicked()
{
    QWidget* pWindow = this->window();
    if(pWindow->isTopLevel())
        pWindow->showMinimized();
}

