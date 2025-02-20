#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog> // 用来输入数据（好像是和建立搜索框相关？见searchUsr()）
#include <QDebug>
#include "privatechat.h"
#include <QMessageBox>


Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE = new QTextEdit; // pointer, show message textedit，显示信息框
    m_pFriendListWidget = new QListWidget; // 好友列表
    m_pInputMsgLE = new QLineEdit; // 信息输入框

    m_pDelFriendPB = new QPushButton("删除好友"); // 删除好友按钮
    m_pFreshFriendPB = new QPushButton("刷新好友列表"); // 刷新好友列表按钮
    m_pShowOnlineUsrPB = new QPushButton("查看在线用户"); // 查看在线用户按钮
    m_pSearchUsrPB = new QPushButton("查找用户"); // 查找用户按钮
    m_pPrivateChatPB = new QPushButton("聊天"); // 私聊按钮
    m_pMsgSendPB = new QPushButton("发送信息"); // 发送信息按钮

    QVBoxLayout *pRightPBVBL = new QVBoxLayout; // 右侧几个按钮合成垂直布局
    // QVBoxLayout: qt vertical box layout 垂直布局
    // pRightPBVBL: pointer, right, push-button, vertical box layout
    pRightPBVBL->addWidget(m_pDelFriendPB); // 删除好友按钮
    pRightPBVBL->addWidget(m_pFreshFriendPB); // 刷新好友列表按钮
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB); // 查看在线用户按钮
    pRightPBVBL->addWidget(m_pSearchUsrPB); // 查找用户按钮
    pRightPBVBL->addWidget(m_pPrivateChatPB); // 私聊按钮

    QHBoxLayout *pTopHBL = new QHBoxLayout; // 在上面合成一个水平布局
    pTopHBL->addWidget(m_pShowMsgTE); // 显示信息框
    pTopHBL->addWidget(m_pFriendListWidget); // 好友列表
    pTopHBL->addLayout(pRightPBVBL); // 前面的按钮们

    QHBoxLayout *pMsgHBL = new QHBoxLayout; // 合成一个水平布局
    pMsgHBL->addWidget(m_pInputMsgLE); // 信息输入框
    pMsgHBL->addWidget(m_pMsgSendPB); // 发送信息按钮

    m_pOnline = new Online;

    QVBoxLayout *pMain = new QVBoxLayout; // 一起合成垂直布局
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide(); // 先不显示，点击搜索之后再显示

    setLayout(pMain);

    // 关联槽函数
    connect(m_pShowOnlineUsrPB, SIGNAL(clicked(bool))
            , this, SLOT(showOnline()));

    connect(m_pSearchUsrPB, SIGNAL(clicked(bool))
            , this, SLOT(searchUsr()));

    connect(m_pFreshFriendPB, SIGNAL(clicked(bool))
            , this, SLOT(freshFriend()));

    connect(m_pDelFriendPB, SIGNAL(clicked(bool))
            , this, SLOT(deleteFriend()));

    connect(m_pPrivateChatPB, SIGNAL(clicked(bool))
            , this, SLOT(privateChat()));

    connect(m_pMsgSendPB, SIGNAL(clicked(bool))
            , this , SLOT(groupChat()));
}

void Friend::showAllOnlineUsr(PDU *pdu)
{
    if(pdu == nullptr)
    {
        return;
    }
    m_pOnline->showUsr(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if(pdu == nullptr)
    {
        return;
    }
    uint uiSize = pdu->uiMsgLen / 32;
    char caName[32] = {'\0'};
    m_pFriendListWidget -> clear(); // 清除好友列表原有数据
    for(uint i=0; i < uiSize; ++i)
    {
        memcpy(caName, (char*)(pdu->caMsg) + i*32, 32);
        m_pFriendListWidget->addItem(caName);
    }
}

void Friend::updateGroupMsg(PDU *pdu) // 有人说话后，更新群聊界面
{
    QString strMsg = QString("%1 says: %2").arg(pdu->caData, (char*)(pdu->caMsg));
    m_pShowMsgTE->append(strMsg);
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline() // 展示在线人数
{
    if(m_pOnline->isHidden())
    {
        m_pOnline->show(); // 假如online页面是隐藏的，显示之

        // 发送请求查询数据库获取在线用户
        PDU *pdu = mkPDU(0); // 制造一个协议
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST; // 在线用户请求
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen); // .write()代表发送
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        m_pOnline->hide(); // 反之隐藏之
    }
}

void Friend::searchUsr()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名：");
    if(!m_strSearchName.isEmpty())
    {
        qDebug() << m_strSearchName;
        PDU *pdu = mkPDU(0);
        memcpy(pdu->caData, m_strSearchName.toStdString().c_str(), m_strSearchName.size()); // 把名字拷贝过来

        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen); // .write()代表发送
        free(pdu);
        pdu = nullptr;
    }

}

void Friend::freshFriend()
{
    QString strName = TcpClient::getInstance().getLoginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FRESH_FRIEND_REQUEST;
    memcpy(pdu->caData, strName.toStdString().c_str(), 32);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Friend::deleteFriend()
{
    if(m_pFriendListWidget->currentItem() != nullptr)
    {
        QString strFriendName = m_pFriendListWidget->currentItem()->text();
        QString strSelfName = TcpClient::getInstance().getLoginName();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        memcpy(pdu->caData, strFriendName.toStdString().c_str(), 32);
        memcpy(pdu->caData + 32, strSelfName.toStdString().c_str(), 32);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Friend::privateChat()
{
    if(m_pFriendListWidget->currentItem() != nullptr)
    {
        QString strChatName = m_pFriendListWidget->currentItem()->text(); // 聊天对象的名字
        PrivateChat::getInstance().setChatName(strChatName);
        if(PrivateChat::getInstance().isHidden())
        {
            PrivateChat::getInstance().show();
        }
    }else{
        QMessageBox::warning(this, "私聊", "请选择私聊的对象");
    }
}

void Friend::groupChat() // 群聊
{
    QString strMsg = m_pInputMsgLE->text(); // 获取输入框内容
    if(!strMsg.isEmpty()) // 非空
    {
        QString strName = TcpClient::getInstance().getLoginName(); // 获得发信人名字，发给服务器
        m_pInputMsgLE -> clear(); // 发送消息后清空输入框
        m_pShowMsgTE -> append(QString("%1 : %2").arg(strName, strMsg)); // 把自己的消息显示在屏幕上
        PDU *pdu = mkPDU(strMsg.size() + 1); // 消息大小为输入框内容+1
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), strName.size()); // 拷贝名字
        strncpy((char*)(pdu->caMsg), strMsg.toStdString().c_str(), strMsg.size()); // 拷贝消息
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen); // 发送
        free(pdu); // 释放
        pdu = nullptr;
    }else{
        QMessageBox::warning(this, "群聊", "信息不能为空");
    }
}
