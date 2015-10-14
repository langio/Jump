#ifndef _SERVICE_DISPATCHER_H
#define _SERVICE_DISPATCHER_H

#include "comm_def.h"

//根据命令字将消息发到对应的服务中
//命令字为4个字节整形，字节从高到底依次编号为1~4，第1个字节保留，第2字节表示服务，第3、4字节表示服务内的命令
typedef struct dispatcher_data
{
	int reserved;
}DispatcherData;

extern "C"
{

//#include "skynet.h"
#include "skynet_server.h"

	DispatcherData * dispatcher_create(void);
	void dispatcher_release(DispatcherData *d);
	static int _cb(struct skynet_context * ctx, void * ud, int type, int session,
			uint32_t source, const void * msg, size_t sz);
	int dispatcher_init(DispatcherData *d, struct skynet_context * ctx, char * parm);
	bool loadSo(struct skynet_context * ctx);
};

class dispatcher
{
public:
	static dispatcher& getInstance()
	{
		static dispatcher _instance;
		return _instance;
	};

	void dispatch(struct skynet_context * ctx, const void * msg, size_t sz);


private:
	dispatcher(){}
	~dispatcher(){}
};

#endif
