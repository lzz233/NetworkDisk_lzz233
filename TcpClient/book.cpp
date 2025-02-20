#include "book.h"
#include "tcpclient.h"
#include "opewidget.h"
#include "sharefile.h"
#include <QInputDialog> // 专门用来输入
#include <QMessageBox>
#include <QFileDialog> // 获得上传路径

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_strEnterDir.clear();

    m_bDownload = false;
    m_pTimer = new QTimer; // 创建定时器

    m_pBookListW = new QListWidget; // 显示图书的列表

    // 按钮
    m_pReturnPB = new QPushButton("返回"); // 返回
    m_pCreateDirPB = new QPushButton("创建文件夹"); // 创建路径
    m_pDelDirPB = new QPushButton("删除文件夹"); // 删除路径
    m_pRenamePB = new QPushButton("重命名"); // 重命名
    m_pFreshFilePB = new QPushButton("刷新"); // 刷新

    QVBoxLayout *pDirVBL = new QVBoxLayout; // 上面五个按钮用垂直布局
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFreshFilePB);

    m_pUploadPB = new QPushButton("上传文件"); // 上传文件
    m_pDownloadPB = new QPushButton("下载文件"); // 下载文件
    m_pDelFilePB = new QPushButton("删除文件"); // 删除文件
    m_pShareFilePB = new QPushButton("分享文件"); // 分享文件
    m_pMoveFilePB = new QPushButton("移动文件"); // 移动文件
    m_pSelectDirPB = new QPushButton("目标目录"); // 目标目录
    m_pSelectDirPB->setEnabled(false);

    QVBoxLayout *pFileVBL = new QVBoxLayout; // 上面六个按钮用垂直布局
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownloadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain = new QHBoxLayout; // 一起合成一个水平布局
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain); // 设置pmain为页面

    connect(m_pCreateDirPB, SIGNAL(clicked(bool))
            , this, SLOT(createDir())); // 创建文件夹按钮信号槽

    connect(m_pFreshFilePB, SIGNAL(clicked(bool))
            , this, SLOT(freshFile())); // 刷新文件按钮信号槽

    connect(m_pDelDirPB, SIGNAL(clicked(bool))
            , this, SLOT(delDir())); // 删除文件夹按钮信号槽

    connect(m_pDelFilePB, SIGNAL(clicked(bool))
            , this, SLOT(delDir())); // 删除文件按钮信号槽

    connect(m_pRenamePB, SIGNAL(clicked(bool))
            , this, SLOT(reNameFile())); // 重命名文件按钮信号槽

    connect(m_pBookListW, SIGNAL(itemDoubleClicked(QListWidgetItem*))
            , this, SLOT(enterDir(QListWidgetItem*))); // 进入文件夹按钮信号槽

    connect(m_pReturnPB, SIGNAL(clicked(bool))
            , this, SLOT(returnPre())); // 返回上级文件夹

    connect(m_pUploadPB, SIGNAL(clicked(bool))
            , this, SLOT(uploadFile())); // 上传文件

    connect(m_pTimer, SIGNAL(timeout())
            , this, SLOT(uploadFileData())); // 定时器时间到了之后上传文件

    connect(m_pDownloadPB, SIGNAL(clicked(bool))
            , this, SLOT(downloadFile())); // 下载文件

    connect(m_pShareFilePB, SIGNAL(clicked(bool))
            , this ,SLOT(shareFile())); // 分享文件

    connect(m_pMoveFilePB, SIGNAL(clicked(bool))
            , this ,SLOT(moveFile())); // 移动文件

    connect(m_pSelectDirPB, SIGNAL(clicked(bool))
            , this ,SLOT(setSelectDir())); // 选择移动文件夹

}

void Book::updateFileList(const PDU *pdu) // 刷新文件列表
{
    if(pdu == nullptr) // 有效性检查
    {
        return;
    }

    // 先清空屏幕
    for(int i = 0; i < m_pBookListW->count(); ++i) // 头一次的count默认是0
    {
        // qDebug() << m_pBookListW->count() << "haha";
        m_pBookListW->clear();
    }

    FileInfo *pFileInfo = nullptr;
    int iCnt = pdu->uiMsgLen/sizeof(FileInfo);
    for(int i = 0; i < iCnt; ++i)
    {
        pFileInfo = (FileInfo*)pdu->caMsg + i;
        // qDebug() << pFileInfo->caFileName << pFileInfo->iFileType;
        QListWidgetItem *pItem = new QListWidgetItem; // 创建一个对象

        if(pFileInfo->iFileType == 0)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/DIR.jpg"))); // 设置（显示）文件夹图标
        }else if(pFileInfo->iFileType == 1){
            pItem->setIcon(QIcon(QPixmap(":/map/FILE.jpg"))); // 设置（显示）文件图标
        }
        pItem->setText(pFileInfo->caFileName); // 设置（显示）文件名
        m_pBookListW->addItem(pItem); // 把弄好的一条文件信息加到列表里去
    }
}

QString Book::getEnterDir()
{
    return m_strEnterDir;
}

void Book::setDownloadStatus(bool status)
{
    m_bDownload = status;
}

bool Book::getDownloadStatus()
{
    return m_bDownload;
}

void Book::setFileSavePath(QString path)
{
    m_strSaveFilePath = path;
}

QString Book::getFileSavePath()
{
    return m_strSaveFilePath;
}

void Book::setFileTotal(qint64 total) // 分别设置和取得下载文件用的信息
{
    m_iFileTotal = total;
}

qint64 Book::getFileTotal()
{
    return m_iFileTotal;
}

void Book::setFileRecved(qint64 total)
{
    m_iFileRecved = total;
}

qint64 Book::getFileRecved()
{
    return m_iFileRecved;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}


void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this, "新建文件夹", "新文件夹名字"); // 创建新文件夹
    if(!strNewDir.isEmpty()){
        if(strNewDir.size() >= 32){ // 限制目录名字大小
            QMessageBox::warning(this, "新建文件夹", "新文件夹名字不能超过32字节");
        }
        QString strName = TcpClient::getInstance().getLoginName(); // 获得名字
        QString strCurPath = TcpClient::getInstance().getCurPath(); // 获得路径
        PDU *pdu = mkPDU(strCurPath.size() + 1); // 名字和文件夹名字放caData，路径放caMsg
        pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32); // 前32字节放名字
        strncpy(pdu->caData + 32, strNewDir.toStdString().c_str(), 32); // 后32字节放文件夹名字
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size()); // 路径放caMsg

        // 发送
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }else{
        QMessageBox::warning(this, "新建文件夹", "新文件夹名字不能为空");
    }
}

void Book::freshFile() // 刷新文件操作
{
    QString strCurPath = TcpClient::getInstance().getCurPath(); // 获取工作目录
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FRESH_FLIE_REQUEST;
    strncpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.size());

    // 发送
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Book::delDir() // 删除文件夹和文件
{
    QString strCurPath = TcpClient::getInstance().getCurPath(); // 获取工作目录
    QListWidgetItem *pItem = m_pBookListW->currentItem(); // 获得当前选中的选项
    if(pItem == nullptr)
    {
        QMessageBox::warning(this, "删除文件", "请选中要删除的文件");
    }else{
        QString strDelName = pItem->text(); // 获取选中选项的文字
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), 64); // 拷贝删除的名字
        memcpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.size()); // 拷贝文件夹的路径

        // 发送
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Book::reNameFile() // 重命名
{
    QString strCurPath = TcpClient::getInstance().getCurPath(); // 获取工作目录
    QListWidgetItem *pItem = m_pBookListW->currentItem(); // 获得当前选中的选项
    if(pItem == nullptr)
    {
        QMessageBox::warning(this, "重命名文件", "请选中要重命名的文件");
    }else{
        QString strOldName = pItem->text(); // 获取选中选项的文字
        QString strNewName = QInputDialog::getText(this, "重命名文件", "输入新的文件名");

        if(!strNewName.isEmpty())
        {
            PDU *pdu = mkPDU(strCurPath.size() + 1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FLIE_REQUEST;
            strncpy(pdu->caData, strOldName.toStdString().c_str(), 32); // 拷贝删除的名字
            strncpy(pdu->caData + 32, strNewName.toStdString().c_str(), 32); // 拷贝删除的名字
            memcpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.size()); // 拷贝文件夹的路径

            // 发送
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = nullptr;
        }else{
            QMessageBox::warning(this, "重命名文件", "文件名不能为空");
        }
    }
}

void Book::enterDir(QListWidgetItem*) // 进入文件夹
{
    QListWidgetItem *pItem = m_pBookListW->currentItem(); // 获得当前选中的选项

    QString strDirName = pItem->text(); // 获取选中选项的文字
    QString strCurPath = TcpClient::getInstance().getCurPath(); // 获取工作目录
    QString strPath = QString("%1/%2").arg(strCurPath, strDirName); // 拼接路径
    m_strEnterDir = strPath;

    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTRY_DIR_REQUEST;
    strncpy(pdu->caData, strDirName.toStdString().c_str(), strDirName.size()); // 拷贝文件名字
    memcpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.size()); // 拷贝文件夹的路径

    // 发送
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;

}

void Book::returnPre() // 返回上级目录
{
    QString strCurPath = TcpClient::getInstance().getCurPath(); // 获取工作目录
    QString strRootPath = "./" + TcpClient::getInstance().getLoginName(); // 获取初始目录
    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this, "返回上级目录", "返回上级目录失败：已经在最高级目录");
    }else{

        int index = strCurPath.lastIndexOf('/'); // 找最后一个斜杠的位置
        strCurPath.remove(index, strCurPath.size()-index); // 删除斜杠后面的东西

        TcpClient::getInstance().setCurPath(strCurPath); // 设置新的工作路径
        freshFile(); // 直接调用刷新文件夹
    }
}

void Book::uploadFile() // 上传文件
{
    QString strCurPath = TcpClient::getInstance().getCurPath(); // 获取工作目录
    m_strUploadPath = QFileDialog::getOpenFileName(); // 获得打开文件目录

    if(!m_strUploadPath.isEmpty())
    {
        int index = m_strUploadPath.lastIndexOf('/');
        QString strFileName = m_strUploadPath.right(m_strUploadPath.size() - index - 1); // 多减一用来减去斜杠
        qDebug() << m_strUploadPath;
        if(m_strUploadPath.isEmpty())
        {
            QMessageBox::warning(this, "上传文件", "请选择需要上传的文件！");
            return ;
        }

        QFile file(m_strUploadPath);
        qint64 fileSize = file.size(); // 获得文件大小
        qDebug() << "上传文件：" << strFileName << " " << fileSize;

        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size()); // 拷贝当前路径
        sprintf(pdu->caData, "%s %lld", strFileName.toStdString().c_str(), fileSize); // 拷贝文件名和文件大小

        // 发送
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen); // 过一会儿再传文件数据，不然容易粘包，粘在pdu后面
        free(pdu);
        pdu = nullptr;

        m_pTimer->start(1000); // 1000毫秒。计时结束之后自动调用槽函数上传文件

    }else{
        QMessageBox::warning(this, "上传文件", "上传文件不能为空");
    }
}

void Book::downloadFile() // 下载文件
{
    qDebug() << "111";
    QString strCurPath = TcpClient::getInstance().getCurPath(); // 获取工作目录
    QListWidgetItem *pItem = m_pBookListW->currentItem(); // 获得当前选中的选项
    if(pItem == nullptr)
    {
        QMessageBox::warning(this, "下载文件", "请选中要下载的文件");
    }else{

        qDebug() << "222";
        QString strFileName = pItem->text(); // 获取选中选项的文字
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        strncpy(pdu->caData, strFileName.toStdString().c_str(), 32); // 拷贝下载的名字
        memcpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.size()); // 拷贝文件夹的路径

        QString strSaveFilePath = QFileDialog::getSaveFileName(); // 弹出选择保存位置窗口
        if(strSaveFilePath.isEmpty())
        {
            QMessageBox::warning(this, "下载文件", "请指定保存位置");
            m_strSaveFilePath.clear();
            return;

        }else{
            m_strSaveFilePath = strSaveFilePath; // 保存下载保存地址
            qDebug() << "333";
        }

        // 发送
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
        qDebug() << "444";
    }
}

void Book::shareFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem(); // 获得当前选中的选项
    if(pItem == nullptr)
    {
        QMessageBox::warning(this, "分享文件", "请选中要分享的文件");
        return;
    }else{

        m_strShareFileName = pItem->text(); // 获取选中选项的文字
        Friend *pFriend = OpeWidget::getInstance().getFriend();
        QListWidget *pFriendList = pFriend->getFriendList(); // 获取好友列表
        ShareFile::getInstance().updateFriend(pFriendList); // 好友列表传给ShareFile的updateFriend
        if(ShareFile::getInstance().isHidden())
        {
            ShareFile::getInstance().show();
        }
    }
}

void Book::moveFile() // 移动文件
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(pCurItem == nullptr)
    {
        QMessageBox::warning(this, "移动文件", "请选择需要移动的文件！");
        return;
    }

    m_strMoveFileName = pCurItem -> text().split('\t')[0]; // 设置需要移动的文件名
    m_strMoveFilePath = TcpClient::getInstance().getCurPath(); // 设置移动文件的原目录

    m_pSelectDirPB->setEnabled(true); // 设置目标目录可点击
    QMessageBox::information(this, "移动文件", "请跳转到需要移动到的目录，\n然后点击“目标目录”按钮。");
}

void Book::setSelectDir() // 选择移动的目标文件
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if (pCurItem == nullptr)
    {
        QMessageBox::warning(this, "移动文件", "请选择需要移动的文件！");
        return;
    }

    // 获取当前路径和目标目录
    QString strCurDir = TcpClient::getInstance().getCurPath();
    QString strDesDir = pCurItem->text();

    // 使用 QDir 拼接目标路径，确保路径正确性
    m_strMoveDesPath = QDir(strCurDir).filePath(strDesDir);

    // 弹出确认对话框
    QString strMoveAffirm = QString("您确认将 %1 的 %2 文件\n移动到 %3 目录下吗？")
                                .arg(m_strMoveFilePath, m_strMoveFileName, m_strMoveDesPath);
    QMessageBox::StandardButton sbMoveAffirm = QMessageBox::question(this, "移动文件", strMoveAffirm);

    if (sbMoveAffirm == QMessageBox::No) // 用户取消移动
    {
        m_strMoveFilePath.clear();
        m_strMoveFileName.clear();
        m_strMoveDesPath.clear();
        m_pSelectDirPB->setEnabled(false);
        return;
    }

    PDU *pdu = mkPDU(m_strMoveFilePath.size() + m_strMoveDesPath.size() + 5);
    if (!pdu) // 检查 PDU 分配是否成功
    {
        QMessageBox::critical(this, "移动文件", "内存分配失败！");
        return;
    }

    pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;

    // 填充 caData：文件路径长度 + 目标路径长度 + 文件名
    sprintf(pdu -> caData, "%d %d %s",
        m_strMoveFilePath.size(),
        m_strMoveDesPath.size(),
        m_strMoveFileName.toStdString().c_str());
// if (sscanf(pdu->caData, "%d %d %31s", &iOldDirSize, &iDesDirSize, caMoveFileName)

    // 填充实际消息内容
    sprintf((char*)pdu -> caMsg, "%s %s",
            m_strMoveFilePath.toStdString().c_str(),
            m_strMoveDesPath.toStdString().c_str());

    // 发送请求并检查返回值
    if (!TcpClient::getInstance().getTcpSocket().write((char *)pdu, pdu->uiPDULen))
    {
        QMessageBox::critical(this, "移动文件", "网络请求失败！");
    }

    // 清理资源
    free(pdu);

    // 清理状态
    m_strMoveFilePath.clear();
    m_strMoveFileName.clear();
    m_strMoveDesPath.clear();
    m_pSelectDirPB->setEnabled(false);
}

void Book::uploadFileData() // 计时结束之后自动调用槽函数上传文件
{
    m_pTimer->stop(); // 先停止，他自己不会自动停

    QFile file(m_strUploadPath);
    if(!file.open(QIODevice::ReadOnly)) // 只读方式打开
    {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }else{

        char *pBuffer = new char[4096]; // 定义读取缓冲区，4096的读写速度最快
        qint64 ret = 0;
        while(true)
        {
            ret = file.read(pBuffer, 4096);
            if(ret > 0 && ret <= 4096) // 每次读4096的，只要读出来有东西就继续读，并发出去
            {
                TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);

            }else if(ret == 0){ // 直到返回值为0，代表读完了

                break;

            }else{

                QMessageBox::warning(this, "上传文件", "上传文件失败：读文件失败");
                break;
            }
        }
        file.close();
        delete[] pBuffer;
        pBuffer = nullptr;
        m_strUploadPath.clear(); // 清除上传文件夹名，以免影响之后上传操作
    }
}
