#ifndef _YAC_THREAD_COND_H
#define _YAC_THREAD_COND_H

#include <sys/time.h>
#include <cerrno>
#include <iostream>
#include "util/yac_ex.h"

namespace util
{
/////////////////////////////////////////////////
/**
 * @file yac_thread_cond.h 
 * @brief 条件变量类(修改至ICE源码). 
 *  
 * @author  jarodruan@tencent.com,skingfan@tencent.com   
 */             
/////////////////////////////////////////////////
class YAC_ThreadMutex;

/**
 *  @brief 线程条件异常类
 */
struct YAC_ThreadCond_Exception : public YAC_Exception
{
    YAC_ThreadCond_Exception(const string &buffer) : YAC_Exception(buffer){};
    YAC_ThreadCond_Exception(const string &buffer, int err) : YAC_Exception(buffer, err){};
    ~YAC_ThreadCond_Exception() throw() {};
};

/**
 *  @brief 线程信号条件类, 所有锁可以在上面等待信号发生
 *  
 *  和YAC_ThreadMutex、YAC_ThreadRecMutex配合使用,
 *  
 *  通常不直接使用，而是使用YAC_ThreadLock/YAC_ThreadRecLock;
 */
class YAC_ThreadCond
{
public:

    /**
     *  @brief 构造函数
     */
    YAC_ThreadCond();

    /**
     *  @brief 析构函数
     */
    ~YAC_ThreadCond();

    /**
     *  @brief 发送信号, 等待在该条件上的一个线程会醒
     */
    void signal();

    /**
     *  @brief 等待在该条件的所有线程都会醒
     */
    void broadcast();

    /**
     *  @brief 获取绝对等待时间
     */
    timespec abstime(int millsecond) const;

    /**
	 *  @brief 无限制等待.
	 *  
     * @param M
     */
    template<typename Mutex>
    void wait(const Mutex& mutex) const
    {
        int c = mutex.count();
        int rc = pthread_cond_wait(&_cond, &mutex._mutex);
        mutex.count(c);
        if(rc != 0)
        {
            throw YAC_ThreadCond_Exception("[YAC_ThreadCond::wait] pthread_cond_wait error", errno);
        }
    }

    /**
	 * @brief 等待时间. 
	 *  
	 * @param M 
     * @return bool, false表示超时, true:表示有事件来了
     */
    template<typename Mutex>
    bool timedWait(const Mutex& mutex, int millsecond) const
    {
        int c = mutex.count();

        timespec ts = abstime(millsecond);

        int rc = pthread_cond_timedwait(&_cond, &mutex._mutex, &ts);

        mutex.count(c);

        if(rc != 0)
        {
            if(rc != ETIMEDOUT)
            {
                throw YAC_ThreadCond_Exception("[YAC_ThreadCond::timedWait] pthread_cond_timedwait error", errno);
            }

            return false;
        }
        return true;
    }

protected:
    // Not implemented; prevents accidental use.
    YAC_ThreadCond(const YAC_ThreadCond&);
    YAC_ThreadCond& operator=(const YAC_ThreadCond&);

private:

    /**
     * 线程条件
     */
    mutable pthread_cond_t _cond;

};

}

#endif

