#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase> // 连接数据库
#include <QSqlQuery>    // 查询数据库，数据库操作
#include <QStringList>

class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    static OpeDB &getInstance();
    void init();
    ~OpeDB();
    bool handelRegist(const char *name, const char *pwd); // 处理注册操作
    bool handelLogin(const char *name, const char *pwd); // 处理登录操作
    void hanleOffline(const char *name); // 处理下线操作
    QStringList handleAllOnline(); // 处理查看在线用户
    int handleSearchUsr(const char *name); // 处理查找用户
    int handleAddFriend(const char *pername, const char *name); // 处理加好友
    bool handleAddFriendAgree(const char *pername, const char *name); // 处理同意加好友
    QStringList handleFreshFriend(const char *name); // 处理刷新好友列表
    bool handleDeleteFriend(const char *pername, const char *name); // 处理删好友

signals:

public slots:
private:
    QSqlDatabase m_db; // 连接数据库
};

#endif // OPEDB_H
