#ifndef __LOGIN_H_
#define __LOGIN_H_

#include "base_transaction.h"

class Login : public Base
{
public:
	Login(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body)
	:Base(ctx, pkg_head, pkg_body){};

	~Login(){};

	int32_t Init();

	int32_t Enter();

private:
	login_req _req;
	login_rsp _rsp;
};

#endif
