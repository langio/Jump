#ifndef _SERVICE_DISPATCHER_H
#define _SERVICE_DISPATCHER_H


//根据命令字将消息发到对应的服务中
//命令字为4个字节整形，字节从高到底依次编号为1~4，低2字节表示服务，第3、4字节表示服务内的命令
typedef struct dispatcher_data
{
	int reserved;
}DispatcherData;

extern "C"
{

#include "skynet.h"

	DispatcherData * dispatcher_create(void);
	void gate_release(DispatcherData *d);
	static int _cb(struct skynet_context * ctx, void * ud, int type, int session,
			uint32_t source, const void * msg, size_t sz);
	int dispatcher_init(DispatcherData *d, struct skynet_context * ctx, char * parm);
};

class dispatcher
{
public:
	static dispatcher& getInstance()
	{
		static dispatcher m_dispatcher;
		return m_dispatcher;
	};

	void dispatch(const void * msg, size_t sz);

private:
	int get_sys_cmd(const void * msg, size_t sz, int& cmd);

private:
	dispatcher(){}
	~dispatcher(){}
};

#endif
