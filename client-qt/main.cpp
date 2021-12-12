#include "widget.h"
#include "common.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    if(w.getLoginStatus() == false)
        return 0;
    w.show();
    return a.exec();
}
