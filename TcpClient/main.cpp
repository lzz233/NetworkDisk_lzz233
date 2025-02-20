#include "tcpclient.h"
#include <QApplication>
#include "sharefile.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font("Times", 24, QFont::Bold); // 设置字体
    a.setFont(font);

    // TcpClient w;
    // w.show();
    TcpClient::getInstance().show();

    // Book w;
    // w.show();
    // ShareFile w;
    // w.show();

    return a.exec(); // a 是上面 QApplication 对象的实例
}
