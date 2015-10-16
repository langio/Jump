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
void dispatcher_release(DispatcherData *d)
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

//	static int32_t confMacroArray[] = {
//			CONFIG_TEST, CONFIG_TEST1
//	    };
//
//	bool ret = TableMgr::getInstance()->reload(confMacroArray, CommFunc::sizeOf(confMacroArray));
//	if (!ret)
//	{
//		cout << "load conf failed" << endl;
//	}

	if(!loadSo(ctx))
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
	//解析消息头部
	uint32_t space_counter = 0;
	uint32_t header_index = 0;

	const char* tmp = (const char*)msg;
	while(space_counter < 2 && header_index < sz)
	{
		if(' ' == tmp[header_index])
		{
			++space_counter;
		}

		++header_index;
	}

	string sExtHead = string(tmp, header_index);

	//cout << sExtHead << endl;

	vector<string> vExtHead = YAC_Common::sepstr<string>(sExtHead, " ");

	__BEGIN_PROC__

	//发到这里的消息应该是 fd open/close/data head body
	if(vExtHead.size() < 2)
	{
		LOG_ERROR(ctx, "invalid msg. sExtHead:%s", sExtHead.c_str());
		break;
	}

	if(2 == space_counter)
	{
		//解析消息
		int8_t * pkg = const_cast<int8_t*>((const int8_t*)msg + header_index);

		//协议包头
		PkgHead *head = (PkgHead*)pkg;
		head->unpack();
		head->client_fd = YAC_Common::strto<uint32_t>(vExtHead[0]);
		size_t pkg_len = sz - header_index;

//		p += sizeof(*head);

		//协议内容--测试代码
		if("data" == vExtHead[1])
		{
//			static int32_t counter = 0;
//
//			LOG_DEBUG(ctx, "cmd:0x%x body_len:%d client_fd:%d\n", head->cmd, head->body_len, head->client_fd);
//
//			login_req req;
//			if (!req.ParseFromArray(p, head->body_len))
//			{
//				LOG_ERROR(ctx, "login_req ParseFromArray failed!");
//				break;
//			}
//			LOG_ERROR(ctx, "login_req:\ncounter:%d\n%s", counter, req.DebugString().c_str());


			//根据命令字将消息发到对应的so中
			uint32_t cmd_type = GET_CMD_SVR_TYPE(head->cmd);

			int32_t module = 0;

			switch(cmd_type)
			{
				case AUTH_SVR:
					break;

				case LOGIN_SVR:
				{
					static uint32_t module_game = 0;
					if (module_game == 0)
					{
						module_game = skynet_handle_findname("game");
					}
					module = module_game;

					break;
				}
			}

			if(module > 0)
			{
				skynet_send(ctx, 0, module, PTYPE_TEXT, 0, pkg, pkg_len);
			}
			else
			{
				LOG_ERROR(ctx, "can't find module");
			}
		}


	}
	else
	{
		//出错
	}

	__END_PROC__

};

