#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QListWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout> // 垂直布局
#include <QHBoxLayout> // 水平布局
#include <QButtonGroup> // 统一管理按钮
#include <QScrollArea> // 滚动浏览区域
#include <QTimer> // 计时器
#include <QListWidget>
#include "protocol.h"

class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);

    static ShareFile &getInstance();

    void updateFriend(QListWidget *pFriendList);

signals:

public slots:
    void cancelSelect();
    void selectAll();

    void okShare();
    void cancelShare();

private:
    QPushButton *m_pSelectAllPB; // 全选
    QPushButton *m_pCancelSelectPB; // 反选
    QPushButton *m_pOKPB; // 确定分享
    QPushButton *m_pCancelPB; // 取消分享

    QScrollArea *m_pSA; // 滚动浏览区域
    QWidget *m_pFriendW; // 用来展示的东西

    QVBoxLayout *m_pFriendWVBL; // 选择好友界面的垂直布局
    QButtonGroup *m_pButtonGroup; // 按钮组
};

#endif // SHAREFILE_H
