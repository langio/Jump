#include "util/yac_http_async.h"
#include "util/yac_common.h"

namespace util
{

YAC_HttpAsync::AsyncRequest::AsyncRequest(YAC_HttpRequest &stHttpRequest, YAC_HttpAsync::RequestCallbackPtr &callbackPtr) 
    : _pHttpAsync(NULL), _callbackPtr(callbackPtr)
{
	_bindAddrSet = false;

    _sReq = stHttpRequest.encode();
    
    stHttpRequest.getHostPort(_sHost, _iPort);
}

YAC_HttpAsync::AsyncRequest::~AsyncRequest()
{
    doClose();
}

void YAC_HttpAsync::AsyncRequest::doClose()
{
    if(_fd.isValid())
    {
        if(_callbackPtr) _callbackPtr->onClose();
        if(_pHttpAsync) _pHttpAsync->erase(_iUniqId);
        _fd.close();
    }
}

void YAC_HttpAsync::AsyncRequest::setBindAddr(const struct sockaddr* addr)
{
	memcpy(&_bindAddr, addr, sizeof(struct sockaddr));

	_bindAddrSet = true;
}

int YAC_HttpAsync::AsyncRequest::doConnect()
{
    _fd.createSocket();
    _fd.setblock();

    //不产生TimeWait状态
    _fd.setNoCloseWait();
    
    try
    {
		if(_bindAddrSet)
		{
			_fd.bind(&_bindAddr,sizeof(_bindAddr));
		}

        int ret = _fd.connectNoThrow(_sHost, _iPort);

        if(ret < 0 && errno != EINPROGRESS)
        {
            _fd.close();
            return ret;
        }
    }
    catch(exception &ex)
    {
        _fd.close();
        return -1;
    }

    return 0;
}

int YAC_HttpAsync::AsyncRequest::doConnect(struct sockaddr* proxyAddr)
{
    _fd.createSocket();
    _fd.setblock();

    //不产生TimeWait状态
    _fd.setNoCloseWait();

	try
	{
		if(_bindAddrSet)
		{
			_fd.bind(&_bindAddr,sizeof(_bindAddr));
		}
		
	    int ret = _fd.connectNoThrow(proxyAddr);
	    if(ret < 0 && errno != EINPROGRESS)
	    {
	        _fd.close();
	        return ret;
	    }
	}
    catch(exception &ex)
    {
        _fd.close();
        return -1;
    }

    return 0;
}

int YAC_HttpAsync::AsyncRequest::recv(void* buf, uint32_t len, uint32_t flag)
{
    int ret = ::recv(_fd.getfd(), buf, len, flag);

    if (ret == 0)
    {
        return 0;
    }
    else if(ret < 0 && errno == EAGAIN)
    {
        return -1;
    }
    else if(ret < 0)
    {
        //其他网络异常
        return -2;
    }

    //正常发送的数据
    return ret;
}
        
int YAC_HttpAsync::AsyncRequest::send(const void* buf, uint32_t len, uint32_t flag)
{
    int ret = ::send(_fd.getfd(), buf, len, flag);

    if (ret < 0 && errno == EAGAIN)
    {
        return -1;
    }
    else if (ret < 0)
    {
        return -2;
    }
    return ret;
}        

void YAC_HttpAsync::AsyncRequest::timeout()
{
    try
    {
        doClose();

        if(_callbackPtr) _callbackPtr->onTimeout();
    }
    catch(exception &ex)
    {
        if(_callbackPtr) _callbackPtr->onException(ex.what());
    }
    catch(...)
    {
        if(_callbackPtr) _callbackPtr->onException("unknown error.");
    }
}

void YAC_HttpAsync::AsyncRequest::doException()
{
    string err("unknown error.");

    if(_fd.isValid())
    {
        int error;
        socklen_t len = sizeof(error);
        _fd.getSockOpt(SO_ERROR, (void*)&error, len, SOL_SOCKET);
        err = strerror(error);

        doClose();
    }

    try { if(_callbackPtr) _callbackPtr->onException(err); } catch(...) { }
}

void YAC_HttpAsync::AsyncRequest::doRequest()
{
    if(!_fd.isValid()) return;

    int ret = -1;

    do
    {
        ret = -1;
        
        if (!_sReq.empty())
        {
            if ((ret = this->send(_sReq.c_str(), _sReq.length(), 0)) > 0)
            {
                _sReq = _sReq.substr(ret);
            }
        }
    }while(ret > 0);

	//网络异常
	if(ret == -2)
	{
		doException();
	}
}

void YAC_HttpAsync::AsyncRequest::doReceive()
{
    if(!_fd.isValid()) return;

    int recv = 0;

    char buff[8192] = {0};

    do
    {
        if ((recv = this->recv(buff, sizeof(buff), 0)) > 0)
        {
            _sRsp.append(buff, recv);
        }
    }
    while (recv > 0);

    if(recv == -2)
    {
        doException();
    }
    else
    {
        try
        {
            //增量decode
            bool ret    = _stHttpResp.incrementDecode(_sRsp);

            //有头部数据了
            if(_callbackPtr && !_stHttpResp.getHeaders().empty())
            {
                bool bContinue = _callbackPtr->onReceive(_stHttpResp);
                if(!bContinue)
                {
                    doClose();
                    return;
                }
            }

            //服务器关闭了连接
            bool bClose = (recv == 0);

            //如果远程关闭连接或者增量decode完毕
            if(bClose || ret)
            {
                doClose();

                if(_callbackPtr) _callbackPtr->onResponse(bClose, _stHttpResp);
            }
        }
        catch(exception &ex)
        {
            if(_callbackPtr) _callbackPtr->onException(ex.what());
        }
        catch(...)
        {
            if(_callbackPtr) _callbackPtr->onException("unknown error.");
        }
    }
}

///////////////////////////////////////////////////////////////////////////

YAC_HttpAsync::YAC_HttpAsync() : _terminate(false)
{
	_bindAddrSet=false;

    _data = new http_queue_type(10000);

    _epoller.create(1024);
}

YAC_HttpAsync::~YAC_HttpAsync()
{
    terminate();

    delete _data;
/*
    for(size_t i = 0; i < _npool.size(); i++)
    {
        delete _npool[i];
    }
    _npool.clear();
*/
}

void YAC_HttpAsync::start(int iThreadNum)
{
//    if(_npool.size() > 0)
//        throw YAC_HttpAsync_Exception("[YAC_HttpAsync::start] thread has started.");

    _tpool.init(1);
    _tpool.start();
/*
    if(iThreadNum <= 0) iThreadNum = 1;

    for(int i = 0; i < iThreadNum; i++)
    {
        _npool.push_back(new YAC_ThreadPool());
        _npool.back()->init(1);
        _npool.back()->start();
    }
*/
    YAC_Functor<void> cmd(this, &YAC_HttpAsync::run);
    YAC_Functor<void>::wrapper_type wt(cmd);

    _tpool.exec(wt);
}

void YAC_HttpAsync::waitForAllDone(int millsecond)
{
    time_t now = YAC_TimeProvider::getInstance()->getNow();

    while(_data->size() > 0)
    {
        if(millsecond < 0)
        {
            YAC_ThreadLock::Lock lock(*this);
            timedWait(100);
            continue;
        }

        {
            //等待100ms
            YAC_ThreadLock::Lock lock(*this);
            timedWait(100);
        }

        if((YAC_TimeProvider::getInstance()->getNow() - now) >= (millsecond/1000))
            break;
    }

    terminate();
}

void YAC_HttpAsync::erase(uint32_t uniqId) 
{ 
    _data->erase(uniqId); 

    YAC_ThreadLock::Lock lock(*this);
    notify();
}

void YAC_HttpAsync::terminate()
{
    _terminate = true;
/*
    for(size_t i = 0; i < _npool.size(); i++)
    {
        _npool[i]->waitForAllDone();
    }
*/
    _tpool.waitForAllDone();
}

void YAC_HttpAsync::timeout(AsyncRequestPtr& ptr) 
{ 
    ptr->timeout(); 
}

int YAC_HttpAsync::doAsyncRequest(YAC_HttpRequest &stHttpRequest, RequestCallbackPtr &callbackPtr, bool bUseProxy,struct sockaddr* addr)
{
	int ret;

    AsyncRequestPtr req = new AsyncRequest(stHttpRequest, callbackPtr);

	if(_bindAddrSet)
	{
		req->setBindAddr(&_bindAddr);
	}

    //发起异步连接请求
	if(bUseProxy)
	{
		ret = req->doConnect(&_proxyAddr);
	}
	else if(NULL != addr)
	{
		ret = req->doConnect(addr);
	}
	else
	{
		ret = req->doConnect();
	}

    if(ret < 0) return -1;

    uint32_t uniqId = _data->generateId();

    req->setUniqId(uniqId);

    req->setHttpAsync(this);

    _data->push(req, uniqId);

    _epoller.add(req->getfd(), uniqId, EPOLLIN | EPOLLOUT);

    return 0;
}

int YAC_HttpAsync::setBindAddr(const char* sBindAddr)
{
	bzero(&_bindAddr,sizeof(_bindAddr));

	struct sockaddr_in* p = (struct sockaddr_in *)&_bindAddr;
	
	try
	{
		YAC_Socket::parseAddr(sBindAddr, p->sin_addr);
	}
    catch(exception &ex)
    {
        return -1;
    }	
	
	p->sin_family = AF_INET;
	p->sin_port   = htons(0);

	_bindAddrSet  = true;
	
	return 0;
}


int YAC_HttpAsync::setProxyAddr(const char* sProxyAddr)
{
    vector<string> v = YAC_Common::sepstr<string>(sProxyAddr, ":");

    if(v.size() < 2)
        return -1;

	return setProxyAddr(v[0].c_str(), YAC_Common::strto<uint16_t>(v[1]));
}

int YAC_HttpAsync::setProxyAddr(const char* sHost, uint16_t iPort)
{
	bzero(&_proxyAddr,sizeof(_proxyAddr));

	struct sockaddr_in *p = (struct sockaddr_in *)&_proxyAddr;
	
	try
	{
		YAC_Socket::parseAddr(sHost, p->sin_addr);
	}
    catch(exception &ex)
    {
        return -1;
    }	
	
	p->sin_family = AF_INET;
	p->sin_port   = htons(iPort);

	return 0;
}

void YAC_HttpAsync::setProxyAddr(const struct sockaddr* addr)
{
	memcpy(&_proxyAddr, addr, sizeof(struct sockaddr));
}

void YAC_HttpAsync::process(AsyncRequestPtr &p, int events)
{
    if (events & (EPOLLERR | EPOLLHUP))
    {
        p->doException();
        return;
    }

    if(events & EPOLLIN)
    {
        p->doReceive();
    }

    if(events & EPOLLOUT)
    {
        p->doRequest();
    } 
}

void YAC_HttpAsync::run() 
{
    YAC_TimeoutQueue<AsyncRequestPtr>::data_functor df(&YAC_HttpAsync::timeout);
    
    async_process_type apt(&YAC_HttpAsync::process);

    int64_t lastDealTimeout = 0;

    while(!_terminate)
    {
        try
        {
            struct timeval tv = {0, 0};
            YAC_TimeProvider::getInstance()->getNow(&tv);

            int64_t now = tv.tv_sec * 1000 + tv.tv_usec % 1000;
            if (lastDealTimeout + 500 < now)
            {
                lastDealTimeout = now;
                _data->timeout(df);
            }

            int num = _epoller.wait(100);

            for (int i = 0; i < num; ++i)
            {
                epoll_event ev = _epoller.get(i);

                uint32_t uniqId = (uint32_t)ev.data.u64;

                AsyncRequestPtr p = _data->get(uniqId, false);
                
                if(!p) continue;

//                async_process_type::wrapper_type w(apt, p, ev.events);

//                _npool[uniqId%_npool.size()]->exec(w);
                process(p, ev.events);
            }
        }
        catch(exception &ex)
        {
            cerr << "[YAC_HttpAsync::run] error:" << ex.what() << endl;
        }
    }
}

}


