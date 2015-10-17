#ifndef __BASE_TRANSACTION_H_
#define __BASE_TRANSACTION_H_
#include "server_env.h"


class Base
{
public:
	Base():counter(1), _finished(false){}
	virtual ~Base(){}

	virtual int32_t Init() = 0;
	virtual int32_t Enter(struct skynet_context * ctx, const PkgHead& pkg_head, const char* msg_body) = 0;

protected:
	void SetFinished(){_finished = true;}

	bool IsFinished(){return _finished;}

private:
	atomic_int counter;
	bool _finished;
};

#endif

