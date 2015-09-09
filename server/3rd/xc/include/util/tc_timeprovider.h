#ifndef __XC_TIME_PROVIDER_H_
#define __XC_TIME_PROVIDER_H_

#include <string>
#include <string.h>
#include "util/tc_monitor.h"
#include "util/tc_thread.h"
#include "util/tc_autoptr.h"

#define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

namespace xutil
{
/////////////////////////////////////////////////
/**
 * @file tc_timeprovider.h
 * @brief 秒级、微妙级时间提供类. 
 *  
 */                    
/////////////////////////////////////////////////
class XC_TimeProvider;

typedef XC_AutoPtr<XC_TimeProvider> XC_TimeProviderPtr;

/**
 * @brief 提供秒级别的时间
 */
class XC_TimeProvider : public XC_Thread, public XC_HandleBase
{
public:

    /**
	 * @brief 获取实例. 
	 *  
     * @return TimeProvider&
     */
    static XC_TimeProvider* getInstance();

    /**
     * @brief 构造函数
     */
    XC_TimeProvider() : _terminate(false),_use_tsc(true),_cpu_cycle(0),_buf_idx(0)
    {
        memset(_t,0,sizeof(_t));
        memset(_tsc,0,sizeof(_tsc));

        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        _t[0] = tv;
        _t[1] = tv;
    }

    /**
     * @brief 析构，停止线程
     */
    ~XC_TimeProvider(); 

    /**
     * @brief 获取时间.
     *
     * @return time_t 当前时间
     */
    time_t getNow()     {  return _t[_buf_idx].tv_sec; }

    /**
     * @brief 获取时间.
     *
	 * @para timeval 
     * @return void 
     */
    void getNow(timeval * tv);

    /**
     * @brief 获取ms时间.
     *
	 * @para timeval 
     * @return void 
     */
    int64_t getNowMs();
    
    /**
     * @brief 获取cpu主频.
     *  
     * @return float cpu主频
     */  

    float cpuMHz();

    /**
     * @brief 运行
     */
protected:

    virtual void run();

    static XC_ThreadLock        g_tl;

    static XC_TimeProviderPtr   g_tp;

private:
    void setTsc(timeval& tt);

    void addTimeOffset(timeval& tt,const int &idx);

protected:

    bool    _terminate;

    bool    _use_tsc;

private:
    float           _cpu_cycle; 

    volatile int    _buf_idx;

    timeval         _t[2];

    uint64_t        _tsc[2];  
};

}

#endif


