//#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "service_dispatcher.h"

DispatcherData * dispatcher_create(void)
{
	DispatcherData * inst = (DispatcherData *)skynet_malloc(sizeof(*inst));
	inst->reserved = 0;

	printf("@@@@@@@@@@@@ dispatcher_create\n");

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

	printf("dispatcher_init\n");

	return 0;

}

void dispatcher::dispatch(const void * msg, size_t sz)
{
	//找到msg开始的第二个空格，这个空格之后是pb消息
	int iSpaceCounter = 0;
	int iPbHeaderIndex = 0;

	const char* tmp = (const char*)msg;
	while(iSpaceCounter < 2 && iPbHeaderIndex < sz)
	{
		if(' ' == tmp[iPbHeaderIndex])
		{
			++iSpaceCounter;
		}

		++iPbHeaderIndex;
	}

	if(2 == iSpaceCounter)
	{
		//解析消息头部，根据命令字转发
	}
	else
	{
		//写错误日志
	}

};



