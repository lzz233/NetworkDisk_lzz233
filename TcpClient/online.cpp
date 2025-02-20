#include "online.h"
#include "ui_online.h"
#include <QDebug>
#include "tcpclient.h"
#include <QMessageBox>

Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUsr(PDU *pdu) // 显示在线用户
{
    if(pdu == nullptr)
    {
        return;
    }
    uint uiSize = pdu->uiMsgLen/32; // 计算个数
    char caTmp[32];
    for(uint i = 0; i < uiSize; ++i)
    {
        memcpy(caTmp, (char*)(pdu->caMsg) + i*32, 32);
        // 拷贝到caTmp，从pdu的caMsg拷贝，一次拷贝
        ui->online_lw->addItem(caTmp);
    }
}

void Online::on_addFriend_pb_clicked()
{
    QListWidgetItem *pItem = ui->online_lw->currentItem(); // 获得在线列表里用户选中的那个用户
    if(nullptr == pItem)
    {
        QMessageBox::warning(this, "添加好友", "请选择要添加的好友！");
        return ;
    }

    QString strPerUsrName = pItem->text(); // 获得要添加好友用户名
    QString strLoginName = TcpClient::getInstance().getLoginName(); // 获得自己的用户名

    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData, strPerUsrName.toStdString().c_str(), strPerUsrName.size()); // 拷贝要加的好友名
    memcpy(pdu->caData + 32, strLoginName.toStdString().c_str(), strLoginName.size()); // 拷贝自己的名字
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen); // 发送
    free(pdu);
    pdu = nullptr;
}

