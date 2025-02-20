#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QMessageBox>

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance() // 单例，返回PrivateChat类的引用
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName) // 保存聊天对象名字
{
    m_strChatName = strName;
    m_strName = TcpClient::getInstance().getLoginName();
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if(pdu == nullptr)
    {
        return;
    }
    char caSendName[32] = {'\0'};
    memcpy(caSendName, pdu->caData + 32, 32); // 这里是发消息的人的名字，没问题
    QString strMsg = QString("%1 says: %2").arg(caSendName, (char*)(pdu->caMsg));
    ui->showMsg_te->append(strMsg);
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_le->text();
    ui->inputMsg_le->clear(); // 发送消息后清空输入框
    ui -> showMsg_te -> append(QString("%1 : %2").arg(m_strName, strMsg)); // 把自己的消息显示在屏幕上
    if(!strMsg.isEmpty()) // 发送消息给服务器来转发给对方
    {
        PDU *pdu = mkPDU(strMsg.size() + 1); // +1是为了\0
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData, m_strChatName.toStdString().c_str(), 32); // 对方名字
        memcpy(pdu->caData + 32, m_strName.toStdString().c_str(), 32); // 我方名字

        strcpy((char*)pdu->caMsg, strMsg.toStdString().c_str()); // 拷贝消息

        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen); // 发送给服务器
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        QMessageBox::warning(this, "私聊", "发送的聊天信息不能为空");
    }
}

