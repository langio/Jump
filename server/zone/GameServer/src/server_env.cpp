#include "server_env.h"
#include "table_mgr.h"

using namespace util;
using namespace protocol;

bool ServerEnv::init()
{
	static int32_t confMacroArray[] = {
				CONFIG_TEST, CONFIG_TEST1
		    };

	bool ret = TableMgr::getInstance()->reload(confMacroArray, CommFunc::sizeOf(confMacroArray));
	if (!ret)
	{
		cout << "load conf failed" << endl;
	}


	//连接redis
	if(!_rdx.connect("localhost", 6379))
	{
		INIT_LOG_ERROR("rdx connect err");
	}

	return ret;
}