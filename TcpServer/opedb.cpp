#include "opedb.h"
#include <QMessageBox>
#include <QDebug>
#include "string.h"

OpeDB::OpeDB(QObject *parent) // 构造函数
    : QObject{parent}
{
    // 连接数据库
    m_db = QSqlDatabase::addDatabase("QSQLITE"); // 连接的数据库类型
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init() // 初始化
{
    m_db.setHostName("localhost"); // 连接哪里就写哪里的IP地址，本地就写localhost
    m_db.setDatabaseName("C:\\Users\\22162\\Documents\\TcpServer\\cloud.db");
    if(m_db.open()){
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while(query.next()){
            QString data = QString("%1, %2, %3, %4").arg(query.value(0).toString()).arg(query.value(1).toString())
                               .arg(query.value(2).toString()).arg(query.value(3).toString());
            qDebug() << data;
        }
    }else{
        QMessageBox::critical(nullptr, "打开数据库", "打开数据库失败");
    }
}

OpeDB::~OpeDB() // 析构函数
{
    m_db.close();
}

bool OpeDB::handelRegist(const char *name, const char *pwd) // 处理注册请求
{
    if(name == nullptr || pwd == nullptr){
        return false;
    }
    QString data = QString("insert into usrInfo(name, pwd) values(\'%1\',\'%2\')").arg(name, pwd);
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handelLogin(const char *name, const char *pwd)
{
    if(name == nullptr || pwd == nullptr){
        return false;
    }
    QString data = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online = 0")
                       .arg(name, pwd);
    QSqlQuery query;
    query.exec(data);
    if(query.next()){ // next直接自动获取一条一条的信息，匹配上了就是true，直接存到query里，没匹配上就是false
        data = QString("update usrInfo set online = 1 where name = \'%1\' and pwd = \'%2\'")
                   .arg(name, pwd);
        QSqlQuery query;
        query.exec(data);
        return true;
    }else{
        return false;
    }
}

void OpeDB::hanleOffline(const char *name)
{
    if(name == nullptr){
        return;
    }
    QString data = QString("update usrInfo set online = 0 where name = \'%1\'")
                       .arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handleAllOnline()
{
    QString data = QString("select name from usrInfo where online = 1");
    QSqlQuery query;
    query.exec(data);

    QStringList result;
    result.clear(); // 先清理一下

    while(query.next()){
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSearchUsr(const char *name)
{
    if(name == nullptr)
    {
        return -1;
    }
    QString data = QString("select online from usrInfo where name = \'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        int ret = query.value(0).toInt();
        if(ret == 1)
        {
            return 1;
        }
        else if(ret == 0)
        {
            return 0;
        } // 存在并在线返回1，存在不在线返回0
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if(name == nullptr || pername == nullptr)
    {
        return -1; // 输入错误
    }
    QString data = QString("select * from friend where (id=(select id from usrInfo where name =\'%1\') and friendId = (select id from usrInfo where name =\'%2\'))"
                           " or (id=(select id from usrInfo where name =\'%3\') and friendId = (select id from usrInfo where name =\'%4\'))")
                       .arg(pername, name, name, pername);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        return 0; // 双方已是好友
    }
    else
    {
        QString data = QString("select online from usrInfo where name = \'%1\'").arg(pername);
        QSqlQuery query;
        query.exec(data);
        if(query.next())
        {
            return 1; // 找到相应用户了
        }
        else
        {
            return 2; // 没找到相应用户
        }
    }
}

bool OpeDB::handleAddFriendAgree(const char *pername, const char *name)
{
    if (name == nullptr || pername == nullptr)
    {
        return false; // 输入错误
    }

    // 查找两个人对应的序号
    QSqlQuery query;
    query.prepare("SELECT id FROM usrInfo WHERE name = :name");

    // 查询第一个用户的ID
    query.bindValue(":name", pername); // 直接使用字符串拼接构建 SQL 语句存在 SQL 注入风险，建议使用参数化查询。
    if (!query.exec() || !query.next()) // 在执行查询后，需要使用 query.next() 来定位到查询结果的第一条记录
    {
        return false; // 查询失败或未找到记录
    }
    int num1 = query.value(0).toInt();

    // 查询第二个用户的ID
    query.bindValue(":name", name);
    if (!query.exec() || !query.next())
    {
        return false; // 查询失败或未找到记录
    }
    int num2 = query.value(0).toInt();

    // 利用序号建立好友关系
    query.prepare("INSERT INTO friend(id, friendId) VALUES(:id, :friendId)");
    query.bindValue(":id", num1);
    query.bindValue(":friendId", num2);
    return query.exec();
}

QStringList OpeDB::handleFreshFriend(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();
    if (name == nullptr)
    {
        return strFriendList;
    }

    QSqlQuery query;
    query.prepare("SELECT name FROM usrInfo WHERE online = 1 "
                  "AND (id IN (SELECT id FROM friend "
                  "WHERE friendId = (SELECT id FROM usrInfo WHERE name = :name)) "
                  "OR id IN (SELECT friendId FROM friend "
                  "WHERE id = (SELECT id FROM usrInfo WHERE name = :name)))");
    query.bindValue(":name", name);

    if (!query.exec())
    {
        return strFriendList;
    }

    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
        // qDebug() << query.value(0).toString();
    }

    return strFriendList;
}

bool OpeDB::handleDeleteFriend(const char *pername, const char *name)
{
    if (name == nullptr || pername == nullptr)
    {
        return false; // 输入错误
    }

    QSqlQuery query;
    query.prepare("SELECT id FROM usrInfo WHERE name = :name");
    query.bindValue(":name", name);
    if (!query.exec() || !query.next())
    {
        qDebug() << "Failed to find user ID for" << name;
        return false;
    }
    int userId = query.value(0).toInt();

    query.prepare("SELECT id FROM usrInfo WHERE name = :pername");
    query.bindValue(":pername", pername);
    if (!query.exec() || !query.next())
    {
        qDebug() << "Failed to find user ID for" << pername;
        return false;
    }
    int friendId = query.value(0).toInt();

    query.prepare("DELETE FROM friend WHERE (id = :userId AND friendId = :friendId) "
                  "OR (friendId = :userId AND id = :friendId)");
    query.bindValue(":userId", userId);
    query.bindValue(":friendId", friendId);

    return query.exec();
}
