#include "custom/ToolBoxPow.h"
#include <qDebug>
#include <QScrollBar>

#include <QLabel>

#define BUTTON_DOWN "QPushButton{"\
"border-style: none;"\
"text-align : left;"\
"color: rgb(81,90,110);"\
"background: rgb(242,244,247);"\
"background-image: url(:/src/arrow_down_normal.png);"\
"background-repeat: repeat-no-repeat;"\
"background-position: right;"\
"background-origin:content;"\
"padding-left:30px;"\
"padding-right:20px;"\
"}"

#define BUTTON_RIGHT "QPushButton{"\
"border-style: none;"\
"text-align : left;"\
"color: rgb(81,90,110);"\
"background: rgb(242,244,247);"\
"background-image: url(:/src/arrow_right_normal.png);"\
"background-repeat: repeat-no-repeat;"\
"background-position: right;"\
"background-origin:content;"\
"padding-left:30px;"\
"padding-right:20px;"\
"}"

ToolBoxPow::ToolBoxPow(QWidget *parent)
    :QWidget(parent)
    , m_buttonUserIndex(0)
{
    m_height = 0;
    m_currentIndex = -1;
    m_exclusive = true;
    m_fillWidget = new QWidget;

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);   //设置滚动
    //幕布
    scrollAreaWidgetContents = new QWidget;
    //格子布局管理器   他的父亲是幕布
    gridLayout = new QGridLayout(scrollAreaWidgetContents);
    gridLayout->setSpacing(0);  //设置内边距
    gridLayout->setContentsMargins(0, 0, 0, 0);
    //垂直布局管理器
    verticalLayout = new QVBoxLayout();
    verticalLayout->setMargin(0); //设置外边距
    verticalLayout->setSpacing(0);//设置内边距
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

    //将垂直布局管理器添加给格子布局管理器
    gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);
    //给滚动区域添加 widget
    scrollArea->setWidget(scrollAreaWidgetContents);      //设置幕布

    //    //竖滚动条和横滚动条都可以一直显示
    //    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    //设置滑动条的样式
    scrollArea->setStyleSheet("QScrollArea{border:0px solid;"
                            "border-radius: 2px;}"
                            "QScrollBar:vertical{width: 4px;border:0px solid;"
                            "border-radius: 2px;margin: 0px,0px,0px,0px;"
                            "background-color: transparent;background:#e1f0fa;}"
                            "QScrollBar:vertical:hover{width: 4px;"
                            "border:0px solid;margin: 0px,0px,0px,0px;background:#e1f0fa;}"
                            "QScrollBar::handle:vertical{width:4px;background:#c0ddee;"
                            "border-radius: 2px;height: 40px;}"
                            "QScrollBar::handle:vertical:hover{background:#c0ddee;"
                            "border-radius: 2px;}"
                            "QScrollBar::add-line:vertical{height:11px;background-color: transparent;"
                            "subcontrol-position:bottom;border:0px solid;"
                            "border-radius: 2px;}"
                            "QScrollBar::sub-line:vertical{height:11px;"
                            "background-color: transparent;subcontrol-position:top;"
                            "border:0px solid;border-radius: 2px;}"
                            "QScrollBar::add-page:vertical{background-color: #e1f0fa;"
                            "border:0px solid;border-radius: 2px;}"
                            "QScrollBar::sub-page:vertical{background-color: #e1f0fa;"
                            "border:0px solid;border-radius: 2px;}"
                            "QScrollBar::up-arrow:vertical{"
                            "border:0px solid;border-radius: 3px;}"
                            "QScrollBar::down-arrow:vertical {"
                            "border:0px solid;border-radius: 3px;}");
    mainGridLayout = new QGridLayout(this);
    mainGridLayout->setMargin(0); //设置外边距
    mainGridLayout->setSpacing(0);//设置内边距
    mainGridLayout->addWidget(scrollArea);
    //在按钮组的集合中某一个按钮组按下响应
    m_buttonGroup = new QButtonGroup;
    connect(m_buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(SlotGroupButtonClicked(int)));
    m_buttonUserGroup = new QButtonGroup;
    connect(m_buttonUserGroup,SIGNAL(buttonClicked(int)),this,SLOT(SlotUserGroupButtonClicked(int)));

    // 用户列表头不可见
    this->AddGroup("userlist");
    this->setGroupHeadVisible("userlist",false);
}

ToolBoxPow::~ToolBoxPow()
{
    foreach(auto ite,buttonMap)
    {
        if(ite)
        {
            delete ite;
            ite = NULL;
        }
    }
    buttonMap.clear();
    if(m_buttonGroup)
    {
        delete m_buttonGroup;
        m_buttonGroup = NULL;
    }
    if(m_buttonUserGroup)
    {
        delete m_buttonUserGroup;
        m_buttonUserGroup = NULL;
    }
    if(m_fillWidget)
    {
        delete m_fillWidget;
        m_fillWidget = NULL;
    }
    if(mainGridLayout)
    {
        delete mainGridLayout;
        mainGridLayout = NULL;
    }
    if(verticalLayout)
    {
        delete verticalLayout;
        verticalLayout = NULL;
    }
    if(gridLayout)
    {
        delete gridLayout;
        gridLayout = NULL;
    }
    if(scrollArea)
    {
        delete scrollArea;
        scrollArea = NULL;
    }
}
/**
 * @brief ToolBoxPow::AddGroup  添加分组    每个分组由一个PushButton和QListWidget组成
 * @param groupName             分组名为唯一标识变量
 */
void ToolBoxPow::AddGroup(QString groupName)
{
    static int buttonIndex = 0;
    BoxGroup* newGroup = new BoxGroup(scrollAreaWidgetContents);
    newGroup->setGroupName(groupName);
    m_buttonGroup->addButton(newGroup->getButton(),buttonIndex++);
#if 1
    verticalLayout->addWidget(newGroup->getButton(), buttonMap.count());
    verticalLayout->addWidget(newGroup->getWidget(), buttonMap.count() + 1);
    verticalLayout->addWidget(m_fillWidget,buttonMap.count() + 2);
#else
    verticalLayout->insertWidget(0,newGroup->getButton(), buttonMap.count());
    verticalLayout->insertWidget(1,newGroup->getWidget(), buttonMap.count() + 1);
    verticalLayout->insertWidget(2,m_fillWidget,buttonMap.count() + 2);
#endif
    buttonMap.insert(groupName,newGroup);
}

int ToolBoxPow::setGroupHeadVisible(QString groupName,bool isVisible)
{
    QMap<QString, BoxGroup *>::iterator ite = buttonMap.find(groupName);
    if(ite!=buttonMap.end())
    {
        BoxGroup* boxGroup = *ite;
        boxGroup->setGroupHeadStatue(isVisible);
    }
    return 0;
}

void ToolBoxPow::AddWXWidget(QString name, QString path)
{
    QPushButton *widget=new QPushButton(scrollAreaWidgetContents);
    QLabel *touLabel = new QLabel(widget);
    QLabel *namelabel=new QLabel(widget);
    widget->setCheckable(true);
    //设置不同控件的样式
//    widget->setStyleSheet(" background: transparent");
    widget->setStyleSheet("QPushButton{     \
                          border-style:none;    \
                          color: rgb(255, 255, 255);    \
                      } \
                      QPushButton:hover{    \
                          background: rgb(215,215,215);  \
                          border-radius: 0px;           \
                          color: rgb(255, 255, 255);    \
                      }                                 \
                      QPushButton:checked{              \
                          background: rgb(202,200,199);  \
                          color: rgb(255, 255, 255);    \
                      }");
    namelabel->setFixedSize(180,60);
    namelabel->move(60,5);
    namelabel->setText(name);
    namelabel->setAlignment(Qt::AlignLeft);
    namelabel->setAlignment(Qt::AlignVCenter);
    QFont font("微软雅黑",11,50);
    namelabel->setFont(font);
    namelabel->setStyleSheet("color: rgb(70,70,70)");
    touLabel->setFixedSize(40,40);
    touLabel->move(15,15);
    QPixmap pixmap=QPixmap(path);
    QPixmap fitpixmap = pixmap.scaled(40, 40, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    touLabel->setPixmap(fitpixmap);
    touLabel->setScaledContents(true);  //设置图片缩放
    QMap<QString, BoxGroup *>::iterator ite = buttonMap.find("userlist");
    if(ite!=buttonMap.end())
    {
        BoxGroup* boxGroup = *ite;
        boxGroup->addWidget(widget);
        m_buttonUserGroup->addButton(widget,m_buttonUserIndex++);
    }
}

/**
 * @brief ToolBoxPow::AddWidget 给分组添加成员
 * @param groupName 组名
 * @param name      成员名
 * @param path      头像路径
 */
void ToolBoxPow::AddQQWidget(QString groupName, QString name, QString path)
{
    QPushButton *widget=new QPushButton(scrollAreaWidgetContents);
    QLabel *touLabel = new QLabel(widget);
    QLabel *namelabel=new QLabel(widget);
    widget->setCheckable(true);
    //设置不同控件的样式
//    widget->setStyleSheet(" background: transparent");
    widget->setStyleSheet("QPushButton{     \
                          border-style:none;    \
                          color: rgb(255, 255, 255);    \
                      } \
                      QPushButton:hover{    \
                          background: rgb(215,215,215);  \
                          border-radius: 0px;           \
                          color: rgb(255, 255, 255);    \
                      }                                 \
                      QPushButton:checked{              \
                          background: rgb(202,200,199);  \
                          color: rgb(255, 255, 255);    \
                      }");
    namelabel->setFixedSize(180,60);
    namelabel->move(80,10);
    namelabel->setText(name);
    namelabel->setAlignment(Qt::AlignLeft);
    namelabel->setAlignment(Qt::AlignVCenter);
    QFont font("微软雅黑",11,50);
    namelabel->setFont(font);
    namelabel->setStyleSheet("color: rgb(70,70,70)");
    touLabel->setFixedSize(50,50);
    touLabel->move(15,15);
    QPixmap pixmap=QPixmap(path);
    QPixmap fitpixmap = pixmap.scaled(50, 50, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    touLabel->setPixmap(fitpixmap);
    touLabel->setScaledContents(true);  //设置图片缩放
    QMap<QString, BoxGroup *>::iterator ite = buttonMap.find(groupName);
    if(ite!=buttonMap.end())
    {
        BoxGroup* boxGroup = *ite;
        boxGroup->addWidget(widget);
        m_buttonUserGroup->addButton(widget,m_buttonUserIndex++);
    }
}

void ToolBoxPow::AddButton(QString groupName, QPushButton *widget)
{
    QMap<QString, BoxGroup *>::iterator ite = buttonMap.find(groupName);
    if(ite!=buttonMap.end())
    {
        BoxGroup* boxGroup = *ite;
        boxGroup->addWidget(widget);
        // 自定义的添加窗口需要添加 button
        m_buttonUserGroup->addButton(widget,m_buttonUserIndex++);
    }
}
void ToolBoxPow::SlotGroupButtonClicked(int index)
{
    if(m_exclusive && m_currentIndex!=-1 && m_currentIndex!=index)
    {
        //设置排他性，将上次操作归位
        QMap<QString, BoxGroup *>::iterator ite = buttonMap.find(m_buttonGroup->button(m_currentIndex)->text());
        if(ite!=buttonMap.end())
        {
            BoxGroup* boxGroup = *ite;
            boxGroup->resetStatus();
        }
    }
    m_currentIndex = index;

    QMap<QString, BoxGroup *>::iterator ite = buttonMap.find(m_buttonGroup->button(index)->text());
    if(ite!=buttonMap.end())
    {
        BoxGroup* boxGroup = *ite;
        if(boxGroup){
            boxGroup->setChicked();
        }
    }
    emit SignalGroupButtonClicked(index);
}

void ToolBoxPow::SlotUserGroupButtonClicked(int index)
{
    qDebug()<<"user index:"<<index<<",checked.";
    emit SignalUserGroupButtonClicked(index);
}

/**
 * @brief 初始化的时候申请按钮组 m_button 和 listWidget 对象，设置按钮组和 listWidget 样式,绑定按钮组的单击信号
 */
BoxGroup::BoxGroup(QWidget *parent)
{
    m_button = new QPushButton(parent);
    m_listWidget = new QListWidget(parent);
    m_widgetHeight = 0;

    QFont font;
    font.setPointSize(12);  //字体大小
    font.setBold(true);     //加粗
    font.setWeight(75);     //加粗程度
    m_button->setStyleSheet(BUTTON_RIGHT);   //设置样式表
    m_button->setMinimumHeight(50);
    m_button->setFont(font);

    m_listWidget->setFrameShape(QListWidget::NoFrame);  //这个接口可以使滚动条失效
    //先影藏 Widget，因为没有成员
    m_listWidget->hide();
    m_listWidget->setFixedHeight(m_widgetHeight);          //设置 Widget 高度
    connect(m_button,SIGNAL(clicked()),this,SLOT(on_pushbutton_checked()));
}

BoxGroup::~BoxGroup()
{
    qDebug()<<"BoxGroup 析构 begin";
    for(int i=0;i<m_listWidget->count();i++){
        QListWidgetItem *item = m_listWidget->item(i);
        QWidget* widget = m_listWidget->itemWidget(item);
        if(widget)
        {
            delete widget;
            widget = NULL;
        }
        if(item)
        {
            delete item;
            item = NULL;
        }
    }
    m_listWidget->clear();
    if(m_listWidget){
        delete m_listWidget;
        m_listWidget =NULL;
    }
    if(m_button){
        delete m_button;
        m_button = NULL;
    }
    qDebug()<<"BoxGroup 析构 end";
}

/**
 * @brief 按钮组单击之后需要设置listWidget（二级菜单）的状态(显示和隐藏）
 */
void BoxGroup::on_pushbutton_checked()
{
    fflushStatus();
}

int BoxGroup::setGroupHeadStatue(bool showStus)
{
    m_button->setVisible(showStus);
    if(!showStus){
        if(!m_listWidget->isVisible())
        {
            // 头不可见，列表必须可见
            m_listWidget->setVisible(true);
        }
    }
    return 0;
}

void BoxGroup::fflushStatus()
{
    if(m_listWidget->isVisible())
    {
        m_listWidget->setVisible(false);    //or hide() 影藏 Widget
        m_button->setStyleSheet(BUTTON_RIGHT);
    }
    else
    {
        m_listWidget->setVisible(true);
        m_button->setStyleSheet(BUTTON_DOWN);
    }
}

/**
 * @brief 重置状态，由上级控制，用于还原之前的状态
 */
void BoxGroup::resetStatus()
{
    if(m_listWidget->isVisible())
    {
        m_listWidget->setVisible(false);    //or hide() 影藏Widget
        m_button->setStyleSheet(BUTTON_RIGHT);
    }
}

/**
 * @brief 设置 Group 的第一个 PushButton 状态为选中
 */
void BoxGroup::setChicked()
{
    if(m_listWidget->count() <= 0)
        return;
//    QWidget* button = (QWidget*)m_listWidget->itemWidget(m_listWidget->item(0));
    QPushButton* button = (QPushButton*)m_listWidget->itemWidget(m_listWidget->item(0));
    if(button){
//        qDebug()<<"Chicked()";
//        button->setChecked(true);
    }
}

/**
 * @brief 给按钮组（一级菜单）添加按钮（二级菜单）
 */
void BoxGroup::addWidget(QWidget *widget)
{
    QPushButton* button = (QPushButton*)widget;
    QListWidgetItem *item=new QListWidgetItem;
    QSize size = item->sizeHint();              //获取item大小
    item->setSizeHint(QSize(size.width(), 65)); //设置固定高度
    m_listWidget->addItem(item);                //listWidget添加Item
    m_listWidget->setItemWidget(item, widget);  //给Item设置Widget

    size = item->sizeHint();                    //获取item大小
    m_widgetHeight += size.height();
    m_listWidget->setFixedHeight(m_widgetHeight);   //设置 Widget 高度
    button->setChecked(true);
}

/**
 * @brief 给按钮组（一级菜单）添加按钮（二级菜单）并且指定按钮的高度
 */
void BoxGroup::addWidget(QWidget *widget, int height)
{
    QListWidgetItem *item=new QListWidgetItem;
    QSize size = item->sizeHint();                  //获取item大小
    item->setSizeHint(QSize(size.width(), height)); //设置固定高度
    m_listWidget->addItem(item);                    //listWidget添加Item
    m_listWidget->setItemWidget(item, widget);      //给Item添加Widget
    size = item->sizeHint();                        //获取item大小
    m_widgetHeight += size.height();
    m_listWidget->setFixedHeight(m_widgetHeight);          //设置 Widget 高度
}
