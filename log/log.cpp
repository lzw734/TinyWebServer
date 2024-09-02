#include<string.h>
#include<time.h>
#include<sys/time.h>
#include"log.h"
#include<pthread.h>

log::log()
{
    m_count = 0;
    m_is_asyc = false;
}

log::~log()
{
    if(m_fp != NULL)
    {
        fclose(m_fp);
    }
}

bool log::init(const char*file_name,int close_log,int log_buf_size,int split_lines,int max_queue_size)
{
    if(max_queue_size >= 1)
    {
        m_is_asyc = true;
        m_log_queue = new block_queue<std::string>(max_queue_size);
        pthread_t pd;
        pthread_create(&pd,NULL,flush_log_thread,NULL);
    }

    m_close_log = close_log;
    m_log_buf_size = log_buf_size;
    m_buf = new char[log_buf_size];
    memset(m_buf,'\0',m_log_buf_size);
    m_split_lines = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    const char *p = strchr(file_name,'/');
    char log_full_name[256] = {0};

    if(p == NULL)
    {
        snprintf(log_full_name,255,"%d_%02d_%02d_%s",my_tm.tm_year + 1900,my_tm.tm_mon + 1,my_tm.tm_mday,file_name);
    }
    else
    {
        strcpy(log_name,p + 1);
        strncpy(dir_name,file_name,p -file_name + 1);
        snprintf(log_full_name,255,"%s%d_%02d_%02d%s",dir_name,my_tm.tm_year + 1900,my_tm.tm_mon + 1,my_tm.tm_mday,log_name);
    }
    m_today = my_tm.tm_mday;
    m_fp = fopen(log_full_name,"a");
    if(m_fp == NULL)
    {
        return false;
    }
    return true;
}

void log::write_log(int level,const char* format,...)
{
    // 获取当前时间并转为tm类型
    struct timeval now = {0,0};
    gettimeofday(&now,NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_time = *sys_tm;
    char s[16];
    switch(level)
    {
        case 0:
            strcpy(s,"[debug]");
            break;
        case 1:
            strcpy(s,"[info]");
            break;
        case 2:
            strcpy(s,"[warn]");
            break;
        case 3:
            strcpy(s,"[error]");
            break;
        default:
            strcpy(s,"[info]");
            break;
    }

    // 写日志
    m_mutex.lock();
    m_count++;
    if(m_today != my_time.tm_mday || m_count % m_split_lines == 0)
    {
        char newlog[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};
        snprintf(tail,16,"%d_%02d_%02d_",my_time.tm_year + 1900,my_time.tm_mon,my_time.tm_mday);
        if(m_today != my_time.tm_mday)
        {
            snprintf(newlog,255,"%s%s%s",dir_name,tail,log_name);
            m_today = my_time.tm_mday;
            m_count = 0;
        }
        else
        {
            snprintf(newlog,255,"%s%s%s.%lld",dir_name,tail,log_name,m_count / m_split_lines);
        }
        m_fp = fopen(newlog,"a");
    }
    m_mutex.unLock();

    va_list valst;
    va_start(valst,format);
    std::string logstr;
    m_mutex.lock();
    int n = snprintf(m_buf,48,"%d-%02d-%02d %02d:%02d:%02d.%06ld %s",
    my_time.tm_year,my_time.tm_mon,my_time.tm_mday,     //  年-月-日
    my_time.tm_hour,my_time.tm_min,my_time.tm_sec,      // 时-分-秒
    now.tv_usec,s);

    int m = vsnprintf(m_buf + n,m_log_buf_size - n - 1,format,valst);
    m_buf[n + m] = '\n';
    m_buf[n + m + 1] = '\0';
    logstr = m_buf;
    m_mutex.unLock();

    if(m_is_asyc && !m_log_queue->full())
    {
        m_log_queue->Push(logstr);
    }
    else
    {
        m_mutex.lock();
        fputs(logstr.c_str(),m_fp);
        m_mutex.unLock();
    }
    va_end(valst);
}

void log::flush()
{
    m_mutex.lock();
    fflush(m_fp);
    m_mutex.unLock();
}


#include<string.h>
#include<time.h>
#include<sys/time.h>
#include"log.h"
#include<pthread.h>

log::log()
{
    m_count = 0;
    m_is_asyc = false;
}

log::~log()
{
    if(m_fp != NULL)
    {
        fclose(m_fp);
    }
}

bool log::init(const char*file_name,int close_log,int log_buf_size,int split_lines,int max_queue_size)
{
    if(max_queue_size >= 1)
    {
        m_is_asyc = true;
        m_log_queue = new block_queue<std::string>(max_queue_size);
        pthread_t pd;
        pthread_create(&pd,NULL,flush_log_thread,NULL);
    }

    m_close_log = close_log;
    m_log_buf_size = log_buf_size;
    m_buf = new char[log_buf_size];
    memset(m_buf,'\0',m_log_buf_size);
    m_split_lines = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    const char *p = strchr(file_name,'/');
    char log_full_name[256] = {0};

    if(p == NULL)
    {
        snprintf(log_full_name,255,"%d_%02d_%02d_%s",my_tm.tm_year + 1900,my_tm.tm_mon + 1,my_tm.tm_mday,file_name);
    }
    else
    {
        strcpy(log_name,p + 1);
        strncpy(dir_name,file_name,p -file_name + 1);
        snprintf(log_full_name,255,"%s%d_%02d_%02d%s",dir_name,my_tm.tm_year + 1900,my_tm.tm_mon + 1,my_tm.tm_mday,log_name);
    }
    m_today = my_tm.tm_mday;
    m_fp = fopen(log_full_name,"a");
    if(m_fp == NULL)
    {
        return false;
    }
    return true;
}

void log::write_log(int level,const char* format,...)
{
    // 获取当前时间并转为tm类型
    struct timeval now = {0,0};
    gettimeofday(&now,NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_time = *sys_tm;
    char s[16];
    switch(level)
    {
        case 0:
            strcpy(s,"[debug]");
            break;
        case 1:
            strcpy(s,"[info]");
            break;
        case 2:
            strcpy(s,"[warn]");
            break;
        case 3:
            strcpy(s,"[error]");
            break;
        default:
            strcpy(s,"[info]");
            break;
    }

    // 写日志
    m_mutex.lock();
    m_count++;
    if(m_today != my_time.tm_mday || m_count % m_split_lines == 0)
    {
        char newlog[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};
        snprintf(tail,16,"%d_%02d_%02d_",my_time.tm_year + 1900,my_time.tm_mon,my_time.tm_mday);
        if(m_today != my_time.tm_mday)
        {
            snprintf(newlog,255,"%s%s%s",dir_name,tail,log_name);
            m_today = my_time.tm_mday;
            m_count = 0;
        }
        else
        {
            snprintf(newlog,255,"%s%s%s.%lld",dir_name,tail,log_name,m_count / m_split_lines);
        }
        m_fp = fopen(newlog,"a");
    }
    m_mutex.unLock();

    va_list valst;
    va_start(valst,format);
    std::string logstr;
    m_mutex.lock();
    int n = snprintf(m_buf,48,"%d-%02d-%02d %02d:%02d:%02d.%06ld %s",
    my_time.tm_year,my_time.tm_mon,my_time.tm_mday,     //  年-月-日
    my_time.tm_hour,my_time.tm_min,my_time.tm_sec,      // 时-分-秒
    now.tv_usec,s);

    int m = vsnprintf(m_buf + n,m_log_buf_size - n - 1,format,valst);
    m_buf[n + m] = '\n';
    m_buf[n + m + 1] = '\0';
    logstr = m_buf;
    m_mutex.unLock();

    if(m_is_asyc && !m_log_queue->full())
    {
        m_log_queue->Push(logstr);
    }
    else
    {
        m_mutex.lock();
        fputs(logstr.c_str(),m_fp);
        m_mutex.unLock();
    }
    va_end(valst);
}

void log::flush()
{
    m_mutex.lock();
    fflush(m_fp);
    m_mutex.unLock();
}

