#ifndef ADDFRIENDDIALOG_H
#define ADDFRIENDDIALOG_H

#include "common.h"
#include "widget.h"
#include <QDialog>

namespace Ui {
class AddFriendDialog;
}

class AddFriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFriendDialog(QWidget *parent = nullptr);
    ~AddFriendDialog();
public:
    AddFriendInfoReq getAddFriendInfoReq(){
        return m_addFriendInfoReq;
    }
private slots:
    void on_pushButton_find_clicked();

    void on_pushButton_addFriend_clicked();
signals:
    void signal_findUserInfo(int account);
private:
    Ui::AddFriendDialog *ui;
    AddFriendInfoReq m_addFriendInfoReq;
};

#endif // ADDFRIENDDIALOG_H
