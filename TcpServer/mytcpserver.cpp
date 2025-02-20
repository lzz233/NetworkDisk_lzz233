#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer() {}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance; // 由于是静态的，所以这个函数调用多次也只是创建一次
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor) // 产生连接时会自动调用这个函数
{
    // 派生QTcpSocket，然后对Socket进行绑定相应的槽函数，这样就可以不同客户端由不同MyTcpSocket进行处理
    // 从而可以实现客户端连接和对应数据收发的socket的关联
    qDebug() << "new client connected";
    MyTcpSocket *pTcpSocket = new MyTcpSocket; // 建立新的socket连接
    pTcpSocket->setSocketDescriptor(socketDescriptor);// 设置其Socket描述符，不同描述符指示不同客户端
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket, SIGNAL(offline(MyTcpSocket*)) // TcpSocket发出的信号，信号名为下线
            , this, SLOT(deleteSocket(MyTcpSocket*))); // 自己也就是客户端接收信号，调用删除socket槽函数
}

void MyTcpServer::reSend(const char *pername, PDU *pdu) // 转发pdu
{
    if(pername == nullptr || pdu == nullptr)
    {
        return;
    }
    QString strName = pername;
    for(int i = 0; i < m_tcpSocketList.size(); ++i){
        if(strName == m_tcpSocketList.at(i)->getName()) // 如果名字相同
        {
            m_tcpSocketList.at(i)->write((char*)pdu, pdu->uiPDULen);
            break;
        }
    }
}

void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin(); // 迭代器指向第一个数据
    for(; iter != m_tcpSocketList.end(); ++iter){ // 匹配相应数据
        if(mysocket == *iter){
            (*iter) -> deleteLater(); // 释放空间 原来是delete *iter，在当前事件循环结束后执行
            // 因为是异步的，原来等于在你还在发送数据的时候断开了。
            // 在多线程环境中，确保对象在所有相关操作完成后才被删除，避免线程间的资源冲突。
            *iter = nullptr; // 把指针也指向空
            m_tcpSocketList.erase(iter); // 把存放在list里的socket指针（也就是前面创建的iter）也删除掉
            break;
        }
    }
    for(int i = 0; i < m_tcpSocketList.size(); ++i){
        qDebug() << m_tcpSocketList.at(i)->getName();
    }
}
