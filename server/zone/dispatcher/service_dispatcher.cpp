#include "comm_def.h"
#include "send_util.h"
#include "public.h"

#include "login.pb.h"
#include "service_dispatcher.h"
#include "util/yac_common.h"
#include "ini_parse.h"

#include "table_mgr.h"

using namespace util;
using namespace protocol;


DispatcherData * dispatcher_create(void)
{
	DispatcherData * inst = (DispatcherData *)skynet_malloc(sizeof(*inst));
	inst->reserved = 0;

	//fprintf(stderr, "dispatcher_create\n");

	cout << "dispatcher_create" << endl;

	return inst;
}

//释放相关资源
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
			dispatcher::getInstance().dispatch(ctx, msg, sz);
			break;
	}

	return 0;
}


//初始化dispatcher
int dispatcher_init(DispatcherData *d, struct skynet_context * ctx, char * parm)
{

	skynet_callback(ctx, d, _cb);
	skynet_command(ctx, "REG", ".dispatcher");

	//fprintf(stderr, "dispatcher_init\n");
	cout << "dispatcher_init" << endl;

	static int32_t confMacroArray[] = {
			CONFIG_TEST
	    };

	bool ret = TableMgr::getInstance()->reload(confMacroArray, CommFunc::sizeOf(confMacroArray));
	if (!ret)
	{
		cout << "load conf failed" << endl;
	}

	if(loadSo(ctx))
	{
		return -1;
	}

	return 0;

}

bool loadSo(struct skynet_context * ctx)
{
	bool ret = true;

	string ini_file("../cfg/game.ini");
	INIParse ini(ini_file.c_str());

	__BEGIN_PROC__

	if(!ini.CheckINI())
	{
		LOG_ERROR(ctx, "check ini file %s failed", ini_file.c_str());
		ret = false;
		break;
	}

	const char* so_file_name = ini.GetStringValue("SO_FILE_NAME", "so");
	if(NULL == so_file_name)
	{
		LOG_ERROR(ctx, "get so file name failed");
		ret = false;
		break;
	}

	vector<string> v_so_file_name = YAC_Common::sepstr<string>(string(so_file_name), ";");

	//加载so
	for(size_t i=0; i<v_so_file_name.size(); ++i)
	{
		struct skynet_context *so_ctx = skynet_context_new(v_so_file_name[i].c_str(), NULL);
		if(NULL == so_ctx)
		{
			LOG_ERROR(ctx, "Can't launch %s service", v_so_file_name[i].c_str());

			ret = false;
			break;
		}
	}

	if(!ret)
	{
		break;
	}

	__END_PROC__

	return ret;
}

void dispatcher::dispatch(struct skynet_context * ctx, const void * msg, size_t sz)
{
	//解析msg头部
	uint32_t iSpaceCounter = 0;
	uint32_t iPbHeaderIndex = 0;

	const char* tmp = (const char*)msg;
	while(iSpaceCounter < 2 && iPbHeaderIndex < sz)
	{
		if(' ' == tmp[iPbHeaderIndex])
		{
			++iSpaceCounter;
		}

		++iPbHeaderIndex;
	}

	string sExtHead = string(tmp, iPbHeaderIndex);

	cout << sExtHead << endl;

	vector<string> vExtHead = YAC_Common::sepstr<string>(sExtHead, " ");

	__BEGIN_PROC__

	//发到这里的消息应该是 fd open/close/data head body
	if(vExtHead.size() < 2)
	{
		LOG_ERROR(ctx, "invalid msg. sExtHead:%s", sExtHead.c_str());
		break;
	}

	if(2 == iSpaceCounter)
	{
		//解析包头
		int8_t * msg_body = const_cast<int8_t*>((const int8_t*)msg + iPbHeaderIndex);
		PkgHead *head = (PkgHead*)msg_body;
		head->unpack();
		head->client_fd = YAC_Common::strto<uint32_t>(vExtHead[0]);

		//根据命令字将消息发到对应的so中
		uint32_t iCmdType = GET_CMD_SVR_TYPE(head->cmd);

		switch(iCmdType)
		{
		case AUTH_SVR:
			break;

		case LOGIN_SVR:
			break;
		}
	}
	else
	{
		//出错
	}

	login_req req;

	int i = 0;
	for(i=0; i<100; ++i)
	{
		req.add_item(i);
	}

	__END_PROC__

};


int dispatcher::get_sys_cmd(const void * msg, size_t sz, int& cmd)
{
	int ret = -1;

	return ret;
}

