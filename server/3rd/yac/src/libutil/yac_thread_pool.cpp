#include "util/yac_thread_pool.h"
#include "util/yac_common.h"

#include <iostream>

namespace util
{

YAC_ThreadPool::ThreadWorker::ThreadWorker(YAC_ThreadPool *tpool)
: _tpool(tpool)
, _bTerminate(false)
{
}

void YAC_ThreadPool::ThreadWorker::terminate()
{
    _bTerminate = true;
    _tpool->notifyT();
}

void YAC_ThreadPool::ThreadWorker::run()
{
    //调用初始化部分
    YAC_FunctorWrapperInterface *pst = _tpool->get();
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
        YAC_FunctorWrapperInterface *pfw = _tpool->get(this);
        if(pfw != NULL)
        {
            auto_ptr<YAC_FunctorWrapperInterface> apfw(pfw);

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

YAC_ThreadPool::KeyInitialize YAC_ThreadPool::g_key_initialize;
pthread_key_t YAC_ThreadPool::g_key;

void YAC_ThreadPool::destructor(void *p)
{
    ThreadData *ttd = (ThreadData*)p;
    if(ttd)
    {
        delete ttd;
    }
}

void YAC_ThreadPool::exit()
{
    YAC_ThreadPool::ThreadData *p = getThreadData();
    if(p)
    {
        delete p;
        int ret = pthread_setspecific(g_key, NULL);
        if(ret != 0)
        {
            throw YAC_ThreadPool_Exception("[YAC_ThreadPool::setThreadData] pthread_setspecific error", ret);
        }
    }

    _jobqueue.clear();
}

void YAC_ThreadPool::setThreadData(YAC_ThreadPool::ThreadData *p)
{
    YAC_ThreadPool::ThreadData *pOld = getThreadData();
    if(pOld != NULL && pOld != p)
    {
        delete pOld;
    }

    int ret = pthread_setspecific(g_key, (void *)p);
    if(ret != 0)
    {
        throw YAC_ThreadPool_Exception("[YAC_ThreadPool::setThreadData] pthread_setspecific error", ret);
    }
}

YAC_ThreadPool::ThreadData* YAC_ThreadPool::getThreadData()
{
    return (ThreadData *)pthread_getspecific(g_key);
}

void YAC_ThreadPool::setThreadData(pthread_key_t pkey, ThreadData *p)
{
    YAC_ThreadPool::ThreadData *pOld = getThreadData(pkey);
    if(pOld != NULL && pOld != p)
    {
        delete pOld;
    }

    int ret = pthread_setspecific(pkey, (void *)p);
    if(ret != 0)
    {
        throw YAC_ThreadPool_Exception("[YAC_ThreadPool::setThreadData] pthread_setspecific error", ret);
    }
}

YAC_ThreadPool::ThreadData* YAC_ThreadPool::getThreadData(pthread_key_t pkey)
{
    return (ThreadData *)pthread_getspecific(pkey);
}

YAC_ThreadPool::YAC_ThreadPool()
{
}

YAC_ThreadPool::~YAC_ThreadPool()
{
    stop();
    clear();
}

void YAC_ThreadPool::clear()
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

void YAC_ThreadPool::init(size_t num)
{
    stop();

    Lock sync(*this);

    clear();

    for(size_t i = 0; i < num; i++)
    {
        _jobthread.push_back(new ThreadWorker(this));
    }
}

void YAC_ThreadPool::stop()
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

void YAC_ThreadPool::start()
{
    Lock sync(*this);

    std::vector<ThreadWorker*>::iterator it = _jobthread.begin();
    while(it != _jobthread.end())
    {
        (*it)->start();
        ++it;
    }
}

bool YAC_ThreadPool::finish()
{
    return _startqueue.empty() && _jobqueue.empty() && _busthread.empty();
}

bool YAC_ThreadPool::waitForAllDone(int millsecond)
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

    int64_t iNow= YAC_Common::now2ms();
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

    millsecond = max((int64_t)0, m  - (YAC_Common::now2ms() - iNow));
    goto start2;

    return false;
}

YAC_FunctorWrapperInterface *YAC_ThreadPool::get(ThreadWorker *ptw)
{
    YAC_FunctorWrapperInterface *pFunctorWrapper = NULL;
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

YAC_FunctorWrapperInterface *YAC_ThreadPool::get()
{
    YAC_FunctorWrapperInterface *pFunctorWrapper = NULL;
    if(!_startqueue.pop_front(pFunctorWrapper))
    {
        return NULL;
    }

    return pFunctorWrapper;
}

void YAC_ThreadPool::idle(ThreadWorker *ptw)
{
    Lock sync(_tmutex);
    _busthread.erase(ptw);

    //无繁忙线程, 通知等待在线程池结束的线程醒过来
    if(_busthread.empty())
    {
        _tmutex.notifyAll();
    }
}

void YAC_ThreadPool::notifyT()
{
    _jobqueue.notifyT();
}



}
