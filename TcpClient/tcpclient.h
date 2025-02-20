#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile> // 操作文件，在这里主要为了启动时访问配置文件
#include <QTcpSocket> // 建立TCP会话连接，为了实现TCP客户端连接服务器及与数据库交互
#include "protocol.h"
#include "opewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig(); // 加载配置文件信息

    static TcpClient &getInstance(); // 创建静态的客户端单例，谁需要客户端就调用一下
    QTcpSocket &getTcpSocket(); // 返回m_tcpSocket的引用。不管哪里用到socket，直接调用就可以收发数据

    QString getLoginName(); // 获得登陆名字接口
    QString getCurPath(); // 获得登陆时工作目录接口
    void setCurPath(QString strCurPath); // 修改工作目录

public slots:
    void showConnect();
    void recvMsg();

private slots:
    // void on_send_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

    void on_login_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;        // 存储配置文件读取到的IP地址
    quint16 m_usPort;       // 无符号16位整型 存储配置文件的端口号

    // 连接服务器，和服务器进行数据交互
    QTcpSocket m_tcpSocket;
    QString m_strLoginName; // 记录登录时的名字

    QString m_strCurPath; // 记录当前路径
    QFile m_file;
};
#endif // TCPCLIENT_H
