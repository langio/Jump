#ifndef _SERVICE_DISPATCHER_H
#define _SERVICE_DISPATCHER_H


//���������ֽ���Ϣ������Ӧ�ķ�����
//������Ϊ4���ֽ����Σ��ֽڴӸߵ������α��Ϊ1~4����2�ֽڱ�ʾ���񣬵�3��4�ֽڱ�ʾ�����ڵ�����
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
