#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
// 页面所用到的部件
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout> // 垂直布局
#include <QHBoxLayout> // 水平布局
#include "online.h" // 所有在线用户

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnlineUsr(PDU *pdu); // 展示所有在线用户
    void updateFriendList(PDU *pdu); // 更新用户列表
    void updateGroupMsg(PDU *pdu); // 更新群聊

    QString m_strSearchName;

    QListWidget *getFriendList(); // 返回好友列表

signals:

public slots:
    void showOnline(); // 显示在线人数
    void searchUsr(); // 查找用户
    void freshFriend(); // 刷新好友列表
    void deleteFriend(); // 删除好友
    void privateChat(); // 私聊
    void groupChat(); // 群聊

private:
    QTextEdit *m_pShowMsgTE; // pointer, show message textedit，显示信息
    QListWidget * m_pFriendListWidget; // 好友列表
    QLineEdit *m_pInputMsgLE; // 信息输入框

    QPushButton *m_pDelFriendPB; // 删除好友按钮
    QPushButton *m_pFreshFriendPB; // 刷新好友按钮
    QPushButton *m_pShowOnlineUsrPB; // 查看在线用户按钮
    QPushButton *m_pSearchUsrPB; // 查找用户按钮
    QPushButton *m_pMsgSendPB; // 发送信息按钮
    QPushButton *m_pPrivateChatPB; // 私聊按钮

    Online *m_pOnline;


};

#endif // FRIEND_H
