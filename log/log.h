#ifndef LOG
#define LOG
#include<iostream>
#include<stdio.h>
#include<string>
#include<stdarg.h>
#include<pthread.h>
#include"block_queue.h"
class log
{
public:
    static log* getInstance()
    {
        static log instance;
        return &instance;
    }
    static void *flush_log_thread(void* args)
    {
        log::getInstance()->async_write_log();
    }
    bool init(const char*file_name,int close_log,int log_buf_size = 8192,int split_lines = 500000,int max_queue_size = 0);
    void write_log(int level,const char *format,...);
    void flush(void);
private:
    log();
    virtual ~log();
    void* async_write_log()
    {
        std::string single_log;
        while(m_log_queue->Pop(single_log))
        {
            m_mutex.lock();
            fputs(single_log.c_str(),m_fp);
            m_mutex.unLock();
        }
    }
private:
    char dir_name[128]; // 路径名
    char log_name[128]; // 文件名
    int m_split_lines;  // 最大行数
    int m_log_buf_size; // 日志缓冲区大小
    long long m_count;  // 日志行数记录
    int m_today;        // 记录哪一天
    FILE *m_fp;         // 文件指针
    char *m_buf;
    block_queue<std::string> *m_log_queue;
    bool m_is_asyc;     // 是否同步标志位
    locker m_mutex;
    int m_close_log;
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {log::getInstance()->write_log(0, format, ##__VA_ARGS__); log::getInstance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {log::getInstance()->write_log(1, format, ##__VA_ARGS__); log::getInstance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {log::getInstance()->write_log(2, format, ##__VA_ARGS__); log::getInstance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {log::getInstance()->write_log(3, format, ##__VA_ARGS__); log::getInstance()->flush();}

#endif
#ifndef LOG
#define LOG
#include<iostream>
#include<stdio.h>
#include<string>
#include<stdarg.h>
#include<pthread.h>
#include"block_queue.h"
class log
{
public:
    static log* getInstance()
    {
        static log instance;
        return &instance;
    }
    static void *flush_log_thread(void* args)
    {
        log::getInstance()->async_write_log();
    }
    bool init(const char*file_name,int close_log,int log_buf_size = 8192,int split_lines = 500000,int max_queue_size = 0);
    void write_log(int level,const char *format,...);
    void flush(void);
private:
    log();
    virtual ~log();
    void* async_write_log()
    {
        std::string single_log;
        while(m_log_queue->Pop(single_log))
        {
            m_mutex.lock();
            fputs(single_log.c_str(),m_fp);
            m_mutex.unLock();
        }
    }
private:
    char dir_name[128]; // 路径名
    char log_name[128]; // 文件名
    int m_split_lines;  // 最大行数
    int m_log_buf_size; // 日志缓冲区大小
    long long m_count;  // 日志行数记录
    int m_today;        // 记录哪一天
    FILE *m_fp;         // 文件指针
    char *m_buf;
    block_queue<std::string> *m_log_queue;
    bool m_is_asyc;     // 是否同步标志位
    locker m_mutex;
    int m_close_log;
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {log::getInstance()->write_log(0, format, ##__VA_ARGS__); log::getInstance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {log::getInstance()->write_log(1, format, ##__VA_ARGS__); log::getInstance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {log::getInstance()->write_log(2, format, ##__VA_ARGS__); log::getInstance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {log::getInstance()->write_log(3, format, ##__VA_ARGS__); log::getInstance()->flush();}

#endif