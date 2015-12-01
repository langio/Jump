#ifndef __BASE_TRANSACTION_H_
#define __BASE_TRANSACTION_H_
#include "server_env.h"
#include "player_data_mgr.h"

using namespace util;
using namespace protocol;
using namespace google::protobuf;

class Base
{
public:
	Base(struct skynet_context * ctx, const PkgHead pkg_head, const char* pkg_body)
	: _ctx(ctx)
	, _pkg_head(pkg_head)
	, _pkg_body(pkg_body)
	{}

	virtual ~Base(){LOG_DEBUG(0, "destruct %s", GetName().c_str());}

	//主要用来保存上下文，为回调做准备
	virtual int32_t Init() = 0;

	//逻辑入口
	virtual int32_t Enter() = 0;

	//逻辑出口
	void Exit()
	{
		LOG_DEBUG(0, "destruct %s", GetName().c_str());

		//销毁自己
		delete this;
	}

	void Send2Client(const PkgHead& pkg_head, const Message& message)
	{
		sendToClinet(pkg_head, message);
		Exit();
	}

protected:
	virtual string GetName(){ return "Base";}


protected:

	struct skynet_context * _ctx;
	PkgHead _pkg_head;
	const char* _pkg_body;
};

#endif

