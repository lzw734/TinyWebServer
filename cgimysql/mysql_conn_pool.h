#ifndef MYSQL_CONN_POOL
#define MYSQL_CONN_POOL
#include<stdio.h>
#include<string>
#include<list>
#include<mysql/mysql.h>
#include"../lock/lock.h"
#include"../log/log.h"
using namespace std;
class connection_pool
{
public:
    MYSQL* getConnection();
    bool releaseConnecttion(MYSQL* conn);
    int getFreeConn(); // 获取空闲连接

    static connection_pool *GetInstance();
    void init(string url,int port,string user,string password,string dbName,int maxConn,int close_log);
private:
    connection_pool();
    ~connection_pool();
    void Destory();
    int m_maxConn;
    int m_curConn;
    int m_freeConn;
    locker lock;
    list<MYSQL *> connList;
    sem reserve;

public:
    string m_url;
    int m_port;
    string m_user;
    string m_password;
    string m_dbName;
    int m_close_log;
};

class connectionRALL
{
public:
    connectionRALL(MYSQL* *conn,connection_pool *connpool);
    ~connectionRALL();
private:
    MYSQL* conRALL;
    connection_pool *poolRALL;
};

#endif
#ifndef MYSQL_CONN_POOL
#define MYSQL_CONN_POOL
#include<stdio.h>
#include<string>
#include<list>
#include<mysql/mysql.h>
#include"../lock/lock.h"
#include"../log/log.h"
using namespace std;
class connection_pool
{
public:
    MYSQL* getConnection();
    bool releaseConnecttion(MYSQL* conn);
    int getFreeConn(); // 获取空闲连接

    static connection_pool *GetInstance();
    void init(string url,int port,string user,string password,string dbName,int maxConn,int close_log);
private:
    connection_pool();
    ~connection_pool();
    void Destory();
    int m_maxConn;
    int m_curConn;
    int m_freeConn;
    locker lock;
    list<MYSQL *> connList;
    sem reserve;

public:
    string m_url;
    int m_port;
    string m_user;
    string m_password;
    string m_dbName;
    int m_close_log;
};

class connectionRALL
{
public:
    connectionRALL(MYSQL* *conn,connection_pool *connpool);
    ~connectionRALL();
private:
    MYSQL* conRALL;
    connection_pool *poolRALL;
};

#endif