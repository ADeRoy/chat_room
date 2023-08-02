#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>
#include <QMouseEvent>
#include <windows.h>
#include <windowsx.h>
#include "registdlg.h"

namespace Ui {
class LoginDlg;
}

class LoginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDlg(QWidget *parent = nullptr);
    ~LoginDlg();
public:
    UserInfo* getUserInfo(){
        return &m_userInfo;
    }
private:
    void Init();
protected:
    // Event handlers
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
public:
    QPoint mouseWindowTopLeft; //鼠标相对窗口左上角的坐标         在mousePressEvent 得到
    QPoint mouseDeskTopLeft;   //鼠标相对于桌面左上角坐标         在mouseMoveEvent实时获取
    QPoint windowDeskTopLeft;  //窗口左上角相对于桌面左上角坐标    在mouseMoveEvent实时计算(矢量)获得
private slots:
    void on_pushbtn_regist_clicked();

    void on_pushButton_login_clicked();

    void on_pushBtn_hide_clicked();

    void on_pushBtn_close_clicked();

private:
    Ui::LoginDlg *ui;
    UserInfo m_userInfo;
};

#endif // LOGINDLG_H
