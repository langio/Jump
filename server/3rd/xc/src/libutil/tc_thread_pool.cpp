#include "util/tc_thread_pool.h"
#include "util/tc_common.h"

#include <iostream>

namespace xutil
{

XC_ThreadPool::ThreadWorker::ThreadWorker(XC_ThreadPool *tpool)
: _tpool(tpool)
, _bTerminate(false)
{
}

void XC_ThreadPool::ThreadWorker::terminate()
{
    _bTerminate = true;
    _tpool->notifyT();
}

void XC_ThreadPool::ThreadWorker::run()
{
    //调用初始化部分
    XC_FunctorWrapperInterface *pst = _tpool->get();
    if(pst)
    {
        try
        {
            (*pst)();
        }
        catch ( ... )
        {
        }
        delete pst;
        pst = NULL;
    }

    //调用处理部分
    while (!_bTerminate)
    {
        XC_FunctorWrapperInterface *pfw = _tpool->get(this);
        if(pfw != NULL)
        {
            auto_ptr<XC_FunctorWrapperInterface> apfw(pfw);

            try
            {
                (*pfw)();
            }
            catch ( ... )
            {
            }

            _tpool->idle(this);
        }
    }

    //结束
    _tpool->exit();
}

//////////////////////////////////////////////////////////////
//
//

XC_ThreadPool::KeyInitialize XC_ThreadPool::g_key_initialize;
pthread_key_t XC_ThreadPool::g_key;

void XC_ThreadPool::destructor(void *p)
{
    ThreadData *ttd = (ThreadData*)p;
    if(ttd)
    {
        delete ttd;
    }
}

void XC_ThreadPool::exit()
{
    XC_ThreadPool::ThreadData *p = getThreadData();
    if(p)
    {
        delete p;
        int ret = pthread_setspecific(g_key, NULL);
        if(ret != 0)
        {
            throw XC_ThreadPool_Exception("[XC_ThreadPool::setThreadData] pthread_setspecific error", ret);
        }
    }

    _jobqueue.clear();
}

void XC_ThreadPool::setThreadData(XC_ThreadPool::ThreadData *p)
{
    XC_ThreadPool::ThreadData *pOld = getThreadData();
    if(pOld != NULL && pOld != p)
    {
        delete pOld;
    }

    int ret = pthread_setspecific(g_key, (void *)p);
    if(ret != 0)
    {
        throw XC_ThreadPool_Exception("[XC_ThreadPool::setThreadData] pthread_setspecific error", ret);
    }
}

XC_ThreadPool::ThreadData* XC_ThreadPool::getThreadData()
{
    return (ThreadData *)pthread_getspecific(g_key);
}

void XC_ThreadPool::setThreadData(pthread_key_t pkey, ThreadData *p)
{
    XC_ThreadPool::ThreadData *pOld = getThreadData(pkey);
    if(pOld != NULL && pOld != p)
    {
        delete pOld;
    }

    int ret = pthread_setspecific(pkey, (void *)p);
    if(ret != 0)
    {
        throw XC_ThreadPool_Exception("[XC_ThreadPool::setThreadData] pthread_setspecific error", ret);
    }
}

XC_ThreadPool::ThreadData* XC_ThreadPool::getThreadData(pthread_key_t pkey)
{
    return (ThreadData *)pthread_getspecific(pkey);
}

XC_ThreadPool::XC_ThreadPool()
{
}

XC_ThreadPool::~XC_ThreadPool()
{
    stop();
    clear();
}

void XC_ThreadPool::clear()
{
    std::vector<ThreadWorker*>::iterator it = _jobthread.begin();
    while(it != _jobthread.end())
    {
        delete (*it);
        ++it;
    }

    _jobthread.clear();
    _busthread.clear();
}

void XC_ThreadPool::init(size_t num)
{
    stop();

    Lock sync(*this);

    clear();

    for(size_t i = 0; i < num; i++)
    {
        _jobthread.push_back(new ThreadWorker(this));
    }
}

void XC_ThreadPool::stop()
{
    Lock sync(*this);

    std::vector<ThreadWorker*>::iterator it = _jobthread.begin();
    while(it != _jobthread.end())
    {
        if ((*it)->isAlive())
        {
            (*it)->terminate();
            (*it)->getThreadControl().join();
        }
        ++it;
    }
}

void XC_ThreadPool::start()
{
    Lock sync(*this);

    std::vector<ThreadWorker*>::iterator it = _jobthread.begin();
    while(it != _jobthread.end())
    {
        (*it)->start();
        ++it;
    }
}

bool XC_ThreadPool::finish()
{
    return _startqueue.empty() && _jobqueue.empty() && _busthread.empty();
}

bool XC_ThreadPool::waitForAllDone(int millsecond)
{
    Lock sync(_tmutex);

start1:
    //任务队列和繁忙线程都是空的
    if (finish())
    {
        return true;
    }

    //永远等待
    if(millsecond < 0)
    {
        _tmutex.timedWait(1000);
        goto start1;
    }

    int64_t iNow= XC_Common::now2ms();
    int m       = millsecond;
start2:

    bool b = _tmutex.timedWait(millsecond);
    //完成处理了
    if(finish())
    {
        return true;
    }

    if(!b)
    {
        return false;
    }

    millsecond = max((int64_t)0, m  - (XC_Common::now2ms() - iNow));
    goto start2;

    return false;
}

XC_FunctorWrapperInterface *XC_ThreadPool::get(ThreadWorker *ptw)
{
    XC_FunctorWrapperInterface *pFunctorWrapper = NULL;
    if(!_jobqueue.pop_front(pFunctorWrapper, 1000))
    {
        return NULL;
    }

    {
        Lock sync(_tmutex);
        _busthread.insert(ptw);
    }

    return pFunctorWrapper;
}

XC_FunctorWrapperInterface *XC_ThreadPool::get()
{
    XC_FunctorWrapperInterface *pFunctorWrapper = NULL;
    if(!_startqueue.pop_front(pFunctorWrapper))
    {
        return NULL;
    }

    return pFunctorWrapper;
}

void XC_ThreadPool::idle(ThreadWorker *ptw)
{
    Lock sync(_tmutex);
    _busthread.erase(ptw);

    //无繁忙线程, 通知等待在线程池结束的线程醒过来
    if(_busthread.empty())
    {
        _tmutex.notifyAll();
    }
}

void XC_ThreadPool::notifyT()
{
    _jobqueue.notifyT();
}



}
