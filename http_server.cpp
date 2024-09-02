#include "./http_server.h"

webServer::webServer()
{
    m_users = new http_conn[MAX_FD];

    // root文件路径
    char server_path[200];
    getcwd(server_path,200);
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root,server_path);
    strcat(m_root,root);

    // 定时器
    users_timer = new client_data[MAX_FD];
}

webServer::~webServer()
{
    close(m_epollfd);
    close(m_listenfd);
    close(m_piefd[0]);
    close(m_piefd[1]);
    delete[] m_users;
    delete[] users_timer;
    delete m_threadPool;
}

void webServer::init(int port,string user,string passwd,string dbnanme,
            int log_write,int opt_linger,int trigmode,int sql_num,
            int thread_num,int close_log,int actor_model)
{
    m_port = port;
    m_userName = user;
    m_userPasswd = passwd;
    m_dbName = dbnanme;
    m_log_write = log_write;
    m_OPT_LINGER = opt_linger;
    m_TRIGMode = trigmode;
    m_close_log = close_log;
    m_sql_num = sql_num;
    m_thread_num = thread_num;
    m_actormodel = actor_model;
}

void webServer::trig_model()
{
    if(0 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 0;
    }
    else if(1 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 1;
    }
    else if(2 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 0;
    }
    else if(3 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 1;
    }
}

void webServer::log_write()
{
    if(0 == m_close_log)
    {
        if(1 == m_log_write)
        {
            log::getInstance()->init("./Serverlog",m_close_log,2000,800000,800);
        }
        else
        {
            log::getInstance()->init("./Serverlog",m_close_log,2000,800000,0);
        }
    }
}

void webServer::sql_pool()
{
    m_connPool = connection_pool::GetInstance();
    m_connPool->init("localhost",3307,m_userName,m_userPasswd,m_dbName,m_sql_num,m_close_log);
    m_users->initmysql_ret(m_connPool);
}

void webServer::threadPool()
{
    m_threadPool = new threadpool<http_conn>(m_actormodel,m_connPool,m_thread_num);
}

void webServer::eventListen()
{
    m_listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(m_listenfd >= 0);

    if(0 == m_OPT_LINGER)
    {
        struct linger tmp = {0,1};
        setsockopt(m_listenfd,SOL_SOCKET,SO_LINGER,&tmp,sizeof(tmp));
    }
    else if(1 == m_OPT_LINGER)
    {
        struct linger tmp = {1,1};
        setsockopt(m_listenfd,SOL_SOCKET,SO_LINGER,&tmp,sizeof(tmp));
    }
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
    ret = bind(m_listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listenfd,5);
    assert(ret >= 0);

    m_utils.init(TIMESLOT);

    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert(m_epollfd >= 0);

    m_utils.addfd(m_epollfd,m_listenfd,false,m_LISTENTrigmode);
    http_conn::m_epollfd = m_epollfd;

    ret = socketpair(PF_UNIX,SOCK_STREAM,0,m_piefd);
    assert(ret >= 0);
    m_utils.setnonblocking(m_piefd[1]);
    m_utils.addfd(m_epollfd,m_piefd[0],false,0);

    m_utils.addsig(SIGPIPE,SIG_IGN);
    m_utils.addsig(SIGALRM,m_utils.sig_handler,false);
    m_utils.addsig(SIGTERM,m_utils.sig_handler,false);
    alarm(TIMESLOT);

    utils::u_piefd = m_piefd;
    utils::u_epollfd = m_epollfd;
}

void webServer::timer(int connfd, struct sockaddr_in client_address)
{
    m_users[connfd].init(connfd,client_address,m_root,m_CONNTrigmode,m_close_log,m_userName,m_userPasswd,m_dbName);

    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;
    util_timer *timer = new util_timer;
    timer->user_data = &users_timer[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    users_timer[connfd].timer = timer;
    m_utils.m_timer_lst.add_timer(timer);
}

void webServer::adjust_timer(util_timer *timer)
{
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    m_utils.m_timer_lst.adjust_timer(timer);
    LOG_INFO("%s","adjust once timer");
}

void webServer::deal_timer(util_timer *timer,int sockfd)
{
    timer->cb_func(&users_timer[sockfd]);
    if(timer)
    {
        m_utils.m_timer_lst.del_timer(timer);
    }
    LOG_INFO("close fd: %d",sockfd);
}

bool webServer::dealclientdata()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    if(0 == m_LISTENTrigmode)
    {
        int connfd = accept(m_listenfd,(struct sockaddr *)&client_address,&client_addrlength);
        if(connfd < 0)
        {
            LOG_ERROR("%s:error is:%d","accept error",errno);
        }
        if(http_conn::m_user_count >= MAX_FD)
        {
            m_utils.show_error(connfd,"Internal server busy");
            LOG_ERROR("%s","Internal server busy");
            return false;
        }
        timer(connfd,client_address);
    }
    else
    {
        while(1)
        {
            int connfd = accept(m_listenfd,(struct sockaddr *)&client_address,&client_addrlength);
            if(connfd < 0)
            {
                LOG_ERROR("%s:error is:%d","accept error",errno);
                break;
            }

            if(http_conn::m_user_count >= MAX_FD)
            {
                m_utils.show_error(connfd,"Internal server busy");
                LOG_ERROR("%s","Internal server busy");
                break;
            }
            timer(connfd,client_address);
        }
        return false;
    }
    return true;
}

bool webServer::dealwithsignal(bool &timeout,bool &stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(m_piefd[0],signals,sizeof(signals),0);
    if(ret == -1)
    {
        return false;
    }
    else if(ret == 0)
    {
        return false;
    }
    else
    {
        for(int i = 0; i < ret; i++)
        {
            switch(signals[i])
            {
                case SIGALRM:
                {
                    timeout = true;
                    break;
                }
                case SIGTERM:
                {
                    stop_server = true;
                     break;
                }
            }
        }
    }
    return true;
}

void webServer::dealwithread(int sockfd)
{
    util_timer *timer = users_timer[sockfd].timer;

    if(1 == m_actormodel)
    {
        if(timer)
        {
            adjust_timer(timer);
        }

        m_threadPool->append(m_users+sockfd,0);
        while(true)
        {
            if(1 == m_users[sockfd].improv)
            {
                if(1 == m_users[sockfd].timer_flag)
                {
                    deal_timer(timer,sockfd);
                    m_users[sockfd].timer_flag = 0;
                }
                m_users[sockfd].timer_flag = 0;
                break;
            }
        }
    }
    else
    {
        if(m_users[sockfd].read_once())
        {
            LOG_INFO("deal with the client(%s)",inet_ntoa(m_users[sockfd].getAddress()->sin_addr));

            m_threadPool->append_p(m_users + sockfd);
            if(timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            deal_timer(timer,sockfd);
        }
    }
}

void webServer::dealwithwrite(int sockfd)
{
    util_timer *timer = users_timer[sockfd].timer;

    if(1 == m_actormodel)
    {
        if(timer)
        {
            adjust_timer(timer);
        }

        m_threadPool->append(m_users + sockfd,1);
        while (1)
        {
            if(1 == m_users[sockfd].improv)
            {
                if(1 == m_users[sockfd].timer_flag)
                {
                    deal_timer(timer,sockfd);
                    m_users[sockfd].timer_flag = 0;
                }
                m_users[sockfd].improv = 0;
                break;
            }
        }
        
    }
    else
    {
        if(m_users[sockfd].write())
        {
            LOG_INFO("send data to client(%s)",inet_ntoa(m_users[sockfd].getAddress()->sin_addr));

            if(timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            deal_timer(timer,sockfd);
        }
    }
}

void webServer::eventLoop()
{
    bool timeout = false;
    bool stop_server = false;

    while(!stop_server)
    {
        int number = epoll_wait(m_epollfd,m_events,MAX_EVENT_NUMBER, - 1);
        if(number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s","epoll failure");
            std::cout << "epoll failure" << std::endl;
            break;
        }
        for(int i = 0; i < number;i++)
        {
            int sockfd = m_events[i].data.fd;

            if(sockfd == m_listenfd)
            {
                bool flag = dealclientdata();
                if(false == flag)
                    continue;
            }
            else if(m_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                util_timer *timer = users_timer[sockfd].timer;
                deal_timer(timer,sockfd);
            }
            else if(sockfd == m_piefd[0] && (m_events[i].events & EPOLLIN))
            {
                bool flag = dealwithsignal(timeout,stop_server);
                if(flag == false)
                    LOG_ERROR("%s","dealclientdata failure");
            }
            else if(m_events[i].events & EPOLLIN)
            {
                dealwithread(sockfd);
            }
            else if(m_events[i].events & EPOLLOUT)
            {
                dealwithwrite(sockfd);
            }
        }
        if(timeout)
        {
            m_utils.timer_handler();
            LOG_INFO("%s","timer tick");
            timeout = false;
        }
       
    }
    std::cout << "服务器退出了" << std::endl;
}
