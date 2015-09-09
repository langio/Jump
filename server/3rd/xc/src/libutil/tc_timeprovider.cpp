#include "util/tc_timeprovider.h"

namespace xutil
{

XC_ThreadLock XC_TimeProvider::g_tl;
XC_TimeProviderPtr XC_TimeProvider::g_tp = NULL;

XC_TimeProvider* XC_TimeProvider::getInstance()
{
    if(!g_tp)
    {
        XC_ThreadLock::Lock lock(g_tl);

        if(!g_tp)
        {
            g_tp = new XC_TimeProvider();

            g_tp->start();
        }
    }
    return g_tp.get();
}

XC_TimeProvider::~XC_TimeProvider() 
{ 
    {
        XC_ThreadLock::Lock lock(g_tl);
        _terminate = true; 
        g_tl.notify(); 
    }

    getThreadControl().join();
}

void XC_TimeProvider::getNow(timeval *tv)  
{ 
    int idx = _buf_idx;
    *tv = _t[idx];

    if(_cpu_cycle != 0 && _use_tsc)     //cpu-cycle在两个interval周期后采集完成
    {    
        addTimeOffset(*tv,idx); 
    }
    else
    {
        ::gettimeofday(tv, NULL);
    }
}

int64_t XC_TimeProvider::getNowMs()
{
    struct timeval tv;
    getNow(&tv);
    return tv.tv_sec * (int64_t)1000 + tv.tv_usec/1000;
}

void XC_TimeProvider::run()
{
    while(!_terminate)
    {
        timeval& tt = _t[!_buf_idx];

        ::gettimeofday(&tt, NULL);

        setTsc(tt);  

        _buf_idx = !_buf_idx;

        XC_ThreadLock::Lock lock(g_tl);

        g_tl.timedWait(800); //修改800时 需对应修改addTimeOffset中offset判读值

    }
}

float XC_TimeProvider::cpuMHz()
{
    if(_cpu_cycle != 0)
        return 1.0/_cpu_cycle;

    return 0;
}

void XC_TimeProvider::setTsc(timeval& tt)
{
    uint32_t low    = 0;
    uint32_t high   = 0;
    rdtsc(low,high);
    uint64_t current_tsc    = ((uint64_t)high << 32) | low;

    uint64_t& last_tsc      = _tsc[!_buf_idx];
    timeval& last_tt        = _t[_buf_idx];

    if(_tsc[_buf_idx] == 0 || _tsc[!_buf_idx] == 0 )
    {
        _cpu_cycle      = 0;
        last_tsc        = current_tsc;
    }
    else
    {
        time_t sptime   = (tt.tv_sec -  last_tt.tv_sec)*1000*1000 + (tt.tv_usec - last_tt.tv_usec);  
        _cpu_cycle      = (float)sptime/(current_tsc - _tsc[_buf_idx]); //us 
        last_tsc        = current_tsc;
    } 
}

void XC_TimeProvider::addTimeOffset(timeval& tt,const int &idx)
{
    uint32_t low    = 0;
    uint32_t high   = 0;
    rdtsc(low,high);
    uint64_t current_tsc = ((uint64_t)high << 32) | low;
    int64_t t =  (int64_t)(current_tsc - _tsc[idx]);
    time_t offset =  (time_t)(t*_cpu_cycle);
    if(t < -1000 || offset > 1000000)//毫秒级别
    {
        //cerr<< "XC_TimeProvider add_time_offset error,correct it by use gettimeofday. current_tsc|"<<current_tsc<<"|last_tsc|"<<_tsc[idx]<<endl;
        _use_tsc = false;
        ::gettimeofday(&tt, NULL);
        return;
    }
    tt.tv_usec += offset;
    while (tt.tv_usec >= 1000000) { tt.tv_usec -= 1000000; tt.tv_sec++;} 
}


}


