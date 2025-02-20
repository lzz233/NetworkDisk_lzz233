#include "sharefile.h"
#include "tcpclient.h"
#include "opewidget.h"
#include <QVBoxLayout>
#include <QCheckBox>

ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{
    m_pSelectAllPB = new QPushButton("全选"); // 全选
    m_pCancelSelectPB = new QPushButton("取消选择"); // 反选
    m_pOKPB = new QPushButton("确定分享"); // 确定分享
    m_pCancelPB = new QPushButton("取消分享"); // 取消分享

    m_pSA = new QScrollArea; // 滚动浏览区域
    m_pFriendW = new QWidget; // 用来展示的东西

    m_pFriendWVBL = new QVBoxLayout(m_pFriendW); // 选择好友界面的垂直布局
    m_pButtonGroup = new QButtonGroup(m_pFriendW); // 按钮组
    m_pButtonGroup->setExclusive(false);// 设置可以多选

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch(); // 弹簧

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);

    connect(m_pCancelSelectPB, SIGNAL(clicked(bool))
            , this, SLOT(cancelSelect())); // 取消选择

    connect(m_pSelectAllPB, SIGNAL(clicked(bool))
            , this, SLOT(selectAll())); // 全选

    connect(m_pOKPB, SIGNAL(clicked(bool))
            , this, SLOT(okShare())); // 全选

    connect(m_pCancelPB, SIGNAL(clicked(bool))
            , this, SLOT(cancelShare())); // 全选


}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if(pFriendList == nullptr)
    {
        return;
    }
    QAbstractButton *tmp = nullptr;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons(); // 获得之前的所有列表里好友
    for(int i = 0; i < preFriendList.size(); ++i)
    {
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete[] tmp;
        tmp = nullptr;
    } // 删除原来的选项

    QCheckBox *pCB = nullptr;
    for(int i = 0; i < pFriendList->count(); ++i)
    {
        pCB = new QCheckBox(pFriendList->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i = 0; i < cbList.size(); ++i)
    {
        if(cbList[i]->isChecked())
        {
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i = 0; i < cbList.size(); ++i)
    {
        if(!cbList[i]->isChecked())
        {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::okShare()
{
    qDebug() << "111";
    QString strName = TcpClient::getInstance().getLoginName(); // 分享者名字
    QString strCurPath = TcpClient::getInstance().getCurPath(); // 文件路径
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName(); // 获得要分享的文件名

    QString strPath = strCurPath + '/' + strShareFileName; // 拼接路径

    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    int cnt = 0; // 计算选中的名字数量
    for(int i = 0; i < cbList.size(); ++i)
    {
        if(cbList[i]->isChecked())
        {
            cnt++;
            qDebug() << "222";
        }
    }

    PDU *pdu = mkPDU(cnt*32 + strPath.size() + 1); // 选中的名字和路径要放进caMsg里
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData, "%s %d", strName.toStdString().c_str(), cnt); // 分享者名字和数量传到caData
    int j = 0;
    for(int i = 0; i < cbList.size(); ++i)
    {
        if(cbList[i]->isChecked())
        {
            memcpy((char*)(pdu->caMsg) + j*32, cbList[i]->text().toStdString().c_str(), cbList[i]->text().size());
            ++j;
        }
    } // 选中的名字放进caMsg里

    memcpy((char*)(pdu->caMsg) + cnt*32, strPath.toStdString().c_str(), strPath.size()); // 路径放进caMsg里

    // 发送
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;

    hide(); // 隐藏界面
    qDebug() << "333";
}

void ShareFile::cancelShare()
{
    hide(); // 隐藏界面
}
