#include "addfrienddialog.h"
#include "ui_addfrienddialog.h"

AddFriendDialog::AddFriendDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFriendDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("WeChat 查找");
    ui->widget_friendInfo->hide();
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint); // 最小化按钮
    setWindowFlags(windowFlags() | Qt::WindowContextHelpButtonHint); // 帮助按钮
}

AddFriendDialog::~AddFriendDialog()
{
    delete ui;
}

void AddFriendDialog::on_pushButton_find_clicked()
{
    ChatLogInfo()<<"on_pushButton_find_clicked in..";
    int account = ui->lineEdit_account->text().toInt();
    /**
     * @brief 因为要维护一个大群（群号为0），所有用户在线都在里面，查找好友暂定在大群里面查找
     *        后期为了应对高并发会从服务器获取好友信息
     */
    GroupUserInfo* userInfo;
    userInfo = ChatInfoManage::getInstance()->getUserInfo(account);
    if(userInfo!= NULL){
        ui->widget_friendInfo->show();
        //找到该用户，显示用户信息
        m_addFriendInfoReq.m_friendAccount = userInfo->m_account;           //保存账号
        m_addFriendInfoReq.m_senderAccount = -1;
        ui->lineEdit_friendName->setText(QString(userInfo->m_userName));
        ui->lineEdit_friend_account->setText(QString::number(userInfo->m_account));
    }
    else {
        ui->widget_friendInfo->hide();
    }
    ChatLogInfo()<<"on_pushButton_find_clicked out..";
}

void AddFriendDialog::on_pushButton_addFriend_clicked()
{
    memset(m_addFriendInfoReq.m_reqInfo,'\0',sizeof(m_addFriendInfoReq.m_reqInfo));
    strncpy(m_addFriendInfoReq.m_reqInfo,ui->textEdit_reqInfo->toPlainText().toStdString().c_str(),ui->textEdit_reqInfo->toPlainText().toStdString().size());
    return accept();    //Closes the dialog and emits the accepted() signal.
}
