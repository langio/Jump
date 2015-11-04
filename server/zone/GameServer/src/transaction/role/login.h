#ifndef __LOGIN_H_
#define __LOGIN_H_

#include "base_transaction.h"

class Login : public Base
{
public:
	Login(){};
	~Login(){};

	int32_t Init();

	int32_t Enter(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body);
};

#endif
