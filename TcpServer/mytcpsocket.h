#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QDir>
#include <QFile>
#include <QTimer>
#include "protocol.h"
#include "opedb.h"

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName(); // 提供获得名字的公用接口

    void copyDir(QString strSrcDir, QString strDesDir); // 分享文件夹

signals:
    void offline(MyTcpSocket *mysocket); // 存入地址

public slots:
    void recvMsg();
    void clientOffline();

    void sendFileToClient(); // 下载文件，计时器到点时调用

private:
    QString m_strName;

    QFile m_file;
    qint64 m_iTotal; // 文件大小
    qint64 m_iReceived; // 接收了多少文件
    bool m_bUpload; // 判断是否接收文件状态

    QTimer *m_pTimer; // 下载文件的计时器
};

#endif // MYTCPSOCKET_H
