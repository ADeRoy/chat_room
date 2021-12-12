#include "registdlg.h"
#include "ui_registdlg.h"

#include <QGraphicsDropShadowEffect>

RegistDlg::RegistDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegistDlg)
{
    ui->setupUi(this);
    Init();
}

RegistDlg::~RegistDlg()
{
    delete ui;
}

void RegistDlg::mousePressEvent(QMouseEvent *event)
{
    mouseWindowTopLeft = event->pos();
}

void RegistDlg::mouseMoveEvent(QMouseEvent *event)
{

    //窗口移动
    if (event->buttons() & Qt::LeftButton)
    {
        mouseDeskTopLeft = event->globalPos();
        windowDeskTopLeft = mouseDeskTopLeft - mouseWindowTopLeft;  //矢量计算
        this->move(windowDeskTopLeft);     //移动到目的地
    }
}

void RegistDlg::disconnectedSlot()
{
    ChatLogInfo()<<"close socket...";
}

void RegistDlg::readyReadSlot()
{
    DeMessageHead header;
    /*接收注册响应*/
    memset(&header,'\0',sizeof(DeMessageHead));
    int len = socket->read((char*)&header,sizeof(DeMessageHead));
    ChatLogInfo()<<"readLen:"<<len;
    ChatLogInfo()<<"length:"<<header.length;
    char *p = (char*)malloc(header.length);
    DeMessagePacket* pPacket = (DeMessagePacket *)p;
    socket->read((char*)pPacket,header.length);

    if(pPacket->error == 0){
        m_status = true;
    }

    RegistInfoResp* resp = (RegistInfoResp*)(p+sizeof(DeMessagePacket));
    ChatLogInfo()<<"account:"<<resp->m_account;

    userInfo.m_account = resp->m_account;
    strncpy(userInfo.m_userName,ui->lineEdit_username->text().toStdString().c_str(),ui->lineEdit_username->text().toStdString().size());
    strncpy(userInfo.m_password,ui->lineEdit_password->text().toStdString().c_str(),ui->lineEdit_password->text().size());

    free(p);
    return accept();    //Closes the dialog and emits the accepted() signal.
}

void RegistDlg::Init()
{
    this->setWindowTitle("WeChat 注册");
    memset(&userInfo,'\0',sizeof (userInfo));
    m_status = false;
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint); // 最小化按钮
    setWindowFlags(windowFlags() | Qt::WindowContextHelpButtonHint); // 帮助按钮

    int width = this->width()-10;
    int height = this->height()-10;
    ui->centerWidget->setGeometry(5,5,width,height);
    ui->centerWidget->setStyleSheet("QWidget{border-radius:4px;background:rgba(255,255,255,1);}");  //设置圆角

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
}

void RegistDlg::on_pushBtn_regist_clicked()
{
    socket = new QTcpSocket;
    QString ipAddressStr = SERVER_ADDR;
    quint16 port = SERVER_PORT;

    socket->connectToHost(ipAddressStr, port);
    if(socket->waitForConnected(3000))
    {
        ChatLogInfo() << "Connect Server success";
    }
    else
    {
        ChatLogInfo() << socket->errorString();
        return;
    }

    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnectedSlot()));     //客户端断开连接
    connect(socket, SIGNAL(readyRead()),this, SLOT(readyReadSlot()));           //接收消息

    RegistInfoReq info;
    memset(&info,'\0',sizeof(RegistInfoReq));
    strncpy(info.m_userName,ui->lineEdit_username->text().toStdString().c_str(),ui->lineEdit_username->text().toStdString().size());
    strncpy(info.m_password,ui->lineEdit_password->text().toStdString().c_str(),ui->lineEdit_password->text().size());
    writeMsg(&info,sizeof(RegistInfoReq),CommandEnum_Registe);
}

void RegistDlg::writeMsg(void *buf, int bufLen, int type)
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
    socket->write(sendMsg, sizeof(DeMessageHead) + header.length);
    delete[] sendMsg;
}

void RegistDlg::on_pushBtn_hide_clicked()
{
    QWidget* pWindow = this->window();
    if(pWindow->isTopLevel())
        pWindow->showMinimized();
}

void RegistDlg::on_pushBtn_close_clicked()
{
   this->close();
}
