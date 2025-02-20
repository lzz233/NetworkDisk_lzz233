#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <qdebug.h>
#include <QMessageBox>
#include <QHostAddress>
#include "privatechat.h"

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadConfig();; // 调用访问配置信息

    connect(&m_tcpSocket, SIGNAL(connected()), // 信号发送方（Socket变量），发送信号类型
            this, SLOT(showConnect())); // 信号处理方，用以处理的槽函数
    connect(&m_tcpSocket, SIGNAL(readyRead()), // 服务器发送数据给客户端，接收到数据之后readyread
            this, SLOT(recvMsg())); // 然后调用recvMsg处理接收到的数据

    // 连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);
}

TcpClient::~TcpClient()
{
    delete ui;
}

// 头文件中加载配置文件函数的定义实现
void TcpClient::loadConfig()
{
    QFile file(":/client.config"); // 文件对象，读取资源文件 ':' + "前缀" + "文件名"
    if(file.open(QIODevice::ReadOnly)) // file.open() 参数：打开方式：只读（注意，这里只读是写在QIODevice下的枚举，所以调用要声明命名空间） 返回true则打开成功
    {
        QByteArray baData = file.readAll(); // 读出所有数据，返回字节数组QByteArray
        QString strData(baData);
        file.close();

        strData.replace("\r", " ");
        strData.replace("\n", "");
        QStringList strList = strData.split(" "); // 替换IP地址与端口号之间\r\n

        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort(); // 无符号短整型


    }else{
        QMessageBox::critical(this, "open config", "open config failed");
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket; // 返回m_tcpSocket的引用。不管哪里用到socket，直接调用就可以收发数据
}

QString TcpClient::getLoginName() // 获得登陆名字接口
{
    return m_strLoginName;
}

QString TcpClient::getCurPath() // 获得登陆时工作目录接口
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath;
}


// 头文件中检测服务器是否连接成功函数实现
void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}

void TcpClient::recvMsg() // 接收来自服务器的数据
{
    if(!OpeWidget::getInstance().getBook()->getDownloadStatus())
    {
        // qDebug() << m_tcpSocket.bytesAvailable(); // 输出接收到的数据大小
        uint uiPDULen = 0;
        m_tcpSocket.read((char*)&uiPDULen, sizeof(uint)); // 先读取uint大小的数据，首个uint正是总数据大小
        uint uiMsgLen = uiPDULen - sizeof(PDU); // 实际消息大小，sizeof(PDU)只会计算结构体大小，而不是分配的大小
        PDU *pdu = mkPDU(uiMsgLen);
        m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint)); // 接收剩余部分数据（第一个uint已读取）
        // qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);
        switch (pdu->uiMsgType) { // 收到数据
        case ENUM_MSG_TYPE_REGIST_RESPOND: // 判断数据类型为注册回复
        {
            if(strcmp(pdu->caData, REGIST_OK) == 0){
                QMessageBox::information(this, "注册", REGIST_OK);
            }else if(strcmp(pdu->caData, REGIST_FAILED) == 0){
                QMessageBox::warning(this, "注册", REGIST_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND: // 判断数据类型为登录回复
        {
            if(strcmp(pdu->caData, LOGIN_OK) == 0) // 登录成功
            {
                m_strCurPath = QString("./%1").arg(m_strLoginName); // 登录时记录默认目录为./用户名
                QMessageBox::information(this, "登录", LOGIN_OK);
                OpeWidget::getInstance().show(); // 显示主操作页面
                this->hide(); // 登录后隐藏登录窗口，this指自己所在的类，也就是TcpClient
                OpeWidget::getInstance().getFriend()->freshFriend(); // 然后刷新一下好友列表
            }
            else if(strcmp(pdu->caData, LOGIN_FAILED) == 0) // 登录失败
            {
                QMessageBox::warning(this, "登录", LOGIN_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND: // 判断数据类型为查看在线人数回复
        {
            OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND: // 判断数据类型为查找用户回复
        {
            if(strcmp(SEARCH_USR_NO, pdu->caData) == 0)
            {
                QMessageBox::information(this, "搜索"
                 , QString("%1: not exist").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            else if(strcmp(SEARCH_USR_ONLINE, pdu->caData) == 0)
            {
                QMessageBox::information(this, "搜索"
                 , QString("%1: online").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            else if(strcmp(SEARCH_USR_OFFLINE, pdu->caData) == 0)
            {
                QMessageBox::information(this, "搜索"
                 , QString("%1: offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }

            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: // 判断数据类型为你加别人回复
        {
            QMessageBox::information(this, "添加好友", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: // 判断数据类型为别人要来加你请求
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData + 32, 32); // 获得加你的人的用户名

            PDU *respdu = mkPDU(0);
            memcpy(respdu->caData, pdu->caData, 32); // 获取自己的名字
            memcpy(respdu->caData + 32, pdu->caData + 32, 32);  // 获取对方名字
            int ret = QMessageBox::information(this, "添加好友"
                     , QString("%1 want to add you as friend").arg(caName)
                                     ,QMessageBox::Yes, QMessageBox::No);
            if(ret == QMessageBox::Yes)
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
            }else if(ret == QMessageBox::No)
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_DISAGREE;
            }
            OpeWidget::getInstance().getFriend()->freshFriend(); // 然后刷新一下
            m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
            // 释放空间
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE: // 判断数据类型为别人同意加你
        {
            char caTmpStr[32] = {'\0'};
            char caPerName[32] = {'\0'};
            memcpy(caTmpStr, pdu->caData, 32);
            if(strcmp(caTmpStr, AGREE_OK) == 0){
                memcpy(caPerName, pdu->caData + 32, 32);
                QMessageBox::information(this, "加好友", QString("你向%1加好友的请求成功了").arg(caPerName));
                OpeWidget::getInstance().getFriend()->freshFriend(); // 然后刷新一下
            }else if(strcmp(caTmpStr, AGREE_FAILED) == 0){
                QMessageBox::warning(this, "加好友", AGREE_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_DISAGREE: // 判断数据类型为别人不同意加你
        {
            char caPerName[32] = {'\0'};
            memcpy(caPerName, pdu->caData, 32);
            QMessageBox::information(this, "加好友", QString("你向%1加好友的请求失败了").arg(caPerName));
            break;
        }
        case ENUM_MSG_TYPE_FRESH_FRIEND_RESPOND: // 判断数据类型为刷新好友列表
        {
            OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND: // 判断数据类型为删除好友
        {
            char caTmpStr[32] = {'\0'};
            memcpy(caTmpStr, pdu->caData, 32);
            if(strcmp(caTmpStr, DELETE_OK) == 0){
                QMessageBox::information(this, "删除好友", DELETE_OK);
                OpeWidget::getInstance().getFriend()->freshFriend(); // 然后刷新一下
            }else if(strcmp(caTmpStr, DELETE_FAILED) == 0){
                QMessageBox::warning(this, "删除好友", DELETE_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: // 转发过来的私聊请求
        {
            PrivateChat &pc = PrivateChat::getInstance();
            if (pc.isHidden())
            {
                pc.show();
            }

            char caPerName[32] = {'\0'}; // 获取发送方用户名
            memcpy(caPerName, pdu->caData + 32, 32);

            pc.setChatName(caPerName);
            pc.updateMsg(pdu);

            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: // 转发过来的群聊请求
        {
            OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu); // 转发给更新群聊界面函数
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND: // 判断数据类型为创建文件夹
        {
            QMessageBox::information(this, "创建文件夹", pdu->caData);
            OpeWidget::getInstance().getBook()->freshFile(); // 刷新一下
            break;
        }
        case ENUM_MSG_TYPE_FRESH_FLIE_RESPOND: // 判断数据类型为刷新文件
        {
            OpeWidget::getInstance().getBook()->updateFileList(pdu); // 调用OpeWidget里的刷新文件列表函数
            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_RESPOND: // 判断数据类型为删除文件夹
        {
            QMessageBox::information(this, "删除文件夹", pdu->caData);
            OpeWidget::getInstance().getBook()->freshFile(); // 刷新一下
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FLIE_RESPOND: // 判断数据类型为重命名文件
        {
            QMessageBox::information(this, "重命名文件", pdu->caData);
            OpeWidget::getInstance().getBook()->freshFile(); // 刷新一下
            break;
        }
        case ENUM_MSG_TYPE_ENTRY_DIR_RESPOND: // 判断数据类型为进入文件夹
        {
            char caTmpStr[32] = {'\0'};
            memcpy(caTmpStr, pdu->caData, 32);
            if(strcmp(caTmpStr, ENTRY_DIR_OK) == 0){
                QMessageBox::information(this, "进入文件夹", ENTRY_DIR_OK);

                QString enterDir = OpeWidget::getInstance().getBook()->getEnterDir();
                m_strCurPath = enterDir; // 修改路径
                OpeWidget::getInstance().getBook()->freshFile(); // 刷新一下
            }else if(strcmp(caTmpStr, ENTRY_DIR_FAILED) == 0){
                QMessageBox::warning(this, "进入文件夹", ENTRY_DIR_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND: // 判断数据类型为上传文件
        {
            QMessageBox::information(this, "上传文件", pdu->caData);
            OpeWidget::getInstance().getBook()->freshFile(); // 刷新一下
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND: // 判断数据类型为下载文件
        {
            qDebug() << "121212";
            qDebug() << "下载文件" << pdu->caData;

            char caFileName[32] = {'\0'};
            qint64 iTotal = 0;
            qDebug() << "解析文件信息前" << pdu->caData;
            sscanf(pdu->caData, "%s %lld", caFileName, &iTotal); // 读取要下载的文件信息
            qDebug() << "解析文件信息后" << caFileName << iTotal;

            OpeWidget::getInstance().getBook()->setFileTotal(iTotal);

            if(strlen(caFileName) > 0 && iTotal > 0)
            {
                qDebug() << "文件名和大小有效" << caFileName << iTotal;
                OpeWidget::getInstance().getBook()->setDownloadStatus(true); // 设置下载状态为真
                OpeWidget::getInstance().getBook()->setFileRecved(0);
                QString savePath = OpeWidget::getInstance().getBook()->getFileSavePath();
                qDebug() << "保存路径" << savePath;
                m_file.setFileName(savePath); // 打开文件

                if(!m_file.open(QIODevice::WriteOnly))
                {
                    qDebug() << "文件打开失败" << savePath;
                    QMessageBox::warning(this, "下载文件", "获得保存文件路径失败");
                    return;
                }
                qDebug() << "131313";
            }
            else
            {
                qDebug() << "文件名或大小无效" << caFileName << iTotal;
            }
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND: // 判断数据类型为分享文件回复
        {
            QMessageBox::information(this, "共享文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE: // 判断数据类型为转发的分享文件
        {
            qDebug() << "444";
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, (char*)(pdu->caMsg), pdu->uiMsgLen); // 拷贝下载路径

            char *pos = strrchr(pPath, '/'); // 找到最后一个斜杠的位置
            if(pos != nullptr)
            {
                ++pos;
                QString strNote = QString("%1 share file -> %2 \n Do you accepct?").arg(pdu->caData, pos);
                int ret = QMessageBox::question(this, "共享文件", strNote); // 决定是否接收
                if(ret == QMessageBox::Yes)
                {
                    PDU *respdu = mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy(respdu->caMsg, (char*)(pdu->caMsg), pdu->uiMsgLen); // 拷贝下载路径
                    QString strName = TcpClient::getInstance().getLoginName();
                    strncpy(respdu->caData, strName.toStdString().c_str(), 32); // 拷贝要接受的用户名
                    m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
                    qDebug() << "555";
                }
            }


            delete[] pPath;
            pPath = nullptr;
            OpeWidget::getInstance().getBook()->freshFile(); // 刷新一下

            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:
        {
            QMessageBox::information(this, "移动文件", pdu -> caData);
            OpeWidget::getInstance().getBook()->freshFile(); // 刷新一下
            break;
        }


        default:
            break;
        }
        // 释放空间
        free(pdu);
        pdu = nullptr;
    }
    else{ // 如果是下载状态

        qDebug() << "141414";
        QByteArray buffer = m_tcpSocket.readAll();
        // QFile file;
        // file.setFileName(OpeWidget::getInstance().getBook()->getFileSavePath()); // 设置保存路径
        qDebug() << m_file.fileName() << "名字";
        m_file.write(buffer); // 写到缓冲区

        Book *pBook = OpeWidget::getInstance().getBook();
        qint64 fileRecved = pBook->getFileRecved(); // 设置收到的文件大小
        qint64 fileTotal = pBook->getFileTotal(); // 文件应该多大
        fileRecved += buffer.size();
        pBook->setFileRecved(fileRecved);

        if(fileTotal <= fileRecved)
        {
            m_file.close();
            pBook->setFileRecved(0);
            pBook->setFileTotal(0);
            pBook->setDownloadStatus(false);

            if(fileTotal < fileRecved) // 如果收到的比预计的还大
            {
                QMessageBox::critical(this, "下载文件", "下载文件失败");
            }
        }
                qDebug() << "151515";
    }
}

void TcpClient::on_login_pb_clicked() // 登录
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text(); // 获取用户名和密码
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        m_strLoginName = strName;
        // 登录信息用户名和密码将通过caData[64]传输
        PDU *pdu = mkPDU(0); // 实际消息体积为0
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST; // 设置为登录请求消息类型
        strncpy(pdu->caData, strName.toStdString().c_str(), 32); // 由于数据库设定的32位，所以最多只拷贝前32位
        strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen); // 发送消息
        // 释放空间
        free(pdu);
        pdu = nullptr;
    }else{
        QMessageBox::critical(this, "登录", "登录失败：用户名或密码错误");
    }
} // 和注册的代码只有发送的请求消息类型不一样，这个是登录请求

void TcpClient::on_regist_pb_clicked() // 注册
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text(); // 获取用户名和密码
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        // 注册信息用户名和密码将通过caData[64]传输
        PDU *pdu = mkPDU(0); // 实际消息体积为0
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST; // 设置为注册请求消息类型
        strncpy(pdu->caData, strName.toStdString().c_str(), 32); // 由于数据库设定的32位，所以最多只拷贝前32位
        strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen); // 发送消息
        // 释放空间
        free(pdu);
        pdu = nullptr;
    }else{
        QMessageBox::critical(this, "注册", "注册失败：用户名或密码不能为空");
    }
}


void TcpClient::on_cancel_pb_clicked()
{

}




