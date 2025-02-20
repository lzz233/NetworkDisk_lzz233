#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include <protocol.h>

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    ~PrivateChat();

    static PrivateChat &getInstance(); // 单例，返回PrivateChat类的引用

    void setChatName(QString strName); // 设置聊天对象
    void updateMsg(const PDU *pdu); // 更新私聊内容

private slots:
    void on_sendMsg_pb_clicked();

private:
    Ui::PrivateChat *ui;
    QString m_strChatName; // 聊天对象的名字
    QString m_strName; // 自己的名字
};

#endif // PRIVATECHAT_H
