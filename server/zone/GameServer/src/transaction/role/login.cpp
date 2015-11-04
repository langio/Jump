#include "login.pb.h"
#include "login.h"

int32_t Login::Init()
{
	return 0;
}

int32_t Login::Enter(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body)
{
	LOG_DEBUG(ctx, "cmd:0x%x body_len:%d client_fd:%d\n", pkg_head.cmd, pkg_head.body_len, pkg_head.client_fd);

	login_req req;

	__BEGIN_PROC__

	if (!req.ParseFromArray(pkg_body, pkg_head.body_len))
	{
		LOG_ERROR(ctx, "login_req ParseFromArray failed!");
		break;
	}
	__END_PROC__

	LOG_ERROR(ctx, "login_req:\ncounter:%d\n%s", counter, req.DebugString().c_str());

	return 0;
}
