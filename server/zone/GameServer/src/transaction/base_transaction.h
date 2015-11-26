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
	Base():_finished(false){}
	virtual ~Base(){LOG_DEBUG(0, "destruct %s", GetName().c_str());}

	virtual int32_t Init() = 0;
	virtual int32_t Enter(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body) = 0;

	void End()
	{
		//销毁自己
		delete this;
	}

	void Send2Client(const PkgHead& pkg_head, const Message& message)
	{
		sendToClinet(pkg_head, message);
		End();
	}

protected:
	void SetFinished(){_finished = true;}

	bool IsFinished(){return _finished;}

	string GetName(){ return "Base";}

private:
	//atomic_int counter;
	bool _finished;
};

#endif

