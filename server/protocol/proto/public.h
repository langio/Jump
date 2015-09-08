#include <netinet/in.h>

//此文件中只定义与客户端相关的结构体，命令字

struct PkgHead
{
	uint16_t ttl;
	uint32_t cmd;			//客户端请求命令字
	uint32_t client_fd;		//给客户端回包时用的，服务器为每个链接分配的id
	int32_t ret;			//服务器返回码

	void unpack()
	{
		ttl = ntohs(ttl);
		cmd = ntohl(cmd);
	}

	void pack()
	{
		cmd = htonl(cmd);
		ret = htonl(ret);
	}
};



//命令字定义
//命令字为4个字节整形，字节从高到底依次编号为1~4，第1个字节保留，第2字节表示服务，第3、4字节表示服务内的命令
#define GET_CMD_SVR_TYPE(cmd) (cmd >> 16)


//鉴权相关命令0x0001xxxx
const int CMD_AUTH_REQ = 0x00010000;		//鉴权请求


//注册、登录相关命令0x0002xxxx
const int CMD_LOGIN_REQ = 0x00020001;		//登录请求
