#ifndef __REGISTER_H_
#define __REGISTER_H_

#include <typeinfo>
#include "base_transaction.h"
#include "reginfo.pb.h"

class Register : public Base
{
public:
	Register(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body)
	:Base(ctx, pkg_head, pkg_body)
	, _p_profile(NULL){};

	~Register(){};

	int32_t Init();

	int32_t Enter();

protected:
	virtual string GetName(){ return "register";}

private:

	void GetUid();
	void SetProfile(int32_t uid);
	void InitReginfo(PReginfo& reginfo, int32_t uid);
	int32_t InitProfile(int32_t uid);

protected:
	reg_req _req;
	reg_rsp _rsp;
	PProfile *_p_profile;
	PProfile _profile;
};

#endif
