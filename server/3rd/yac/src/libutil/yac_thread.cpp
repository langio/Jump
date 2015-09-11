#include "util/yac_thread.h"
#include <cerrno>

namespace util
{

YAC_ThreadControl::YAC_ThreadControl(pthread_t thread) : _thread(thread)
{
}

YAC_ThreadControl::YAC_ThreadControl() : _thread(pthread_self())
{
}

void YAC_ThreadControl::join()
{
    if(pthread_self() == _thread)
    {
        throw YAC_ThreadThreadControl_Exception("[YAC_ThreadControl::join] can't be called in the same thread");
    }

    void* ignore = 0;
    int rc = pthread_join(_thread, &ignore);
    if(rc != 0)
    {
        throw YAC_ThreadThreadControl_Exception("[YAC_ThreadControl::join] pthread_join error ", rc);
    }
}

void YAC_ThreadControl::detach()
{
    if(pthread_self() == _thread)
    {
        throw YAC_ThreadThreadControl_Exception("[YAC_ThreadControl::join] can't be called in the same thread");
    }

    int rc = pthread_detach(_thread);
    if(rc != 0)
    {
        throw YAC_ThreadThreadControl_Exception("[YAC_ThreadControl::join] pthread_join error", rc);
    }
}

pthread_t YAC_ThreadControl::id() const
{
    return _thread;
}

void YAC_ThreadControl::sleep(long millsecond)
{
    struct timespec ts;
    ts.tv_sec = millsecond / 1000;
    ts.tv_nsec = millsecond % 1000;
    nanosleep(&ts, 0);
}

void YAC_ThreadControl::yield()
{
    sched_yield();
}

YAC_Thread::YAC_Thread() : _running(false)
{
}

void YAC_Thread::threadEntry(YAC_Thread *pThread)
{
    pThread->_running = true;

    {
        YAC_ThreadLock::Lock sync(pThread->_lock);
        pThread->_lock.notifyAll();
    }

    try
    {
        pThread->run();
    }
    catch(...)
    {
        pThread->_running = false;
        throw;
    }
    pThread->_running = false;
}

YAC_ThreadControl YAC_Thread::start()
{
    YAC_ThreadLock::Lock sync(_lock);

    if(_running)
    {
        throw YAC_ThreadThreadControl_Exception("[YAC_Thread::start] thread has start");
    }

    int ret = pthread_create(&_tid,
                   0,
                   (void *(*)(void *))&threadEntry,
                   (void *)this);

    if(ret != 0)
    {
        throw YAC_ThreadThreadControl_Exception("[YAC_Thread::start] thread start error", ret);
    }

    _lock.wait();

    return YAC_ThreadControl(_tid);
}

YAC_ThreadControl YAC_Thread::getThreadControl() const
{
    return YAC_ThreadControl(_tid);
}

bool YAC_Thread::isAlive() const
{
    return _running;
}

}

