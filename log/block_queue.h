#ifndef BLOCK_QUEUE
#define BLOCK_QUEUE
#include<iostream>
#include<pthread.h>
#include<sys/time.h>
#include "../lock/lock.h"

template<class T>
class block_queue
{
public:
    block_queue(int maxsize = 1000)
    {
        if(maxsize <= 0)
        {
            exit(-1);
        }
        m_max_size = maxsize;
        m_array = new T[maxsize];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }
    ~block_queue()
    {
        m_mutex.lock();
        if(m_array != NULL)
        {
            delete m_array;
        }
        m_mutex.unLock();
    }
    // 清除队列
    void clear()
    {
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unLock();
    }

    // 判断队列是否满了
    bool full()
    {
        bool isFull = false;
        m_mutex.lock();
        if(m_size >= m_max_size)
        {
            isFull = true;
        }
        m_mutex.unLock();
        return isFull;
    }

    // 判断队列是否为空
    bool empty()
    {
        bool isEmpty = true;
        m_mutex.lock();
        if(m_size <= 0)
        {
            isEmpty = true;
        }
        m_mutex.unLock();
        return isEmpty;
    }
    // 获取队头
    bool Front(T &value)
    {
        m_mutex.lock();
        if(m_size == 0)
        {
            m_mutex.unLock();
            return false;
        }
        value = m_array[m_front];
        m_mutex.unLock();
        return true;
    }
    // 获取队尾
    bool Back(T &value)
    {
        m_mutex.lock();
        if(m_size == 0)
        {
            m_mutex.unLock();
            return false;
        }
        value = m_array[m_back];
        m_mutex.unLock();
        return true;   
    }
    // 获取队列元素个数
    int Size()
    {
        int size = 0;
        m_mutex.lock();
        size = m_size;
        m_mutex.unLock();
        return size;
    }
    int MaxSize()
    {
        int maxSize = 0;
        m_mutex.lock();
        maxSize = m_max_size;
        m_mutex.unLock();
        return maxSize;
    }
    // 生产者线程
    bool Push(const T &item)
    {
        m_mutex.lock();
        if(m_size >= m_max_size)
        {
            m_cond.boradcast();
            m_mutex.unLock();
            return false;
        }
        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        m_size++;
        m_cond.boradcast();
        m_mutex.unLock();
        return true;
    }

    // 消费者线程
    bool Pop(T &item)
    {
        m_mutex.lock();
        while(m_size <= 0)
        {
            if(!m_cond.wait(m_mutex.get()))
            {
                m_mutex.unLock();
                return false;
            }

        }
        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unLock();
        return true;
    }

    // 带超时处理的消费者线程
    bool Pop(T &item,int ms_timeout)
    {
        struct timespec t = {0,0};
        struct timeval now = {0,0};
        gettimeofday(&now,NULL);
        m_mutex.lock();
        if(m_size <= 0)
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if(!m_cond.timewait(m_mutex.get(),t))
            {
                m_mutex.lock();
                return false;
            }
        }

        if (m_size <= 0)
        {
            m_mutex.unLock();
            return false;
        }

        m_front = (m_front + 1) % MaxSize;
        item = m_array[m_front];
        m_size--;
        m_mutex.unLock();
        return true;
    }
private:
    locker m_mutex;
    cond m_cond;

    T* m_array; 
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};

#endif