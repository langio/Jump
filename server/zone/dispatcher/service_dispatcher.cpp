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


//���������ֽ���Ϣ������Ӧ�ķ�����
//������Ϊ4���ֽ����Σ��ֽڴӸߵ������α��Ϊ1~4����2�ֽڱ�ʾ���񣬵�3��4�ֽڱ�ʾ�����ڵ�����
typedef struct dispatcher_data
{
	int reserved;
}DispatcherData;

DispatcherData * dispatcher_create(void)
{
	printf("@@@@@@@@@@@@ dispatcher_create\n");
	return NULL;
}

//�ͷ�gate�����Դ
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


//��ʼ��dispatcher��ز���
int dispatcher_init(DispatcherData *d, struct skynet_context * ctx, char * parm)
{

	skynet_callback(ctx, d, _cb);

	printf("dispatcher_init\n");

	return 0;

}

void dispatcher::dispatch(const void * msg, size_t sz)
{
	//�ҵ�msg��ʼ�ĵڶ����ո�����ո�֮����pb��Ϣ
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
		//������Ϣͷ��������������ת��
	}
	else
	{
		//д������־
	}

};



