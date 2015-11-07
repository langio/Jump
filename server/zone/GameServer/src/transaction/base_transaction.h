#ifndef __BASE_TRANSACTION_H_
#define __BASE_TRANSACTION_H_
#include "server_env.h"
#include "player_data_mgr.h"

using namespace util;
using namespace protocol;

class Base
{
public:
	Base():_finished(false){}
	virtual ~Base(){}

	virtual int32_t Init() = 0;
	virtual int32_t Enter(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body) = 0;

protected:
	void SetFinished(){_finished = true;}

	bool IsFinished(){return _finished;}

private:
	//atomic_int counter;
	bool _finished;
};

#endif

