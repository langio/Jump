#include "util/xc_thread.h"
#include <cerrno>

namespace xutil
{

XC_ThreadControl::XC_ThreadControl(pthread_t thread) : _thread(thread)
{
}

XC_ThreadControl::XC_ThreadControl() : _thread(pthread_self())
{
}

void XC_ThreadControl::join()
{
    if(pthread_self() == _thread)
    {
        throw XC_ThreadThreadControl_Exception("[XC_ThreadControl::join] can't be called in the same thread");
    }

    void* ignore = 0;
    int rc = pthread_join(_thread, &ignore);
    if(rc != 0)
    {
        throw XC_ThreadThreadControl_Exception("[XC_ThreadControl::join] pthread_join error ", rc);
    }
}

void XC_ThreadControl::detach()
{
    if(pthread_self() == _thread)
    {
        throw XC_ThreadThreadControl_Exception("[XC_ThreadControl::join] can't be called in the same thread");
    }

    int rc = pthread_detach(_thread);
    if(rc != 0)
    {
        throw XC_ThreadThreadControl_Exception("[XC_ThreadControl::join] pthread_join error", rc);
    }
}

pthread_t XC_ThreadControl::id() const
{
    return _thread;
}

void XC_ThreadControl::sleep(long millsecond)
{
    struct timespec ts;
    ts.tv_sec = millsecond / 1000;
    ts.tv_nsec = millsecond % 1000;
    nanosleep(&ts, 0);
}

void XC_ThreadControl::yield()
{
    sched_yield();
}

XC_Thread::XC_Thread() : _running(false)
{
}

void XC_Thread::threadEntry(XC_Thread *pThread)
{
    pThread->_running = true;

    {
        XC_ThreadLock::Lock sync(pThread->_lock);
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

XC_ThreadControl XC_Thread::start()
{
    XC_ThreadLock::Lock sync(_lock);

    if(_running)
    {
        throw XC_ThreadThreadControl_Exception("[XC_Thread::start] thread has start");
    }

    int ret = pthread_create(&_tid,
                   0,
                   (void *(*)(void *))&threadEntry,
                   (void *)this);

    if(ret != 0)
    {
        throw XC_ThreadThreadControl_Exception("[XC_Thread::start] thread start error", ret);
    }

    _lock.wait();

    return XC_ThreadControl(_tid);
}

XC_ThreadControl XC_Thread::getThreadControl() const
{
    return XC_ThreadControl(_tid);
}

bool XC_Thread::isAlive() const
{
    return _running;
}

}

