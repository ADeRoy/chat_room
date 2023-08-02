#ifndef TOOLBOXPOW_H
#define TOOLBOXPOW_H

#include <QHBoxLayout>
#include <QObject>
#include <QWidget>
#include <QScrollArea>
#include <QPushButton>
#include <QMap>
#include <QListWidget>
#include <QTimer>
#include <QButtonGroup>

/**
 * @brief BoxGroup 由按钮组Button(一级菜单)+QListWidget(二级菜单)组成
 */
class BoxGroup:public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 初始化的时候
     */
    BoxGroup(QWidget *parent = nullptr);
    ~BoxGroup();
private slots:
    void on_pushbutton_checked();
public:
    void setGroupName(QString str){
        m_button->setText(str);
    }
    // 设置没有组的列表，只显示成员
    int setGroupHeadStatue(bool showStus);
    // 刷新状态
    void fflushStatus();
    void resetStatus();
    void setChicked();
    void addWidget(QWidget* widget);
    void addWidget(QWidget* widget, int height);
    int getHeight(){ return m_widgetHeight; }
public:
    QPushButton* getButton(){return m_button;}
    QWidget* getWidget(){return m_listWidget;}
private:
    QPushButton* m_button;
    QListWidget* m_listWidget;
    QButtonGroup* m_buttonGroup;    // 用户组集合
    int m_widgetHeight;
};

class ToolBoxPow : public QWidget
{
    Q_OBJECT
public:
    ToolBoxPow(QWidget *parent = nullptr);
    ~ToolBoxPow();
public:
    void AddGroup(QString groupName);
    int  setGroupHeadVisible(QString groupName,bool isVisible);
    void AddWXWidget(QString name,QString path);
    void AddQQWidget(QString groupName,QString name,QString path);
    void AddButton(QString groupName,QPushButton* widget);
    QString getUserName(){
        QMap<QString, BoxGroup *>::iterator ite = buttonMap.find("userlist");
        if(ite!=buttonMap.end())
        {
            BoxGroup* boxGroup = *ite;
//            boxGroup->addWidget(widget);
//            m_buttonUserGroup->addButton(widget,m_buttonUserIndex++);
        }
        return QString("test");
    }
public:
    // 设置排他属性，true:只有一个组展开 false:所以组可以同时展开
    int setExclusive(bool status){ return m_exclusive = status; };
private:
    QVBoxLayout* getLayout(){return verticalLayout;}
signals:
    // 用户组按钮单击信号，例如家人被点击了触发信号
    void SignalGroupButtonClicked(int index);
    // 用户按钮单击信号，例如家人被点击了触发信号
    void SignalUserGroupButtonClicked(int Index);
private slots:
    // 分组按钮点击响应槽函数
    void SlotGroupButtonClicked(int index); //这个是组按钮按下后响应槽函数
    // 用户按钮点击槽函数
    void SlotUserGroupButtonClicked(int index); //这个是组按钮按下后响应槽函数
private:
    QGridLayout* mainGridLayout;
    QScrollArea* scrollArea;
    QVBoxLayout* verticalLayout;
    QGridLayout* gridLayout;        //格子布局
    QWidget *    m_fillWidget;      //填充矩形
    int m_height;
    int m_currentIndex;     //当前点击的下标
    int m_exclusive;        //是否排他
    QWidget* scrollAreaWidgetContents;
    QMap<QString, BoxGroup*> buttonMap;
    QButtonGroup* m_buttonGroup;        //这个是组按钮的group集合组  ，例如家人，朋友
    QButtonGroup* m_buttonUserGroup;    //这个是用户组按钮的group集合组，例如张三，李四
    int m_buttonUserIndex;
};


#endif // TOOLBOXPOW_H
