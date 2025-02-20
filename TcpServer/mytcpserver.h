#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();

    static MyTcpServer &getInstance();

    void incomingConnection(qintptr socketDescriptor);

    void reSend(const char* pername, PDU *pdu); // 转发pdu的函数

public slots:
    void deleteSocket(MyTcpSocket *mysocket); // 退出登录后删除socket

private:
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MYTCPSERVER_H
