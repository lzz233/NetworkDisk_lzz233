#include "mytcpsocket.h"
#include "mytcpserver.h"
#include <QDebug>
#include <QDir>
#include <QFileInfoList> // 显示文件详细信息


MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, SIGNAL(readyRead()) // 当接收到客户端的数据时，服务器会发送readyRead()信号
        ,this, SLOT(recvMsg())); // 需要由服务器的相应receiveMsg槽函数进行处理
    connect(this, SIGNAL(disconnected()) // 当接收到客户端的数据时，服务器会发送readyRead()信号
            ,this, SLOT(clientOffline())); // 需要由服务器的相应receiveMsg槽函数进行处理

    m_bUpload = false; // 初始上传文件状态为false
    m_pTimer = new QTimer;

    connect(m_pTimer, SIGNAL(timeout())
            , this, SLOT(sendFileToClient())); // 下载文件。到了时间给客户端传文件
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDesDir)
{
    qDebug() << "Copying from" << strSrcDir << "to" << strDesDir;

    // 确保路径以分隔符结尾
    if (!strSrcDir.endsWith(QDir::separator())) {
        strSrcDir += QDir::separator();
    }
    if (!strDesDir.endsWith(QDir::separator())) {
        strDesDir += QDir::separator();
    }

    // 确保目标目录存在
    QDir desDir(strDesDir);
    if (!desDir.exists() && !desDir.mkpath(strDesDir)) {
        qWarning() << "Failed to create target directory:" << strDesDir;
        return;
    }

    // 获取源目录内容，排除 "." 和 ".."
    QDir srcDir(strSrcDir);
    QFileInfoList fileInfoList = srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Readable);

    QString srcTmp, desTmp;
    for (const QFileInfo &fileInfo : fileInfoList) {
        srcTmp = fileInfo.filePath();
        desTmp = desDir.filePath(fileInfo.fileName());

        if (fileInfo.isFile()) {
            qDebug() << "Copying file:" << fileInfo.fileName();
            if (QFile::exists(desTmp)) {
                QFile::remove(desTmp); // 删除已存在的文件
            }
            if (!QFile::copy(srcTmp, desTmp)) {
                qWarning() << "Failed to copy file:" << srcTmp << "to" << desTmp;
            }
        } else if (fileInfo.isDir()) {
            qDebug() << "Processing subdirectory:" << fileInfo.fileName();

            // 递归调用
            copyDir(srcTmp, desTmp);
        }
    }

    qDebug() << "Directory copied successfully.";
}


void MyTcpSocket::recvMsg()
{
    if(!m_bUpload) // 如果不是上传文件，则封装pdu
    {
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen, sizeof(uint)); // 先读取uint大小的数据，首个uint正是总数据大小
        uint uiMsgLen = uiPDULen - sizeof(PDU); // 实际消息大小，sizeof(PDU)只会计算结构体大小，而不是分配的大小
        PDU *pdu = mkPDU(uiMsgLen);
        this -> read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint)); // 接收剩余部分数据（第一个uint已读取）
        switch (pdu->uiMsgType) { // 收到数据
        case ENUM_MSG_TYPE_REGIST_REQUEST: // 判断数据类型为注册请求
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData + 32, 32); // 获得注册的用户名和密码
            bool ret = OpeDB::getInstance().handelRegist(caName, caPwd); // 去数据库处理获得的数据
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            if(ret) // 根据返回值判断处理成功还是失败
            {
                strcpy(respdu->caData, REGIST_OK); // 如果处理成功
                QDir dir;
                dir.mkdir(QString("./%1").arg(caName)); // 创建以名字命名的文件夹

            }else{
                strcpy(respdu->caData, REGIST_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen); // 发送消息
            // 释放空间
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST: // 判断数据类型为登录请求
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData + 32, 32); // 获得登录的用户名和密码
            bool ret = OpeDB::getInstance().handelLogin(caName, caPwd); // 去数据库处理获得的数据
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if(ret){ // 根据返回值判断处理成功还是失败
                strcpy(respdu->caData, LOGIN_OK); // 登录成功
                m_strName = caName; // 如果你有多个终端同时登录，每个终端都会有自己的MyTcpSocket实例，每个实例都有自己的m_strName成员变量。
            }else{
                strcpy(respdu->caData, LOGIN_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen); // 发送消息
            // 释放空间
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST: // 判断数据类型为查看在线用户请求
        {
            QStringList ret = OpeDB::getInstance().handleAllOnline();
            uint uiMsgLen = ret.size()*32; // 消息大小为名字个数乘以32字节
            PDU *respdu = mkPDU(uiMsgLen); // 制造返回的pdu协议（就是在信息）
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i = 0; i < ret.size(); ++ i)
            {
                // caMsg是int型，直接加32就不对了，要先转成char*型
                memcpy((char*)(respdu->caMsg) + i*32
                       , ret.at(i).toStdString().c_str()
                       , ret.at(i).size());
            } // 参数分别是：目标内存区域的起始地址，源内存区域的起始地址，要复制的字节数
            // 不直接返回ret，因为ret是链表，内存空间不一定连续
            write((char*)respdu, respdu->uiPDULen); // 发送消息
            // 释放空间
            free(respdu);
            respdu = nullptr;

            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST: // 判断数据类型为查找用户请求
        {
            int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData); // 要通过instance静态实例来操作才行
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if(ret == -1)
            {
                strcpy(respdu->caData, SEARCH_USR_NO);
            }else if(ret == 1)
            {
                strcpy(respdu->caData, SEARCH_USR_ONLINE);
            }else if(ret == 0)
            {
                strcpy(respdu->caData, SEARCH_USR_OFFLINE);
            }
            write((char*)respdu, respdu->uiPDULen); // 发送消息
            // 释放空间
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: // 判断数据类型为加好友请求
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32); // 获得要加的的用户名
            strncpy(caName, pdu->caData + 32, 32); // 获得登录的用户名
            int ret = OpeDB::getInstance().handleAddFriend(caPerName, caName); // 要通过instance静态实例来操作才行
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            if(ret == -1)
            {
                strcpy(respdu->caData, UNKNOWN_ERROR); // 未知错误
            }else if(ret == 0)
            {
                strcpy(respdu->caData, ADD_FRIEND_EXIST); // 已是好友
            }else if(ret == 1)
            {
                strcpy(respdu->caData, ADD_FRIEND_OK); // 用户存在，发送加好友请求
                MyTcpServer::getInstance().reSend(caPerName, pdu); // 这里是转发不是回复，所以用pdu而不是respdu
            }else if(ret == 2)
            {
                strcpy(respdu->caData, ADD_FRIEND_EMPTY); // 用户不存在
            }
            write((char*)respdu, respdu->uiPDULen); // 发送消息
            // 释放空间
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE: // 判断数据类型为同意加好友请求
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32); // 获得要加的的用户名
            strncpy(caName, pdu->caData + 32, 32); // 获得登录的用户名

            PDU *respdu = mkPDU(0);
            memcpy(respdu->caData + 32, pdu->caData, 32); // 只需要复制同意的那个人名就行
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;

            // 在数据库里添加好友信息
            bool ret = OpeDB::getInstance().handleAddFriendAgree(caPerName, caName);
            if(ret){ // 根据返回值判断处理成功还是失败
                memcpy(respdu->caData, AGREE_OK, 32);
            }else{
                memcpy(respdu->caData, AGREE_FAILED, 32);
            }

            write((char*)respdu, respdu->uiPDULen); // 发送消息
            // 释放空间
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_DISAGREE: // 判断数据类型为不同意加好友请求
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32); // 获得要加的的用户名
            strncpy(caName, pdu->caData + 32, 32); // 获得登录的用户名

            PDU *respdu = mkPDU(0);
            memcpy(respdu->caData, pdu->caData, 32); // 只需要复制不同意的那个人名就行
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_DISAGREE;

            write((char*)respdu, respdu->uiPDULen); // 发送消息
            // 释放空间
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_FRESH_FRIEND_REQUEST: // 判断数据类型为刷新好友列表请求
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData, 32); // 获得登录的用户名
            QStringList ret = OpeDB::getInstance().handleFreshFriend(caName); // 从数据库获取用户好友列表
            uint uiMsgLen = ret.size() * 32; // size返回个数

            PDU *respdu = mkPDU(uiMsgLen);
            for(int i = 0; i < ret.size(); ++i)
            {
                memcpy((char*)(respdu->caMsg) + i*32
                       , ret.at(i).toStdString().c_str()
                       , ret.at(i).size());
            }

            respdu->uiMsgType = ENUM_MSG_TYPE_FRESH_FRIEND_RESPOND;

            write((char*)respdu, respdu->uiPDULen); // 发送消息
            // 释放空间
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: // 判断数据类型为删除好友
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32); // 获得要删除的的用户名
            strncpy(caName, pdu->caData + 32, 32); // 获得登录的用户名
            bool ret = OpeDB::getInstance().handleDeleteFriend(caPerName, caName);
            PDU *respdu = mkPDU(0);
            if(ret){ // 根据返回值判断处理成功还是失败
                memcpy(respdu->caData, DELETE_OK, 32);
            }else{
                memcpy(respdu->caData, DELETE_FAILED, 32);
            }
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            write((char*)respdu, respdu->uiPDULen); // 发送消息
            // 释放空间
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: // 判断数据类型私聊请求
        {
            char caPerName[32] = {'\0'};
            memcpy(caPerName, pdu->caData, 32);
            MyTcpServer::getInstance().reSend(caPerName, pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: // 判断数据类型群聊请求
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData, 32); // 获得登录的用户名
            QStringList OnlineFriend = OpeDB::getInstance().handleFreshFriend(caName); // 从数据库获取用户好友列表
            QString tmp;
            for(int i = 0; i < OnlineFriend.size(); ++i)
            {
                tmp = OnlineFriend.at(i);
                MyTcpServer::getInstance().reSend(tmp.toStdString().c_str(), pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST: // 判断数据类型创建子文件夹请求
        {
            QDir qdir;
            QString strCurPath = QString("%1").arg((char*)pdu->caMsg); // 拷贝当前工作目录
            char caName[32] = {'\0'};
            memcpy(caName, pdu->caData, 32); // 拷贝自己的名字
            char caNewDir[32] = {'\0'};
            memcpy(caNewDir, pdu->caData + 32, 32); // 拷贝要创建的文件夹名字
            bool ret = qdir.exists(strCurPath);
            PDU *respdu = mkPDU(0);
            if(!ret) // 当前目录不存在
            {
                QDir dir;
                dir.mkdir(strCurPath); // 创建以当前工作目录命名的文件夹
            }

            QString strNewPath = strCurPath + "/" + caNewDir; // 拼接路径
            ret = qdir.exists(strNewPath);
            // qDebug() << "路径已经存在了" << strCurPath;

            if(ret) // 创建的文件夹已经存在
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData, "创建的文件夹已经存在");
            }else{
                qdir.mkdir(strNewPath); // 创建新子文件夹
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData, "创建文件夹成功");
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_FRESH_FLIE_REQUEST: // 判断数据类型刷新文件请求
        {
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen); // 拷贝路径

            QDir qdir(pCurPath);
            QFileInfoList fileInfoList = qdir.entryInfoList(); // 获取路径下的每个文件信息

            int iFileCnt = fileInfoList.size(); // 获得文件个数
            PDU *respdu = mkPDU(sizeof(FileInfo) * iFileCnt); // 制作PDU，PDU消息大小设定为FileInfo结构体的大小乘以其个数
            respdu->uiMsgType = ENUM_MSG_TYPE_FRESH_FLIE_RESPOND;

            FileInfo *pFileInfo = nullptr;
            QString strFileName; // 临时用一下的变量
            for(int i = 0; i < iFileCnt; ++i) // 前两个文件代表本级目录和上级目录
            {
                // pFileInfo代表存放文件信息的结构体
                pFileInfo = (FileInfo*)(respdu->caMsg) + i; // 转换成结构体类型，指向camsg的首位。这样+1就等于+1个结构体大小
                strFileName = fileInfoList[i].fileName(); // 临时提取文件名

                memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size()); // 拷贝文件名
                if(fileInfoList[i].isDir())
                {
                    pFileInfo->iFileType = 0; // 文件类型为文件夹
                }else if(fileInfoList[i].isFile())
                {
                    pFileInfo->iFileType = 1; // 文件类型为文件
                }
            }

            // 发送
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            delete[] pCurPath;
            pCurPath = nullptr;

            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_REQUEST: // 判断数据类型删除文件请求
        {
            char caDirName[64] = {'\0'};
            memcpy(caDirName, pdu->caData, 64); // 拷贝要删除的文件夹名字

            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen); // 拷贝路径

            QString strPath = QString("%1/%2").arg(pCurPath, caDirName); // 拼接路径

            QFileInfo fileInfo(strPath);
            QDir qdir; // 创建qdir对象，用来删除
            qdir.setPath(strPath);
            bool ret = false;
            if(fileInfo.isDir()) // 判断为文件夹
            {
                qDebug() << strPath << "判断为文件夹";
                ret = qdir.removeRecursively(); // 删除路径，包括里面所有的文件

            }else{ // 判断为其他文件

                qDebug() << strPath << "判断为文件";
                QFile file(strPath); // 直接用路径删行不通
                ret = file.remove(); // 删除文件
            }

            PDU *respdu = mkPDU(0);
            if(ret)
            {
                memcpy(respdu->caData, DELETE_DIR_OK, strlen(DELETE_DIR_OK)); // 把宏拷贝到caData里
            }else{
                memcpy(respdu->caData, DELETE_DIR_FAILED, strlen(DELETE_DIR_FAILED)); // 把宏拷贝到caData里
            }
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;

            // 发送
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            delete[] pCurPath;
            pCurPath = nullptr;

            break;
        }
        case ENUM_MSG_TYPE_RENAME_FLIE_REQUEST: // 重命名文件
        {
            char caOldName[32] = {'\0'};
            memcpy(caOldName, pdu->caData, 32); // 拷贝旧文件名字
            char caNewName[32] = {'\0'};
            memcpy(caNewName, pdu->caData + 32, 32); // 拷贝新文件名字

            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen); // 拷贝路径

            QString strOldPath = QString("%1/%2").arg(pCurPath, caOldName); // 拼接路径
            QString strNewPath = QString("%1/%2").arg(pCurPath, caNewName); // 拼接路径

            QDir qdir;
            bool ret = qdir.rename(strOldPath, strNewPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FLIE_RESPOND;
            if(ret) // 重命名成功
            {
                strncpy(respdu->caData, RENAME_FILE_OK, 64);
            }else{
                strncpy(respdu->caData, RENAME_FILE_FAILED, 64);
            }

            // 发送
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            delete[] pCurPath;
            pCurPath = nullptr;

            break;
        }
        case ENUM_MSG_TYPE_ENTRY_DIR_REQUEST: // 进入文件夹
        {
            char caDirName[32] = {'\0'};
            memcpy(caDirName, pdu->caData, 32); // 拷贝旧文件名字

            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen); // 拷贝路径

            QString strPath = QString("%1/%2").arg(pCurPath, caDirName); // 拼接路径
            QFileInfo fileInfo(strPath);
            PDU *respdu = mkPDU(0);
            if(fileInfo.isDir()) // 判断为文件夹
            {
                QDir qdir(pCurPath);
                QFileInfoList fileInfoList = qdir.entryInfoList(); // 获取路径下的每个文件信息

                int iFileCnt = fileInfoList.size(); // 获得文件个数
                respdu = mkPDU(sizeof(FileInfo) * iFileCnt); // 制作PDU，PDU消息大小设定为FileInfo结构体的大小乘以其个数
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTRY_DIR_RESPOND;

                FileInfo *pFileInfo = nullptr;
                QString strFileName; // 临时用一下的变量
                for(int i = 0; i < iFileCnt; ++i) // 前两个文件代表本级目录和上级目录
                {
                    // pFileInfo代表存放文件信息的结构体
                    pFileInfo = (FileInfo*)(respdu->caMsg) + i; // 转换成结构体类型，指向camsg的首位。这样+1就等于+1个结构体大小
                    strFileName = fileInfoList[i].fileName(); // 临时提取文件名

                    memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size()); // 拷贝文件名
                    if(fileInfoList[i].isDir())
                    {
                        pFileInfo->iFileType = 0; // 文件类型为文件夹
                    }else if(fileInfoList[i].isFile())
                    {
                        pFileInfo->iFileType = 1; // 文件类型为文件
                    }
                }

                memcpy(respdu->caData, ENTRY_DIR_OK, strlen(ENTRY_DIR_OK)); // 把宏拷贝到caData里


            }else if(fileInfo.isFile()){ // 判断为常规文件

                memcpy(respdu->caData, ENTRY_DIR_FAILED, strlen(ENTRY_DIR_FAILED)); // 把宏拷贝到caData里
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTRY_DIR_RESPOND;
            }
            // 发送
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            delete[] pCurPath;
            pCurPath = nullptr;

            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST: // 上传文件
        {
            char caFileName[32] = {'\0'};
            qint64 fileSize = 0;
            sscanf(pdu->caData, "%s %lld", caFileName, &fileSize); // 读取数据。sscanf要取地址的。

            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, (char*)pdu->caMsg, pdu->uiMsgLen); // 拷贝工作路径
            QString strPath = QString("%1/%2").arg(pCurPath, caFileName); // 拼接路径

            m_file.setFileName(strPath);
            // 以只写的方式打开文件，文件如果不存在会自动创建
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_bUpload = true; // 设置上传文件状态
                m_iTotal = fileSize; // 设置文件大小
                m_iReceived = 0; // 设置上传的文件数
            }

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;

            // 发送
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            delete[] pCurPath;
            pCurPath = nullptr;

            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST: // 下载文件
        {
            qDebug() << "555";
            char caFileName[32] = {'\0'};
            strncpy(caFileName, pdu->caData, 32);

            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, (char*)pdu->caMsg, pdu->uiMsgLen); // 拷贝工作路径
            QString strPath = QString("%1/%2").arg(pCurPath, caFileName); // 拼接路径

            QFileInfo fileinfo(strPath);
            qint64 fileSize = fileinfo.size(); // 获得文件大小

            PDU *respdu = mkPDU(0);
            sprintf(respdu->caData, "%s %lld", caFileName, fileSize);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;

            // 发送
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            delete[] pCurPath;
            pCurPath = nullptr;

            // 借用 m_file对象进行下载文件，因为目前是单线程阻塞的，所以不影响
            m_file.setFileName(strPath);
            if (m_file.open(QIODevice::ReadOnly)) {
                m_pTimer->start(1000); // 1秒后启动计时器
                qDebug() << "666";
            } else {
                qDebug() << "文件打开失败：" << strPath;
            }

            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            char caSendName[32] = {'\0'}; // 发送者名字
            int num = 0; // 要发给多少人
            sscanf(pdu->caData, "%s %d", caSendName, &num);

            int size = 32 * num;

            PDU *respdu = mkPDU(pdu->uiMsgLen - size); // 只传路径
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            memcpy(respdu->caData, caSendName, 32);
            memcpy(respdu->caMsg, (char*)(pdu->caMsg) + size, pdu->uiMsgLen - size);

            char caRcvName[32] = {'\0'}; // 接收者名字
            for(int i = 0; i < num; ++i)
            {
                memcpy(caRcvName, (char*)(pdu->caMsg) + i*32, 32);
                MyTcpServer::getInstance().reSend(caRcvName, respdu);
            } // 循环读取并发送

            free(respdu);
            respdu = nullptr; // 先释放一下

            respdu = mkPDU(0); // 产生给分享者的回复pdu
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strncpy(respdu->caData, SHARE_FILE_OK, strlen(SHARE_FILE_OK));
            write((char*)respdu, respdu->uiPDULen);

            free(respdu);
            respdu = nullptr; // 再释放一下
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
        {
            qDebug() << "666";
            QString strRcvPath = QString("./%1").arg(pdu->caData); // 默认保存至用户名根目录下
            QString strShareFilePath = (char*)pdu->caMsg; // 分享的目录
            QFileInfo fileInfo(strShareFilePath);

            int idx = strShareFilePath.lastIndexOf('/');
            QString strFileName = strShareFilePath.right(strShareFilePath.size() - idx - 1); // 获取去除掉斜杠的文件名
            strRcvPath = strRcvPath + '/' + strFileName; // 拼接路径

            if(fileInfo.isFile())
            {
                QFile::copy(strShareFilePath, strRcvPath);

            }else if(fileInfo.isDir()){

                if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                {
                    return;
                }
                qDebug() << "777";
                copyDir(strShareFilePath, strRcvPath);
                qDebug() << "888";
            }

            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
        {
            char caMoveFileName[32]; // 要移动的文件名
            int iOldDirSize = 0;
            int iDesDirSize = 0;

            // 从 caData 中解析出旧目录大小、目标目录大小和文件名
            if (sscanf(pdu->caData, "%d %d %s", &iOldDirSize, &iDesDirSize, caMoveFileName) != 3)
            {
                qDebug() << "iOldDirSize 内容：" << iOldDirSize;
                qDebug() << "iDesDirSize 内容：" << iDesDirSize;
                qDebug() << "caMoveFileName 内容：" << caMoveFileName;
                qDebug() << "解析请求数据失败";
                break;
            }

            // 动态分配内存以存储旧目录和目标目录路径
            char* caOldDir = new char[iOldDirSize + 1];
            char* caDesDir = new char[iDesDirSize + 1];

            // 从 caMsg 中解析出旧目录和目标目录路径
            if (sscanf((char*)pdu->caMsg, "%s %s", caOldDir, caDesDir) != 2)
            {
                qDebug() << "解析路径失败，可能原因：";
                qDebug() << "1. caMsg 数据格式不匹配";
                qDebug() << "2. 路径中包含空格或特殊字符";
                qDebug() << "3. 路径长度超出预期";
                qDebug() << "caMsg 内容：" << QString::fromUtf8((char*)pdu->caMsg);
                qDebug() << "caOldDir 内容：" << caOldDir;
                qDebug() << "caDesDir 内容：" << caDesDir;
                delete[] caOldDir;
                delete[] caDesDir;
                break;
            }
            qDebug() << "移动文件：" << caMoveFileName << "从" << caOldDir << "到" << caDesDir;

            // 创建响应 PDU
            PDU* respdu = mkPDU(32); // 假设响应消息内容最大为 32 字节
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;

            // 拼接完整的旧路径和目标路径
            QString oldFilePath = QString("%1/%2").arg(caOldDir, caMoveFileName);
            QString desFilePath = QString("%1/%2").arg(caDesDir, caMoveFileName);

            // 检查目标路径是否为目录
            QFileInfo fileInfo(desFilePath);
            if (fileInfo.isDir())
            {
                strncpy(respdu->caData, MOVE_FILE_FAILED, 32);
            }
            else
            {
                // 尝试移动文件
                if (QFile::rename(oldFilePath, desFilePath))
                {
                    strncpy(respdu->caData, MOVE_FILE_OK, 32);
                }
                else
                {
                    strncpy(respdu->caData, MOVE_FILE_FAILED, 32);
                }
            }

            // 发送响应
            write((char*)respdu, respdu->uiPDULen);

            // 清理资源
            free(respdu);
            delete[] caOldDir;
            delete[] caDesDir;

            break;
        }
        default:
            break;
        }
        // 释放空间
        free(pdu);
        pdu = nullptr;
        // qDebug() << caName << caPwd << pdu->uiMsgType;
    }else{ // 如果是上传文件，则直接以二进制形式上传，而不是封装在pdu里

        qDebug() << "777";

        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;

        QByteArray buff = readAll(); // 读取
        m_file.write(buff);
        m_iReceived += buff.size();

        if (m_iReceived >= m_iTotal) {

            qDebug() << "888";
            m_file.close();
            m_bUpload = false;
            if (m_iReceived == m_iTotal) {
                strncpy(respdu->caData, UPLOAD_FILE_OK, sizeof(UPLOAD_FILE_OK));
            } else {
                strncpy(respdu->caData, UPLOAD_FILE_FAILED, sizeof(UPLOAD_FILE_FAILED));
            }
            qDebug() << "999";

            // 发送
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
        }
    }
}

void MyTcpSocket::clientOffline()
{
    OpeDB::getInstance().hanleOffline(m_strName.toStdString().c_str());
    // .c_str()好像是获得数据首地址，由此将qstring转到string再转到char*
    emit offline(this); // 这个this指的就是offline里对象指的地址。emit指发送信号
}

void MyTcpSocket::sendFileToClient()
{
    m_pTimer->stop();
    qDebug() << "101010";

    char* pData = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = m_file.read(pData, 4096);
        qDebug() << "文件读取返回值：" << ret;  // 调试信息

        if(ret > 0 && ret <= 4096)
        {
            qDebug() << "开始发送数据，大小：" << ret;  // 调试信息
            qint64 writeRet = write(pData, ret);
            qDebug() << "发送数据返回值：" << writeRet;  // 调试信息
            if (writeRet < 0)
            {
                qDebug() << "发送数据失败，退出程序";
                m_file.close();
                break;
            }
        }
        else if(ret == 0)
        {
            qDebug() << "文件读取完成";
            m_file.close();
            break;
        }
        else if(ret < 0)
        {
            qDebug() << "文件读取失败";
            m_file.close();
            break;
        }
    }
    delete[] pData;
    pData = nullptr;
    qDebug() << "111111";
}
