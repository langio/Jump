#ifndef	__YAC_THREAD_H_
#define __YAC_THREAD_H_

#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include "util/yac_ex.h"
#include "util/yac_monitor.h"

namespace util
{
/////////////////////////////////////////////////
/** 
 * @file yac_thread.h 
 * @brief  线程类(修改至ICE源码). 
 *  
 * @author jarodruan@tencent.com  
 */          
/////////////////////////////////////////////////

/**
 * @brief  线程控制异常类
 */
struct YAC_ThreadThreadControl_Exception : public YAC_Exception
{
    YAC_ThreadThreadControl_Exception(const string &buffer) : YAC_Exception(buffer){};
    YAC_ThreadThreadControl_Exception(const string &buffer, int err) : YAC_Exception(buffer, err){};
    ~YAC_ThreadThreadControl_Exception() throw() {};
};

/**
 * @brief  线程控制类
 */
class YAC_ThreadControl
{
public:

    /**
	 * @brief  构造, 表示当前运行的线程，
	 *  		join和detach在不能在该对象上调用
	*/
	   
    YAC_ThreadControl();

    /**
     * @return explicit
     */
    explicit YAC_ThreadControl(pthread_t);

    /**
     * @brief  等待当前线程结束, 不能在当前线程上调用
     */
    void join();

    /**
     * @brief  detach, 不能在当前线程上调用
     */
    void detach();

    /**
     * @brief  获取当前线程id.
     *
     * @return pthread_t当前线程id
     */
    pthread_t id() const;

    /**
	 * @brief  休息ms时间. 
	 *  
     * @param millsecond 休息的时间，具体ms数字
     */
    static void sleep(long millsecond);

    /**
     * @brief  交出当前线程控制权
     */
    static void yield();

private:

    pthread_t _thread;
};

/**
 *
 */
class YAC_Runable
{
public:
    virtual ~YAC_Runable(){};
    virtual void run() = 0;
};

/**
 * @brief 线程基类. 
 * 线程基类，所有自定义线程继承于该类，同时实现run接口即可, 
 *  
 * 可以通过YAC_ThreadContorl管理线程。
 */
class YAC_Thread : public YAC_Runable
{
public:

	/**
     * @brief  构造函数
	 */
	YAC_Thread();

	/**
     * @brief  析构函数
	 */
	virtual ~YAC_Thread(){};

	/**
     * @brief  线程运行
	 */
	YAC_ThreadControl start();

    /**
     * @brief  获取线程控制类.
     *
     * @return ThreadControl
     */
    YAC_ThreadControl getThreadControl() const;

    /**
     * @brief  线程是否存活.
     *
     * @return bool 存活返回true，否则返回false
     */
    bool isAlive() const;

	/**
     * @brief  获取线程id.
	 *
	 * @return pthread_t 线程id
	 */
	pthread_t id() { return _tid; }

protected:

	/**
	 * @brief  静态函数, 线程入口. 
	 *  
	 * @param pThread 线程对象
	 */
	static void threadEntry(YAC_Thread *pThread);

	/**
     * @brief  运行
	 */
    virtual void run() = 0;

protected:
    /**
     * 是否在运行
     */
    bool            _running;

    /**
     * 线程ID
     */
	pthread_t	    _tid;

    /**
     * 线程锁
     */
    YAC_ThreadLock   _lock;
};

}
#endif

