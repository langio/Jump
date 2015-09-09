#ifndef __XC_THREAD_MUTEX_H
#define __XC_THREAD_MUTEX_H

#include "util/tc_lock.h"

namespace xutil
{
/////////////////////////////////////////////////
/** 
 * @file tc_thread_mutex.h 
 * @brief 线程锁互斥类(修改至ICE源码). 
 *  
 */
             
/////////////////////////////////////////////////
class XC_ThreadCond;

/**
 * @brief 线程互斥对象
 */
struct XC_ThreadMutex_Exception : public XC_Lock_Exception
{
    XC_ThreadMutex_Exception(const string &buffer) : XC_Lock_Exception(buffer){};
    XC_ThreadMutex_Exception(const string &buffer, int err) : XC_Lock_Exception(buffer, err){};
    ~XC_ThreadMutex_Exception() throw() {};
};

/**
* @brief 线程锁 . 
*  
* 不可重复加锁，即同一个线程不可以重复加锁 
*  
* 通常不直接使用，和XC_Monitor配合使用，即XC_ThreadLock; 
*/
class XC_ThreadMutex
{
public:

    XC_ThreadMutex();
    virtual ~XC_ThreadMutex();

    /**
     * @brief 加锁
     */
    void lock() const;

    /**
     * @brief 尝试锁
     * 
     * @return bool
     */
    bool tryLock() const;

    /**
     * @brief 解锁
     */
    void unlock() const;

    /**
	 * @brief 加锁后调用unlock是否会解锁， 
	 *  	  给XC_Monitor使用的 永远返回true
     * @return bool
     */
    bool willUnlock() const { return true;}

protected:

    // noncopyable
    XC_ThreadMutex(const XC_ThreadMutex&);
    void operator=(const XC_ThreadMutex&);

	/**
     * @brief 计数
	 */
    int count() const;

    /**
     * @brief 计数
	 */
    void count(int c) const;

    friend class XC_ThreadCond;

protected:
    mutable pthread_mutex_t _mutex;

};

/**
* @brief 线程锁类. 
*  
* 采用线程库实现
**/
class XC_ThreadRecMutex
{
public:

    /**
    * @brief 构造函数
    */
    XC_ThreadRecMutex();

    /**
    * @brief 析够函数
    */
    virtual ~XC_ThreadRecMutex();

    /**
	* @brief 锁, 调用pthread_mutex_lock. 
	*  
    * return : 返回pthread_mutex_lock的返回值
    */
    int lock() const;

    /**
	* @brief 解锁, pthread_mutex_unlock. 
	*  
    * return : 返回pthread_mutex_lock的返回值
    */
    int unlock() const;

    /**
	* @brief 尝试锁, 失败抛出异常. 
	*  
    * return : true, 成功锁; false 其他线程已经锁了
    */
    bool tryLock() const;

    /**
     * @brief 加锁后调用unlock是否会解锁, 给XC_Monitor使用的
     * 
     * @return bool
     */
    bool willUnlock() const;
protected:

	/**
     * @brief 友元类
     */
    friend class XC_ThreadCond;

	/**
     * @brief 计数
	 */
    int count() const;

    /**
     * @brief 计数
	 */
    void count(int c) const;

private:
    /**
    锁对象
    */
    mutable pthread_mutex_t _mutex;
	mutable int _count;
};

}

#endif

