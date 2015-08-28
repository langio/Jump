#include "skynet.h"
#include "skynet_socket.h"
//#include "databuffer.h"
//#include "hashid.h"

//#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "service_dispatcher.h"


//根据命令字将消息发到对应的服务中
//命令字为4个字节整形，字节从高到底依次编号为1~4，低2字节表示服务，第3、4字节表示服务内的命令
typedef struct dispatcher_data
{
	int reserved;
}DispatcherData;

DispatcherData * dispatcher_create(void)
{
	printf("@@@@@@@@@@@@ dispatcher_create\n");
	return NULL;
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



