#include "widget.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w(argc, argv);
    w.show();
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));  //这一行代码连接了Qt应用程序对象app的lastWindowClosed信号和quit槽。这意味着当应用程序的最后一个窗口关闭时，应用程序将退出。

    return a.exec();
}
