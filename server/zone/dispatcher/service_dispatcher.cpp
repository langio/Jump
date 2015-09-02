#include "comm_def.h"

#include "login.pb.h"
#include "service_dispatcher.h"


DispatcherData * dispatcher_create(void)
{
	DispatcherData * inst = (DispatcherData *)skynet_malloc(sizeof(*inst));
	inst->reserved = 0;

	//fprintf(stderr, "dispatcher_create\n");

	cout << "dispatcher_create" << endl;

	return inst;
}

//释放gate相关资源
void gate_release(DispatcherData *d)
{
	skynet_free(d);
}


static int _cb(struct skynet_context * ctx, void * ud, int type, int session,
		uint32_t source, const void * msg, size_t sz)
{
	DispatcherData *d = (DispatcherData *)ud;
	if(d)
	{

	}

	switch (type)
	{
		case PTYPE_TEXT:
			//_ctrl(g, msg, (int) sz);
			dispatcher::getInstance().dispatch(msg, sz);
			break;
	}

	return 0;
}


//初始化dispatcher相关参数
int dispatcher_init(DispatcherData *d, struct skynet_context * ctx, char * parm)
{

	skynet_callback(ctx, d, _cb);
	skynet_command(ctx, "REG", ".dispatcher");

	//fprintf(stderr, "dispatcher_init\n");
	cout << "dispatcher_init" << endl;

	return 0;

}

void dispatcher::dispatch(const void * msg, size_t sz)
{
	//找到msg开始的第二个空格，这个空格之后是pb消息
	unsigned iSpaceCounter = 0;
	unsigned iPbHeaderIndex = 0;

	const char* tmp = (const char*)msg;
	while(iSpaceCounter < 2 && iPbHeaderIndex < sz)
	{
		if(' ' == tmp[iPbHeaderIndex])
		{
			++iSpaceCounter;
		}

		++iPbHeaderIndex;
	}

	string s = string(tmp, iPbHeaderIndex);

	cout << s << endl;

	if(2 == iSpaceCounter)
	{
		//解析消息头部，根据命令字转发
	}
	else
	{
		//写错误日志
	}

	login_req req;

	int i = 0;
	for(i=0; i<100; ++i)
	{
		req.add_item(i);
	}

};


int dispatcher::get_sys_cmd(const void * msg, size_t sz, int& cmd)
{
	int ret = -1;

	return ret;
}

