#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout> // 垂直布局
#include <QHBoxLayout> // 水平布局
#include <QTimer> // 计时器
#include "protocol.h"

class Book : public QWidget // 图书的界面设计
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);

    void updateFileList(const PDU *pdu); // 刷新文件列表函数
    QString getEnterDir();

    void setDownloadStatus(bool status); // 设置下载状态
    bool getDownloadStatus();
    void setFileSavePath(QString path);
    QString getFileSavePath();

    void setFileTotal(qint64 total); // 分别设置和取得下载文件用的信息
    qint64 getFileTotal();
    void setFileRecved(qint64 total);
    qint64 getFileRecved();

    QString getShareFileName();

public slots:
    void createDir(); // 创建目录
    void freshFile(); // 刷新文件
    void delDir(); // 删除目录
    void reNameFile(); // 重命名文件
    void enterDir(QListWidgetItem*); // 进入文件夹
    void returnPre(); // 返回上级文件夹
    void uploadFile(); // 上传文件
    void downloadFile(); // 下载文件
    void shareFile(); // 分享文件
    void moveFile(); // 移动文件
    void setSelectDir(); // 选择目标文件

    void uploadFileData(); // 定时器到了之后上传文件，防止粘包


signals:

private:
    QListWidget *m_pBookListW; // 显示图书的列表

    // 按钮
    QPushButton *m_pReturnPB; // 返回
    QPushButton *m_pCreateDirPB; // 创建路径
    QPushButton *m_pDelDirPB; // 删除路径
    QPushButton *m_pRenamePB; // 重命名
    QPushButton *m_pFreshFilePB; // 刷新
    QPushButton *m_pUploadPB; // 上传文件
    QPushButton *m_pDownloadPB; // 下载文件
    QPushButton *m_pDelFilePB; // 删除文件
    QPushButton *m_pShareFilePB; // 分享文件
    QPushButton *m_pMoveFilePB; // 移动文件
    QPushButton *m_pSelectDirPB; // 选择移动文件夹

    QString m_strEnterDir; // 记录进入的目录
    QString m_strUploadPath; // 记录上传文件的路径

    QTimer *m_pTimer; // 创建定时器

    QString m_strSaveFilePath; // 创建下载保存路径
    bool m_bDownload; // 记录是否处于下载状态

    qint64 m_iFileTotal; // 总的下载大小
    qint64 m_iFileRecved; // 收到的下载大小

    QString m_strShareFileName; // 分享的文件名

    QString m_strMoveFileName; // 要移动的文件名
    QString m_strMoveFilePath; // 要移动的文件目录
    QString m_strMoveDesPath; // 要移动的文件目录
};

#endif // BOOK_H
