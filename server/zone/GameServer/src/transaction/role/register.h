#ifndef __REGISTER_H_
#define __REGISTER_H_

#include <typeinfo>
#include "base_transaction.h"

class Register : public Base
{
public:
	Register(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body)
	:Base(ctx, pkg_head, pkg_body){};
	~Register(){};

	int32_t Init();

	int32_t Enter();

protected:
	virtual string GetName(){ return "register";}

private:

	void SetName(struct skynet_context * ctx, PkgHead& head, const reg_req& req, reg_rsp& rsp);
	void GetUid(struct skynet_context * ctx, PkgHead& head, const reg_req& req, reg_rsp& rsp);
	void SetProfile(struct skynet_context * ctx, PkgHead& head, const reg_req& req, reg_rsp& rsp, int32_t uid);
	void InitProfile(PProfile& profile);

protected:
	reg_req _req;
	reg_rsp _rsp;
};

#endif
