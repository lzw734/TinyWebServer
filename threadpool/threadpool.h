#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<list>
#include<cstdio>
#include<exception>
#include<pthread.h>
#include "../lock/lock.h"
#include "../cgimysql/mysql_conn_pool.h"

template <typename T>
class threadpool
{
public:
    threadpool(int actor_model,connection_pool *connPool,int thread_num = 8,int max_request = 10000);
    ~threadpool();
    bool append(T* request,int state);
    bool append_p(T *request);
private:
    static void *worker(void *arg);
    void run();
private:
    int m_thread_num;
    int m_max_requests;
    pthread_t *m_threads;
    std::list<T*> m_workqueue;
    locker m_queue_lock;
    sem m_queue_stat;
    connection_pool *m_connPool;
    int m_actor_model;
};

template <typename T>
threadpool<T>::threadpool(int actor_model,connection_pool *connPool,int thread_num,int max_request):m_thread_num(thread_num),m_max_requests(max_request),m_actor_model(actor_model),m_connPool(connPool)
{
    if(thread_num <= 0 || max_request <= 0)
        throw std::exception();
    m_threads = new pthread_t[thread_num];
    if(!m_threads)
        throw std::exception();
    for(int i = 0; i < thread_num;i++)
    {
        if(pthread_create(m_threads + i,NULL,worker,this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
}

template <typename T>
bool threadpool<T>::append(T* request,int state)
{
    m_queue_lock.lock();
    if(m_workqueue.size() > m_max_requests)
    {
        m_queue_lock.unLock();
        return false;
    }
    request->m_state = state;
    m_workqueue.push_back(request);
    m_queue_lock.unLock();
    m_queue_stat.post();
    return true;
}

template <typename T>
bool threadpool<T>::append_p(T *request)
{
    m_queue_lock.lock();
    if(m_workqueue.size() > m_max_requests)
    {
        m_queue_lock.unLock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queue_lock.unLock();
    m_queue_stat.post();
    return true;
}

template <typename T>
void *threadpool<T>::worker(void *arg)
{
    threadpool *pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run()
{
    while(1)
    {
        m_queue_stat.wait();
        m_queue_lock.lock();
        if(m_workqueue.empty())
        {
            m_queue_lock.unLock();
            continue;
        }
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queue_lock.unLock();

        if(!request)
            continue;
        if(1 == m_actor_model)
        {
            if(request->m_state == 0)
            {
                if(request->read_once())
                {
                    request->improv = 1;
                    connectionRALL mysqlconn(&request->mysql,m_connPool);
                    request->process();
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
            else
            {
                if(request->write())
                {
                    request->improv = 1;
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        }
        else
        {
            connectionRALL mysqlconn(&request->mysql,m_connPool);
            request->process();
        }

    }
}


#endif // THREADPOOL_H