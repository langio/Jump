#include "login.pb.h"
#include "login.h"

int32_t Login::Init()
{
	return 0;
}

int32_t Login::Enter()
{
	LOG_DEBUG(_ctx, "cmd:0x%x body_len:%d client_fd:%d\n", _pkg_head.cmd, _pkg_head.body_len, _pkg_head.client_fd);

	__BEGIN_PROC__

	if (!_req.ParseFromArray(_pkg_body, _pkg_head.body_len))
	{
		LOG_ERROR(_ctx, "login_req ParseFromArray failed!");
		break;
	}

	PlayerID player_id;
	player_id.Init(_req.uid(), _req.zone_id());
	PProfile *p_profile = PlayerDataMgr::getInstance().GetProfile(player_id);


	if(p_profile != NULL)
	{
		PProfile *rsp_profile = _rsp.mutable_profile();
		*rsp_profile = *p_profile;

		sendToClinet(_pkg_head, _rsp);
	}
	else
	{
		auto got_reply = [&](Command<string>& c)
		{
			if(!c.ok())
			{
				LOG_ERROR(_ctx, "cmd:%s status:%d", c.cmd().c_str(), c.status());
				return;
			}

			LOG_DEBUG(_ctx, "cmd:%s", c.cmd().c_str());

			PProfile profile;
			if (!profile.ParseFromString(c.reply()))
			{
				LOG_ERROR(_ctx, "profile ParseFromArray failed!");
				return;
			}

			bool exist;
			PlayerID player_id;
			player_id.Init(_req.uid(), _req.zone_id());
			PProfile *p_profile = PlayerDataMgr::getInstance().Add(player_id, exist);
			if(exist)
			{
				//从cache中读数据理论上不应该走到这里
				LOG_ERROR(_ctx, "profile exist!!!");
				return;
			}
			*p_profile = profile;

			PProfile *rsp_profile = _rsp.mutable_profile();
			*rsp_profile = *p_profile;

			sendToClinet(_pkg_head, _rsp);
		};

		string profile_key = CommFunc::getProfileKey(player_id._uid, player_id._zone_id);
		ServerEnv::getInstance().getRdx().command<string>({"GET", profile_key}, got_reply);

	}



	__END_PROC__

	LOG_ERROR(_ctx, "login_req:\n%s", _req.DebugString().c_str());

	return 0;


}
