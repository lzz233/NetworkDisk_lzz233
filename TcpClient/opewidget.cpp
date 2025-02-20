#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent)
    : QWidget{parent}
{
    m_pListW = new QListWidget(this);
    m_pListW->addItem("好友");
    m_pListW->addItem("图书");

    m_pFriend = new Friend;
    m_pBook = new Book;

    m_pSW = new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    QHBoxLayout *pMain = new QHBoxLayout; // 水平布局
    pMain->addWidget(m_pListW); // 列表窗口
    pMain->addWidget(m_pSW); // 堆栈窗口

    setLayout(pMain);

    // 将m_pListWidget的行号变化信号与m_pSW的设置当前页面槽函数关联
    connect(m_pListW, SIGNAL(currentRowChanged(int)) // 函数参数为改变后的行号
            , m_pSW, SLOT(setCurrentIndex(int))); // 函数参数为设置的页面下标

    // 连接选中项变化信号到自定义槽函数
    connect(m_pListW, SIGNAL(itemSelectionChanged())
            , this, SLOT(autoFresh()));
}

OpeWidget &OpeWidget::getInstance() // 调用这个函数的时候，始终返回同一个对象的引用
{
    static OpeWidget instance;
    return instance;
}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pBook;
}

void OpeWidget::autoFresh()
{
    QList<QListWidgetItem*> selectedItems = m_pListW->selectedItems(); // 获取所有选中的项
    if (selectedItems.isEmpty()) {
        return; // 如果没有选中项，直接返回
    }

    QString selectedText = selectedItems.first()->text(); // 获取第一个选中的项的文本

    if (selectedText == "好友") {
        m_pFriend->freshFriend(); // 刷新好友列表
    } else if (selectedText == "图书") {
        m_pBook->freshFile(); // 刷新图书列表
    }
}


