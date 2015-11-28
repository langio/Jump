#include "login.pb.h"
#include "login.h"

int32_t Login::Init()
{
	return 0;
}

int32_t Login::Enter()
{return 0;
	/*
	 *
	LOG_DEBUG(ctx, "cmd:0x%x body_len:%d client_fd:%d\n", pkg_head.cmd, pkg_head.body_len, pkg_head.client_fd);

	login_req req;

	__BEGIN_PROC__

	if (!req.ParseFromArray(pkg_body, pkg_head.body_len))
	{
		LOG_ERROR(ctx, "login_req ParseFromArray failed!");
		break;
	}

	PlayerID player_id;
	player_id.Init(req.uid(), req.zone_id());
	PProfile *p_profile = PlayerDataMgr::getInstance().GetProfile(player_id);

	login_rsp rsp;
	if(p_profile != NULL)
	{
		PProfile *rsp_profile = rsp.mutable_profile();
		*rsp_profile = *p_profile;

		sendToClinet(pkg_head, rsp);
	}
	else
	{
		auto got_reply = [=, &rsp](Command<string>& c)
		{
			if(!c.ok())
			{
				LOG_ERROR(ctx, "cmd:%s status:%d", c.cmd().c_str(), c.status());
				return;
			}

			LOG_DEBUG(ctx, "cmd:%s", c.cmd().c_str());

			PProfile profile;
			if (!profile.ParseFromString(c.reply()))
			{
				LOG_ERROR(ctx, "profile ParseFromArray failed!");
				return;
			}

			bool exist;
			PProfile *p_profile = PlayerDataMgr::getInstance().Add(player_id, exist);
			if(exist)
			{
				//从cache中读数据理论上不应该走到这里
				LOG_ERROR(ctx, "profile exist!!!");
				return;
			}
			*p_profile = profile;

			PProfile *rsp_profile = rsp.mutable_profile();
			*rsp_profile = *p_profile;

			sendToClinet(pkg_head, rsp);
		};

		string key = player_id.ToString();
		ServerEnv::getInstance().getRdx().command<string>({"GET", key}, got_reply);

	}



	__END_PROC__

	LOG_ERROR(ctx, "login_req:\n%s", req.DebugString().c_str());

	return 0;

	*/
}
