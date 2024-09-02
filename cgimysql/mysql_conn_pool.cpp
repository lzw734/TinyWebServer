#include<mysql/mysql.h>
#include<stdio.h>
#include<string>
#include<string.h>
#include"mysql_conn_pool.h"
#include"../log/log.h"

connection_pool::connection_pool()
{
    m_curConn = 0;
    m_freeConn = 0;
}

connection_pool* connection_pool::GetInstance()
{
    static connection_pool conn_pool;
    return &conn_pool;
}

// 初始化MySQL连接
void connection_pool::init(string url,int port,string user,string password,string dbName,int maxConn,int close_log)
{
    m_url = url;
    m_port = port;
    m_user = user;
    m_password = password;
    m_dbName = dbName;
    m_maxConn = maxConn;
    m_close_log = close_log;

    for(int i = 0; i < maxConn; i++)
    {
        MYSQL *con = NULL;
        con = mysql_init(con);
        if(con == NULL)
        {
            LOG_ERROR("mysql error");
            exit(1);
        }
        con = mysql_real_connect(con, url.c_str(), user.c_str(), password.c_str(), dbName.c_str(), port, NULL, 0);
        // if(con == NULL)
        // {
        //     LOG_ERROR("mysql conn error");
        //     exit(1);
        // }

        connList.push_back(con); 
        m_freeConn++;
    }
    reserve = sem(m_freeConn);
    m_maxConn = m_freeConn;
}

MYSQL* connection_pool::getConnection()
{
    MYSQL* con;
    if(0 == connList.size())
    {
         LOG_WARN("conn is empty")
        return NULL;
    }

    lock.lock();
    con = connList.front();
    connList.pop_front();

    ++m_curConn;
    --m_freeConn;
    lock.unLock();
    return con;
}

bool connection_pool::releaseConnecttion(MYSQL* conn)
{
    if(NULL == conn)
    {
        LOG_WARN("传入的mysql连接为空,无法释放");
        return false;
    }

    lock.lock();
    connList.push_back(conn);
    m_curConn--;
    m_freeConn++;
    lock.unLock();

    reserve.post();
    return true;
}

// 销毁数据库连接池
void connection_pool::Destory()
{
    lock.lock();
    if(m_maxConn > 0)
    {
        list<MYSQL*>::iterator it;
        for(it = connList.begin();it != connList.end();it++)
        {
            mysql_close(*it);
        }
        m_curConn = 0;
        m_maxConn = 0;
        connList.clear();
    }
    lock.unLock();
}

// 获取数据库连接池数量
int connection_pool::getFreeConn()
{
    return this->m_freeConn;
}

connection_pool::~connection_pool()
{
    Destory();
}

connectionRALL::connectionRALL(MYSQL**conn,connection_pool *connpool)
{
    *conn = connpool->getConnection();
    if(! *conn)
    {
        exit(1);
    }
    conRALL = *conn;
    poolRALL = connpool;
}

connectionRALL::~connectionRALL()
{
    poolRALL->releaseConnecttion(conRALL);
}