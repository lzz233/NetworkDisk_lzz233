#include "tcpserver.h"
#include "ui_tcpserver.h"
#include <QByteArray>
#include <QDebug> // 调试
#include <QMessageBox> // 消息提示框
#include <QHostAddress>
#include "mytcpserver.h" // QTcpServer派生类实现监听和接收客户端的TCP连接

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);

    loadConfig(); // 调用访问配置信息
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort); // 监听客户端的连接请求
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config"); // 文件对象，读取资源文件 ':' + "前缀" + "文件名"
    if(file.open(QIODevice::ReadOnly)) // file.open() 参数：打开方式：只读（注意，这里只读是写在QIODevice下的枚举，所以调用要声明命名空间） 返回true则打开成功
    {
        QByteArray baData = file.readAll(); // 读出所有数据，返回字节数组QByteArray
        QString strData(baData);
        file.close();

        strData.replace("\r", " ");
        strData.replace("\n", ""); // 替换IP地址、端口号与服务器文件系统根地址之间\r\n
        QStringList strList = strData.split(" ");

        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort(); // 无符号短整型
        qDebug() << "IP: " << m_strIP << " port: " << m_usPort; // 打印结果。当你使用qDebug()输出QString类型的变量时，qDebug()会自动将字符串内容用引号括起来。这是为了更清晰地显示字符串的内容

    }else{
        QMessageBox::critical(this, "open config", "open config failed");
    }
}



