#include "util/xc_thread_cond.h"
#include "util/xc_timeprovider.h"
#include <string.h>
#include <cassert>
#include <iostream>

using namespace std;


namespace xutil
{

XC_ThreadCond::XC_ThreadCond()
{
    int rc;

    pthread_condattr_t attr;

    rc = pthread_condattr_init(&attr);
    if(rc != 0)
    {
        throw XC_ThreadCond_Exception("[XC_ThreadCond::XC_ThreadCond] pthread_condattr_init error", errno);
    }

    rc = pthread_cond_init(&_cond, &attr);
    if(rc != 0)
    {
        throw XC_ThreadCond_Exception("[XC_ThreadCond::XC_ThreadCond] pthread_cond_init error", errno);
    }

    rc = pthread_condattr_destroy(&attr);
    if(rc != 0)
    {
        throw XC_ThreadCond_Exception("[XC_ThreadCond::XC_ThreadCond] pthread_condattr_destroy error", errno);
    }
}

XC_ThreadCond::~XC_ThreadCond()
{
    int rc = 0;
    rc = pthread_cond_destroy(&_cond);
    if(rc != 0)
    {
        cerr << "[XC_ThreadCond::~XC_ThreadCond] pthread_cond_destroy error:" << string(strerror(rc)) << endl;
    }
//    assert(rc == 0);
}

void XC_ThreadCond::signal()
{
    int rc = pthread_cond_signal(&_cond);
    if(rc != 0)
    {
        throw XC_ThreadCond_Exception("[XC_ThreadCond::signal] pthread_cond_signal error", errno);
    }
}

void XC_ThreadCond::broadcast()
{
    int rc = pthread_cond_broadcast(&_cond);
    if(rc != 0)
    {
        throw XC_ThreadCond_Exception("[XC_ThreadCond::broadcast] pthread_cond_broadcast error", errno);
    }
}

timespec XC_ThreadCond::abstime( int millsecond) const
{
    struct timeval tv;

    //gettimeofday(&tv, 0);
    XC_TimeProvider::getInstance()->getNow(&tv);

    int64_t it  = tv.tv_sec * (int64_t)1000000 + tv.tv_usec + (int64_t)millsecond * 1000;

    tv.tv_sec   = it / (int64_t)1000000;
    tv.tv_usec  = it % (int64_t)1000000;

    timespec ts;
    ts.tv_sec   = tv.tv_sec;
    ts.tv_nsec  = tv.tv_usec * 1000; 
      
    return ts; 
}


}

