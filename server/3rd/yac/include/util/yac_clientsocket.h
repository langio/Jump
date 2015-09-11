#ifndef _YAC_CLIENTSOCKET_H__
#define _YAC_CLIENTSOCKET_H__

#include "util/yac_socket.h"
#include "util/yac_http.h"

namespace util
{
/////////////////////////////////////////////////
/** 
 * @file  yac_clientsocket.h 
 * @brief 客户端发包收包类.
 *  
 * @author  jarodruan@tencent.com 
 */             
/////////////////////////////////////////////////
/**
*  @brief 解析endpoint异常类
*/
struct YAC_EndpointParse_Exception : public YAC_Exception
{
    YAC_EndpointParse_Exception(const string &buffer) : YAC_Exception(buffer){};
    ~YAC_EndpointParse_Exception() throw() {};
};

/**
 *  @brief 表示一个网络端口,支持以下格式:
 *  
 * 1:tcp -h 127.0.0.1 -p 2345 -t 10000
 *  
 * 2:tcp -h /tmp/sock.sock -p 0 -t 10000
 *  
 * 3:udp -h 127.0.0.1 -p 2345 -t 10000 
 *  
 * -p 0:表示本地套接字 
 *  
 * -q 0:表示qos的dscp值
 *  
 * 此时-h表示的文件路径
 */
class YAC_Endpoint
{
public:
    /**
     *
     */
    YAC_Endpoint();

    /**
     * @brief 构造函数
     * @param host
     * @param port
     * @param timeout, 超时时间, 毫秒
     * @param type, SOCK_STREAM或SOCK_DGRAM
     */
    YAC_Endpoint(const string& host, int port, int timeout, int istcp = true, int grid = 0,int qos = 0)
    {
        _host    = host;
        _port    = port;
        _timeout = timeout;
        _istcp   = istcp;
        _grid    = grid;
        _qos     = qos;
    }

    /**
     * @brief 用字符串描述来构造
     * @param desc
     */
    YAC_Endpoint(const string& desc)
    {
        parse(desc);
    }

    /**
     * @brief 拷贝构造
     * @param l
     */
    YAC_Endpoint(const YAC_Endpoint& l)
    {
        _host   = l._host;
        _port   = l._port;
        _timeout= l._timeout;
        _istcp  = l._istcp;
        _grid   = l._grid;
        _qos    = l._qos;
    }

    /**
     * @brief 赋值函数
     * @param l
     *
     * @return YAC_Endpoint&
     */
    YAC_Endpoint& operator = (const YAC_Endpoint& l)
    {
        if(this != &l)
        {
            _host   = l._host;
            _port   = l._port;
            _timeout= l._timeout;
            _istcp  = l._istcp;
            _grid   = l._grid;
            _qos    = l._qos;
        }

        return *this;
    }

    /**
     * ==
     * @param l
     *
     * @return bool
     */
    bool operator == (const YAC_Endpoint& l)
    {
        return (_host == l._host && _port == l._port && _timeout == l._timeout && _istcp == l._istcp && _grid == l._grid && _qos == l._qos);
    }

    /**
     * @brief 设置ip
     * @param str
     */
    void setHost(const string& host)    { _host = host; }

    /**
     * @brief 获取ip
     *
     * @return const string&
     */
    string getHost() const              { return _host; }

    /**
     * @brief 设置端口
     * @param port
     */
    void setPort(int port)              { _port = port; }

    /**
     * @brief 获取端口
     *
     * @return int
     */
    int getPort() const                 { return _port; }

    /**
     * @brief 设置超时时间
     * @param timeout
     */
    void setTimeout(int timeout)        { _timeout = timeout; }

    /**
     * @brief 获取超时时间
     *
     * @return int
     */
    int getTimeout() const              { return _timeout; }

    /**
     * @brief  是否是TCP, 否则则为UDP
     *
     * @return bool
     */
    bool isTcp() const                  { return _istcp; }

    /**
     * @brief 设置为TCP或UDP
     * @param bTcp
     */
    void setTcp(bool bTcp)              { _istcp = bTcp; }

    /**
     * @brief 获取路由状态
     * @param grid
     */
    int getGrid() const                 { return _grid; }

    /**
     * @brief 设置路由状态
     * @param grid
     */
    void setGrid(int grid)              { _grid = grid; }

    /**
     * @brief 获取路由状态
     * @param grid
     */
    int getQos() const                 { return _qos; }

    /**
     * @brief 设置路由状态
     * @param grid
     */
    void setQos(int qos)              { _qos = qos; }

    /**
     * @brief 是否是本地套接字
     *
     * @return bool
     */
    bool isUnixLocal() const            { return _port == 0; }

    /**
     * @brief 字符串描述
     *
     * @return string
     */
    string toString()
    {
        ostringstream os;
        os << (isTcp()?"tcp" : "udp") << " -h " << _host << " -p " << _port << " -t " << _timeout;
        if (_grid != 0) os << " -g " << _grid;
        if (_qos != 0) os << " -q " << _qos;
        return os.str();
    }

    /**
     * @brief  字符串形式的端口
	 * tcp:SOCK_STREAM 
	 *  
	 * udp:SOCK_DGRAM 
	 *  
	 * -h: ip 
	 *  
	 * -p: 端口 
	 *  
	 * -t: 超时时间, 毫秒 
	 *  
	 * -p 和 -t可以省略, -t默认10s 
	 *  
	 * tcp -h 127.0.0.1 -p 2345 -t 10000 
	 *  
     * @param desc
     */
    void parse(const string &desc);

protected:
    /**
     * ip
     */
    std::string _host;

    /**
     * 端口
     */
    int         _port;

    /**
     * 超时时间
     */
    int         _timeout;

    /**
     * 类型
     */
    int         _istcp;

    /**
     * 路由状态
     */
    int         _grid;

    /**
     *  网络Qos的dscp值
     */
    int         _qos;
};

/*************************************YAC_ClientSocket**************************************/

/**
* @brief 客户端socket相关操作基类
*/
class YAC_ClientSocket
{
public:

    /**
    *  @brief 构造函数
	 */
	YAC_ClientSocket() : _timeout(3000) {}

    /**
     * @brief 析够函数
     */
	virtual ~YAC_ClientSocket(){}

    /**
    * @brief 构造函数
    * @param sIP      服务器IP
	* @param iPort    端口, port为0时:表示本地套接字此时ip为文件路径
    * @param iTimeout 超时时间, 毫秒
	*/
	YAC_ClientSocket(const string &sIp, int iPort, int iTimeout) { init(sIp, iPort, iTimeout); }

    /**
    * @brief 初始化函数
    * @param sIP      服务器IP
	* @param iPort    端口, port为0时:表示本地套接字此时ip为文件路径
    * @param iTimeout 超时时间, 毫秒
	*/
	void init(const string &sIp, int iPort, int iTimeout)
    {
        _socket.close();
        _ip         = sIp;
        _port       = iPort;
        _timeout    = iTimeout;
    }

    /**
    * @brief 发送到服务器
    * @param sSendBuffer 发送buffer
    * @param iSendLen    发送buffer的长度
    * @return            int 0 成功,<0 失败
    */
    virtual int send(const char *sSendBuffer, size_t iSendLen) = 0;

    /**
    * @brief 从服务器返回不超过iRecvLen的字节
    * @param sRecvBuffer 接收buffer
	* @param iRecvLen    指定接收多少个字符才返回,输出接收数据的长度
    * @return            int 0 成功,<0 失败
    */
    virtual int recv(char *sRecvBuffer, size_t &iRecvLen) = 0;

    /**
    * @brief  定义发送的错误
    */
    enum
    {
        EM_SUCCESS  = 0,      	/** EM_SUCCESS:发送成功*/
		EM_SEND     = -1,		/** EM_SEND:发送错误*/
		EM_SELECT   = -2,	    /** EM_SELECT:select 错误*/
		EM_TIMEOUT  = -3,		/** EM_TIMEOUT:select超时*/
		EM_RECV     = -4,		/** EM_RECV: 接受错误*/
		EM_CLOSE    = -5,		/**EM_CLOSE: 服务器主动关闭*/
		EM_CONNECT  = -6,		/** EM_CONNECT : 服务器连接失败*/
		EM_SOCKET   = -7,		/**EM_SOCKET : SOCKET初始化失败*/
        
       
       
        
        
       
       
    };

protected:
    /**
     * 套接字句柄
     */
	YAC_Socket 	_socket;

    /**
     * ip或文件路径
     */
	string		_ip;

    /**
     * 端口或-1:标示是本地套接字
     */
	int     	_port;

    /**
     * 超时时间, 毫秒
     */
	int			_timeout;
};

/**
 * @brief TCP客户端Socket 
 * 多线程使用的时候，不用多线程同时send/recv，小心串包； 
 */
class YAC_TCPClient : public YAC_ClientSocket
{
public:
    /**
    * @brief  构造函数
	 */
	YAC_TCPClient(){}

    /**
    * @brief  构造函数
    * @param sIp       服务器Ip
    * @param iPort     端口
    * @param iTimeout  超时时间, 毫秒
	*/
	YAC_TCPClient(const string &sIp, int iPort, int iTimeout) : YAC_ClientSocket(sIp, iPort, iTimeout)
    {
    }

    /**
    * @brief  发送到服务器
    * @param sSendBuffer  发送buffer
    * @param iSendLen     发送buffer的长度
    * @return             int 0 成功,<0 失败
    */
    int send(const char *sSendBuffer, size_t iSendLen);

    /**
    * @brief  从服务器返回不超过iRecvLen的字节
    * @param sRecvBuffer 接收buffer
	* @param iRecvLen    指定接收多少个字符才返回,输出接收数据的长度
    * @return            int 0 成功,<0 失败
    */
    int recv(char *sRecvBuffer, size_t &iRecvLen);

    /**
	*  @brief 从服务器直到结束符(注意必须是服务器返回的结束符,
	*         而不是中间的符号 ) 只能是同步调用
    * @param sRecvBuffer 接收buffer, 包含分隔符
    * @param sSep        分隔符
    * @return            int 0 成功,<0 失败
    */
    int recvBySep(string &sRecvBuffer, const string &sSep);

    /**
     * @brief 接收倒服务器关闭连接为止
     * @param recvBuffer
     *
     * @return int 0 成功,<0 失败
     */
    int recvAll(string &sRecvBuffer);

    /**
     * @brief  从服务器返回iRecvLen的字节
     * @param sRecvBuffer, sRecvBuffer的buffer长度必须大于等于iRecvLen
	 * @param iRecvLen 
     * @return int 0 成功,<0 失败
     */
    int recvLength(char *sRecvBuffer, size_t iRecvLen);

    /**
    * @brief  发送到服务器, 从服务器返回不超过iRecvLen的字节
    * @param sSendBuffer  发送buffer
    * @param iSendLen     发送buffer的长度
    * @param sRecvBuffer  接收buffer
	* @param iRecvLen     接收buffer的长度指针[in/out], 
	*   			      输入时表示接收buffer的大小,返回时表示接收了多少个字符
    * @return             int 0 成功,<0 失败
    */
	int sendRecv(const char* sSendBuffer, size_t iSendLen, char *sRecvBuffer, size_t &iRecvLen);

    /**
    * @brief  发送倒服务器, 并等待服务器直到结尾字符, 包含结尾字符
	* sSep必须是服务器返回的结束符,而不是中间的符号，只能是同步调用
	* (一次接收一定长度的buffer,如果末尾是sSep则返回, 
	* 否则继续等待接收)
    * 
    * @param sSendBuffer  发送buffer
    * @param iSendLen     发送buffer的长度
    * @param sRecvBuffer  接收buffer
    * @param sSep         结尾字符
    * @return             int 0 成功,<0 失败
    */
	int sendRecvBySep(const char* sSendBuffer, size_t iSendLen, string &sRecvBuffer, const string &sSep);

    /**
    * @brief  发送倒服务器, 并等待服务器直到结尾字符(\r\n), 包含\r\n
    * 注意必须是服务器返回的结束符,而不是中间的符号
	* 只能是同步调用 
    * (一次接收一定长度的buffer,如果末尾是\r\n则返回,否则继续等待接收)
    * 
    * @param sSendBuffer  发送buffer
    * @param iSendLen     发送buffer的长度
    * @param sRecvBuffer  接收buffer
    * @param sSep         结尾字符
    * @return             int 0 成功,<0 失败
    */
	int sendRecvLine(const char* sSendBuffer, size_t iSendLen, string &sRecvBuffer);

    /**
     * @brief  发送到服务器, 接收直到服务器关闭连接为止
     * 此时服务器关闭连接不作为错误
     * @param sSendBuffer
     * @param iSendLen
     * @param sRecvBuffer
     *
     * @return int
     */
    int sendRecvAll(const char* sSendBuffer, size_t iSendLen, string &sRecvBuffer);

protected:
    /**
     * @brief  获取socket
     *
     * @return int
     */
    int checkSocket();
};

/*************************************YAC_TCPClient**************************************/
 /**
  * @brief  多线程使用的时候，不用多线程同时send/recv，小心串包
  */
class YAC_UDPClient : public YAC_ClientSocket
{
public:
    /**
    * @brief  构造函数
	 */
	YAC_UDPClient(){};

    /**
    * @brief  构造函数
    * @param sIp       服务器IP
    * @param iPort     端口
    * @param iTimeout  超时时间, 毫秒
	*/
	YAC_UDPClient(const string &sIp, int iPort, int iTimeout) : YAC_ClientSocket(sIp, iPort, iTimeout)
    {
    }

    /**
     * @brief  发送数据
     * @param sSendBuffer 发送buffer
     * @param iSendLen    发送buffer的长度
     *
     * @return            int 0 成功,<0 失败
     */
    int send(const char *sSendBuffer, size_t iSendLen);

    /**
     * @brief  接收数据
     * @param sRecvBuffer  接收buffer
	 * @param iRecvLen     输入/输出字段 
     * @return             int 0 成功,<0 失败
     */
    int recv(char *sRecvBuffer, size_t &iRecvLen);

    /**
     * @brief  接收数据, 并返回远程的端口和ip
     * @param sRecvBuffer 接收buffer
     * @param iRecvLen    输入/输出字段
     * @param sRemoteIp   输出字段, 远程的ip
     * @param iRemotePort 输出字段, 远程端口
     *
     * @return int 0 成功,<0 失败
     */
    int recv(char *sRecvBuffer, size_t &iRecvLen, string &sRemoteIp, uint16_t &iRemotePort);

    /**
     * @brief  发送并接收数据
     * @param sSendBuffer 发送buffer
     * @param iSendLen    发送buffer的长度
     * @param sRecvBuffer 输入/输出字段
     * @param iRecvLen    输入/输出字段
     *
     * @return int 0 成功,<0 失败
     */
    int sendRecv(const char *sSendBuffer, size_t iSendLen, char *sRecvBuffer, size_t &iRecvLen);

    /**
     * @brief  发送并接收数据, 同时获取远程的ip和端口
     * @param sSendBuffer  发送buffer
     * @param iSendLen     发送buffer的长度
     * @param sRecvBuffer  输入/输出字段
     * @param iRecvLen     输入/输出字段
     * @param sRemoteIp    输出字段, 远程的ip
     * @param iRemotePort  输出字段, 远程端口
     *
     * @return int 0 成功,<0 失败
     */
    int sendRecv(const char *sSendBuffer, size_t iSendLen, char *sRecvBuffer, size_t &iRecvLen, string &sRemoteIp, uint16_t &iRemotePort);

protected:
    /**
     * @brief  获取socket
     *
     * @return YAC_Socket&
     */
    int checkSocket();
};

}

#endif
