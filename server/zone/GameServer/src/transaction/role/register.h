#ifndef __REGISTER_H_
#define __REGISTER_H_

#include "base_transaction.h"

class Register : public Base
{
public:
	Register(){};
	~Register(){};

	int32_t Init();

	int32_t Enter(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body);

private:

	void SetName(struct skynet_context * ctx, const PkgHead& pkg_head, const reg_req& req, reg_rsp& rsp);
	void GetUid(struct skynet_context * ctx, const PkgHead& pkg_head, const reg_req& req, reg_rsp& rsp);
	void SetProfile(struct skynet_context * ctx, const PkgHead& pkg_head, const reg_req& req, reg_rsp& rsp, int32_t uid);
	void InitProfile(PProfile& profile);
};

#endif
