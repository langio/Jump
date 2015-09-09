#ifndef _XC_THREAD_COND_H
#define _XC_THREAD_COND_H

#include <sys/time.h>
#include <cerrno>
#include <iostream>
#include "util/xc_ex.h"

namespace xutil
{
/////////////////////////////////////////////////
/**
 * @file xc_thread_cond.h 
 * @brief 条件变量类(修改至ICE源码). 
 *  
 */             
/////////////////////////////////////////////////
class XC_ThreadMutex;

/**
 *  @brief 线程条件异常类
 */
struct XC_ThreadCond_Exception : public XC_Exception
{
    XC_ThreadCond_Exception(const string &buffer) : XC_Exception(buffer){};
    XC_ThreadCond_Exception(const string &buffer, int err) : XC_Exception(buffer, err){};
    ~XC_ThreadCond_Exception() throw() {};
};

/**
 *  @brief 线程信号条件类, 所有锁可以在上面等待信号发生
 *  
 *  和XC_ThreadMutex、XC_ThreadRecMutex配合使用,
 *  
 *  通常不直接使用，而是使用XC_ThreadLock/XC_ThreadRecLock;
 */
class XC_ThreadCond
{
public:

    /**
     *  @brief 构造函数
     */
    XC_ThreadCond();

    /**
     *  @brief 析构函数
     */
    ~XC_ThreadCond();

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
            throw XC_ThreadCond_Exception("[XC_ThreadCond::wait] pthread_cond_wait error", errno);
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
                throw XC_ThreadCond_Exception("[XC_ThreadCond::timedWait] pthread_cond_timedwait error", errno);
            }

            return false;
        }
        return true;
    }

protected:
    // Not implemented; prevents accidental use.
    XC_ThreadCond(const XC_ThreadCond&);
    XC_ThreadCond& operator=(const XC_ThreadCond&);

private:

    /**
     * 线程条件
     */
    mutable pthread_cond_t _cond;

};

}

#endif

