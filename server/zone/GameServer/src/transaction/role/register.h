#ifndef __REGISTER_H_
#define __REGISTER_H_

#include <typeinfo>
#include "base_transaction.h"
#include "reginfo.pb.h"

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

	void GetUid();
	void SetProfile(int32_t uid);
	void InitProfile();

protected:
	reg_req _req;
	reg_rsp _rsp;
	PProfile _profile;
};

#endif
