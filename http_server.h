#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<error.h>
#include<fcntl.h>
#include<stdlib.h>
#include<cassert>
#include<sys/epoll.h>

#include "./threadpool/threadpool.h"
#include "./http_conn/http_conn.h"

const int MAX_FD = 65536;
const int MAX_EVENT_NUMBER = 10000;
const int TIMESLOT = 5;

class webServer
{
public:
    webServer();
    ~webServer();

    void init(int port,string user,string passwd,string dbnanme,
            int log_write,int opt_linger,int trigmode,int sql_num,
            int thread_num,int close_log,int actor_model
    );
    void threadPool();
    void sql_pool();
    void log_write();
    void trig_model();
    void eventListen();
    void eventLoop();
    void timer(int connfd,struct sockaddr_in client_address);
    void adjust_timer(util_timer *timer);
    void deal_timer(util_timer *timer,int sockfd);
    bool dealclientdata();
    bool dealwithsignal(bool &timeout,bool &stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);
public:
    int m_port;
    char *m_root;
    int m_log_write;
    int m_close_log;
    int m_actormodel;

    int m_piefd[2];
    int m_epollfd;
    http_conn *m_users;

    // 数据库相关
    connection_pool *m_connPool;
    string m_userName;
    string m_userPasswd;
    string m_dbName;
    int m_sql_num;

    // 线程池相关
    threadpool<http_conn> *m_threadPool;
    int m_thread_num;

    // epoll_event相关
    epoll_event m_events[MAX_EVENT_NUMBER];

    int m_listenfd;
    int m_OPT_LINGER;
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_CONNTrigmode;

    // 定时器相关
    client_data *users_timer;
    utils m_utils;

};

#endif // HTTPSERVER_H