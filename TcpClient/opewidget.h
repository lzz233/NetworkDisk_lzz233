#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include "friend.h"
#include "book.h"
#include <QWidget>
#include <QListWidget>
#include <QStackedWidget> // 堆栈，用来实现friend和book的多个窗口，但每次只显示其中一个


class OpeWidget : public QWidget // 是否可以理解为登录后的主页面
{
    Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = nullptr);
    static OpeWidget &getInstance(); // 静态单例，返回OpeWidget类型的引用。负责登录跳转
    Friend *getFriend(); // 获取好友
    Book *getBook(); // 获取图书

public slots:
    void autoFresh();



signals:

private:
    QListWidget *m_pListW;
    Friend *m_pFriend;
    Book *m_pBook;

    QStackedWidget *m_pSW;
};

#endif // OPEWIDGET_H
