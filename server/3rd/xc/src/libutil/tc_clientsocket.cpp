#include <cerrno>
#include <iostream>
#include "util/tc_clientsocket.h"
#include "util/tc_epoller.h"
#include "util/tc_common.h"

namespace taf
{
TC_Endpoint::TC_Endpoint()
{
    _host = "0.0.0.0";
    _port = 0;
    _timeout = 3000;
    _istcp = true;
    _grid = 0;
    _qos = 0;
}

void TC_Endpoint::parse(const string &str)
{
	_grid = 0;
    _qos = 0;

    const string delim = " \t\n\r";

    string::size_type beg;
    string::size_type end = 0;

    beg = str.find_first_not_of(delim, end);
    if(beg == string::npos)
    {
        throw TC_EndpointParse_Exception("TC_Endpoint::parse error : " + str);
    }

    end = str.find_first_of(delim, beg);
    if(end == string::npos)
    {
        end = str.length();
    }

    string desc = str.substr(beg, end - beg);
    if(desc == "tcp")
    {
		_istcp = true;
    }
	else if(desc == "udp")
	{
		_istcp = false;
	}
	else
	{
		throw TC_EndpointParse_Exception("TC_Endpoint::parse tcp or udp error : " + str);
	}

    desc = str.substr(end);
	end  = 0;
    while(true)
    {
    	beg = desc.find_first_not_of(delim, end);
    	if(beg == string::npos)
    	{
    	    break;
    	}

    	end = desc.find_first_of(delim, beg);
    	if(end == string::npos)
    	{
    	    end = desc.length();
    	}

    	string option = desc.substr(beg, end - beg);
    	if(option.length() != 2 || option[0] != '-')
    	{
    	    throw TC_EndpointParse_Exception("TC_Endpoint::parse error : " + str);
    	}

    	string argument;
    	string::size_type argumentBeg = desc.find_first_not_of(delim, end);
    	if(argumentBeg != string::npos && desc[argumentBeg] != '-')
    	{
    	    beg = argumentBeg;
    	    end = desc.find_first_of(delim, beg);
    	    if(end == string::npos)
    	    {
                end = desc.length();
    	    }
    	    argument = desc.substr(beg, end - beg);
    	}

    	switch(option[1])
    	{
    	    case 'h':
    	    {
        		if(argument.empty())
        		{
                    throw TC_EndpointParse_Exception("TC_Endpoint::parse -h error : " + str);
        		}
        		const_cast<string&>(_host) = argument;
        		break;
    	    }
    	    case 'p':
    	    {
        		istringstream p(argument);
        		if(!(p >> const_cast<int&>(_port)) || !p.eof() || _port < 0 || _port > 65535)
        		{
                    throw TC_EndpointParse_Exception("TC_Endpoint::parse -p error : " + str);
        		}
        		break;
    	    }
    	    case 't':
    	    {
        		istringstream t(argument);
        		if(!(t >> const_cast<int&>(_timeout)) || !t.eof())
        		{
                    throw TC_EndpointParse_Exception("TC_Endpoint::parse -t error : " + str);
        		}
        		break;
    	    }
            case 'g':
            {
        		istringstream t(argument);
        		if(!(t >> const_cast<int&>(_grid)) || !t.eof())
        		{
                    throw TC_EndpointParse_Exception("TC_Endpoint::parse -g error : " + str);
        		}
        		break;
            }
            case 'q':
            {
        		istringstream t(argument);
        		if(!(t >> const_cast<int&>(_qos)) || !t.eof())
        		{
                    throw TC_EndpointParse_Exception("TC_Endpoint::parse -q error : " + str);
        		}
        		break;
            }
            default:
    	    {
                ///throw TC_EndpointParse_Exception("TC_Endpoint::parse error : " + str);
    	    }
    	}
    }

    if(_host.empty())
    {
        throw TC_EndpointParse_Exception("TC_Endpoint::parse error : host must not be empty: " + str);
    }
    else if(_host == "*")
    {
        const_cast<string&>(_host) = "0.0.0.0";
    }
}

/*************************************TC_TCPClient**************************************/

#define LEN_MAXRECV 8196

int TC_TCPClient::checkSocket()
{
    if(!_socket.isValid())
    {
        try
        {
            if(_port == 0)
            {
                _socket.createSocket(SOCK_STREAM, AF_LOCAL);
            }
            else
            {
                _socket.createSocket(SOCK_STREAM, AF_INET);

            }

            //设置非阻塞模式
            _socket.setblock(false);

            try
            {
                if(_port == 0)
                {
                    _socket.connect(_ip.c_str());
                }
                else
                {
                    _socket.connect(_ip, _port);
                }
            }
            catch(TC_SocketConnect_Exception &ex)
            {
                if(errno != EINPROGRESS)
                {
                    _socket.close();
                    return EM_CONNECT;
                }
            }

            if(errno != EINPROGRESS)
            {
                _socket.close();
                return EM_CONNECT;
            }

            TC_Epoller epoller(false);
            epoller.create(1);
            epoller.add(_socket.getfd(), 0, EPOLLOUT);
            int iRetCode = epoller.wait(_timeout);
            if (iRetCode < 0)
            {
                _socket.close();
                return EM_SELECT;
            }
            else if (iRetCode == 0)
            {
                _socket.close();
                return EM_TIMEOUT;
            }

            //设置为阻塞模式
            _socket.setblock(true);
        }
        catch(TC_Socket_Exception &ex)
        {
            _socket.close();
            return EM_SOCKET;
        }
    }
    return EM_SUCCESS;
}

int TC_TCPClient::send(const char *sSendBuffer, size_t iSendLen)
{
    int iRet = checkSocket();
    if(iRet < 0)
    {
        return iRet;
    }

    iRet = _socket.send(sSendBuffer, iSendLen);
    if(iRet < 0)
    {
        _socket.close();
        return EM_SEND;
    }

    return EM_SUCCESS;
}

int TC_TCPClient::recv(char *sRecvBuffer, size_t &iRecvLen)
{
    int iRet = checkSocket();
    if(iRet < 0)
    {
        return iRet;
    }

    TC_Epoller epoller(false);
    epoller.create(1);
    epoller.add(_socket.getfd(), 0, EPOLLIN);

    int iRetCode = epoller.wait(_timeout);
    if (iRetCode < 0)
    {
        _socket.close();
        return EM_SELECT;
    }
    else if (iRetCode == 0)
    {
        _socket.close();
        return EM_TIMEOUT;
    }

    epoll_event ev  = epoller.get(0);
    if(ev.events & EPOLLIN)
    {
        int iLen = _socket.recv((void*)sRecvBuffer, iRecvLen);
        if (iLen < 0)
        {
            _socket.close();
            return EM_RECV;
        }
        else if (iLen == 0)
        {
            _socket.close();
            return EM_CLOSE;
        }

        iRecvLen = iLen;
        return EM_SUCCESS;
    }
    else
    {
        _socket.close();
    }

    return EM_SELECT;
}

int TC_TCPClient::recvBySep(string &sRecvBuffer, const string &sSep)
{
    sRecvBuffer.clear();

    int iRet = checkSocket();
    if(iRet < 0)
    {
        return iRet;
    }

    TC_Epoller epoller(false);
    epoller.create(1);
    epoller.add(_socket.getfd(), 0, EPOLLIN);

    while(true)
    {
        int iRetCode = epoller.wait(_timeout);
        if (iRetCode < 0)
        {
            _socket.close();
            return EM_SELECT;
        }
        else if (iRetCode == 0)
        {
            _socket.close();
            return EM_TIMEOUT;
        }

        epoll_event ev  = epoller.get(0);
        if(ev.events & EPOLLIN)
        {
            char buffer[LEN_MAXRECV] = "\0";

            int len = _socket.recv((void*)&buffer, sizeof(buffer));
            if (len < 0)
            {
                _socket.close();
                return EM_RECV;
            }
            else if (len == 0)
            {
                _socket.close();
                return EM_CLOSE;
            }

            sRecvBuffer.append(buffer, len);

            if(sRecvBuffer.length() >= sSep.length() 
               && sRecvBuffer.compare(sRecvBuffer.length() - sSep.length(), sSep.length(), sSep) == 0)
            {
                break;
            }
        }
    }

    return EM_SUCCESS;
}

int TC_TCPClient::recvAll(string &sRecvBuffer)
{
    sRecvBuffer.clear();

    int iRet = checkSocket();
    if(iRet < 0)
    {
        return iRet;
    }

    TC_Epoller epoller(false);
    epoller.create(1);
    epoller.add(_socket.getfd(), 0, EPOLLIN);

    while(true)
    {
        int iRetCode = epoller.wait(_timeout);
        if (iRetCode < 0)
        {
            _socket.close();
            return EM_SELECT;
        }
        else if (iRetCode == 0)
        {
            _socket.close();
            return EM_TIMEOUT;
        }

        epoll_event ev  = epoller.get(0);
        if(ev.events & EPOLLIN)
        {
            char sTmpBuffer[LEN_MAXRECV] = "\0";

            int len = _socket.recv((void*)sTmpBuffer, LEN_MAXRECV);
            if (len < 0)
            {
                _socket.close();
                return EM_RECV;
            }
            else if (len == 0)
            {
                _socket.close();
                return EM_SUCCESS;
            }

            sRecvBuffer.append(sTmpBuffer, len);
        }
        else
        {
            _socket.close();
            return EM_SELECT;
        }
    }

    return EM_SUCCESS;
}

int TC_TCPClient::recvLength(char *sRecvBuffer, size_t iRecvLen)
{
    int iRet = checkSocket();
    if(iRet < 0)
    {
        return iRet;
    }

    size_t iRecvLeft = iRecvLen;
    iRecvLen = 0;

    TC_Epoller epoller(false);
    epoller.create(1);
    epoller.add(_socket.getfd(), 0, EPOLLIN);

    while(iRecvLeft != 0)
    {
        int iRetCode = epoller.wait(_timeout);
        if (iRetCode < 0)
        {
            _socket.close();
            return EM_SELECT;
        }
        else if (iRetCode == 0)
        {
            _socket.close();
            return EM_TIMEOUT;
        }

        epoll_event ev  = epoller.get(0);
        if(ev.events & EPOLLIN)
        {
            int len = _socket.recv((void*)(sRecvBuffer + iRecvLen), iRecvLeft);
            if (len < 0)
            {
                _socket.close();
                return EM_RECV;
            }
            else if (len == 0)
            {
                _socket.close();
                return EM_CLOSE;
            }

            iRecvLeft -= len;
            iRecvLen += len;
        }
        else
        {
            _socket.close();
            return EM_SELECT;
        }
    }

    return EM_SUCCESS;
}

int TC_TCPClient::sendRecv(const char* sSendBuffer, size_t iSendLen, char *sRecvBuffer, size_t &iRecvLen)
{
    int iRet = send(sSendBuffer, iSendLen);
    if(iRet != EM_SUCCESS)
    {
        return iRet;
    }

    return recv(sRecvBuffer, iRecvLen);
}

int TC_TCPClient::sendRecvBySep(const char* sSendBuffer, size_t iSendLen, string &sRecvBuffer, const string &sSep)
{
    int iRet = send(sSendBuffer, iSendLen);
    if(iRet != EM_SUCCESS)
    {
        return iRet;
    }

    return recvBySep(sRecvBuffer, sSep);
}

int TC_TCPClient::sendRecvLine(const char* sSendBuffer, size_t iSendLen, string &sRecvBuffer)
{
    return sendRecvBySep(sSendBuffer, iSendLen, sRecvBuffer, "\r\n");
}


int TC_TCPClient::sendRecvAll(const char* sSendBuffer, size_t iSendLen, string &sRecvBuffer)
{
    int iRet = send(sSendBuffer, iSendLen);
    if(iRet != EM_SUCCESS)
    {
        return iRet;
    }

    return recvAll(sRecvBuffer);
}

/*************************************TC_UDPClient**************************************/

int TC_UDPClient::checkSocket()
{
    if(!_socket.isValid())
    {
        try
        {
            if(_port == 0)
            {
                _socket.createSocket(SOCK_DGRAM, AF_LOCAL);
            }
            else
            {
                _socket.createSocket(SOCK_DGRAM, AF_INET);
            }
        }
        catch(TC_Socket_Exception &ex)
        {
            _socket.close();
            return EM_SOCKET;
        }

        try
        {
            if(_port == 0)
            {
                _socket.connect(_ip.c_str());
                if(_port == 0)
                {
                    _socket.bind(_ip.c_str());
                }
            }
            else
            {
                _socket.connect(_ip, _port);
            }
        }
        catch(TC_SocketConnect_Exception &ex)
        {
            _socket.close();
            return EM_CONNECT;
        }
        catch(TC_Socket_Exception &ex)
        {
            _socket.close();
            return EM_SOCKET;
        }
    }
    return EM_SUCCESS;
}

int TC_UDPClient::send(const char *sSendBuffer, size_t iSendLen)
{
    int iRet = checkSocket();
    if(iRet < 0)
    {
        return iRet;
    }

    iRet = _socket.send(sSendBuffer, iSendLen);
    if(iRet <0 )
    {
        return EM_SEND;
    }

    return EM_SUCCESS;
}

int TC_UDPClient::recv(char *sRecvBuffer, size_t &iRecvLen)
{
    string sTmpIp;
    uint16_t iTmpPort;

    return recv(sRecvBuffer, iRecvLen, sTmpIp, iTmpPort);
}

int TC_UDPClient::recv(char *sRecvBuffer, size_t &iRecvLen, string &sRemoteIp, uint16_t &iRemotePort)
{
    int iRet = checkSocket();
    if(iRet < 0)
    {
        return iRet;
    }

    TC_Epoller epoller(false);
    epoller.create(1);
    epoller.add(_socket.getfd(), 0, EPOLLIN);
    int iRetCode = epoller.wait(_timeout);
    if (iRetCode < 0)
    {
        return EM_SELECT;
    }
    else if (iRetCode == 0)
    {
        return EM_TIMEOUT;
    }

    epoll_event ev  = epoller.get(0);
    if(ev.events & EPOLLIN)
    {
        iRet = _socket.recvfrom(sRecvBuffer, iRecvLen, sRemoteIp, iRemotePort);
        if(iRet <0 )
        {
            return EM_SEND;
        }

        iRecvLen = iRet;
        return EM_SUCCESS;
    }

    return EM_SELECT;
}

int TC_UDPClient::sendRecv(const char *sSendBuffer, size_t iSendLen, char *sRecvBuffer, size_t &iRecvLen)
{
    int iRet = send(sSendBuffer, iSendLen);
    if(iRet != EM_SUCCESS)
    {
        return iRet;
    }

    return recv(sRecvBuffer, iRecvLen);
}

int TC_UDPClient::sendRecv(const char *sSendBuffer, size_t iSendLen, char *sRecvBuffer, size_t &iRecvLen, string &sRemoteIp, uint16_t &iRemotePort)
{
    int iRet = send(sSendBuffer, iSendLen);
    if(iRet != EM_SUCCESS)
    {
        return iRet;
    }

    return recv(sRecvBuffer, iRecvLen, sRemoteIp, iRemotePort);
}

}
