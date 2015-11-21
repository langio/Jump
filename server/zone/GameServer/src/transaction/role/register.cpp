#include "register.pb.h"
#include "register.h"

int32_t Register::Init()
{
	return 0;
}

int32_t Register::Enter(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body)
{
	LOG_DEBUG(ctx, "cmd:0x%x body_len:%d client_fd:%d\n", pkg_head.cmd, pkg_head.body_len, pkg_head.client_fd);

	reg_req req;

	__BEGIN_PROC__

	if (!req.ParseFromArray(pkg_body, pkg_head.body_len))
	{
		LOG_ERROR(ctx, "reg_req ParseFromArray failed!");
		break;
	}

	reg_rsp rsp;

	//从redis拉取uid, uid和昵称全局唯一
	{
		//验证name是否唯一
		auto got_name_reply = [=, &rsp](Command<string>& c)
		{

			if(c.status() == c.NIL_REPLY)
			{
				//name不存在，获取uid
				GetUid(ctx, pkg_head, req, rsp);

			}
			else
			{
				if(c.ok())
				{
					//昵称已被使用，给客户端回包，流程结束
					LOG_DEBUG(ctx, "name duplicate. %s", req.name().c_str());

					rsp.set_ret(RET_REG_ERR_DUPLICATE_NAME);
				}
				else
				{
					//给客户端回包，流程结束
					LOG_ERROR(ctx, "get name err. cmd:%s status:%d", c.cmd().c_str(), c.status());

					rsp.set_ret(RET_REDIS_ERR_GET);
				}

				Send2Client(pkg_head, rsp);
			}

		};

		string key_value = req.name() + " 1";
		ServerEnv::getInstance().getRdx().command<string>({"GETSET", key_value}, got_name_reply);
	}



	__END_PROC__

	LOG_ERROR(ctx, "reg_req:\n%s", req.DebugString().c_str());

	return 0;
}


void Register::GetUid(struct skynet_context * ctx, const PkgHead& pkg_head, const reg_req& req, reg_rsp& rsp)
{
	auto got_reply = [=, &rsp](Command<string>& c)
	{
		if(c.ok())
		{
			int32_t uid = UIDBASE + S2I(c.reply());
			SetProfile(ctx, pkg_head, req, rsp, uid);
		}
		else
		{
			LOG_ERROR(ctx, "GetUid failed. cmd:%s status:%d", c.cmd().c_str(), c.status());

			//给客户端回包，流程结束
			rsp.set_ret(RET_REDIS_ERR_INCR);
			Send2Client(pkg_head, rsp);
		}


	};

	string key = UIDKEY;
	ServerEnv::getInstance().getRdx().command<string>({"INCR", key}, got_reply);
}

void Register::SetProfile(struct skynet_context * ctx, const PkgHead& pkg_head, const reg_req& req, reg_rsp& rsp, int32_t uid)
{
	PProfile profile;

	PPlayerId *player_id = profile.mutable_player_id();

	player_id->set_uid(uid);
	player_id->set_zone_id(req.zone_id());
	profile.mutable_base_info()->set_name(req.name());

	InitProfile(profile);
	string key = PROFILEPREFIX + I2S(player_id->uid()) + "_" + I2S(player_id->zone_id());
	string value = profile.SerializeAsString();

	auto got_reply = [=, &rsp](Command<string>& c)
	{
		if(c.ok())
		{
			//本地缓存profile
			PlayerID id;
			id.ToPlayerId(player_id);
			PProfile *p_profile = PlayerDataMgr::getInstance().GetProfile(id);
			*p_profile = profile;

			//给客户端回包，流程结束
			Send2Client(pkg_head, rsp);
		}
		else
		{
			LOG_ERROR(ctx, "SetProfile failed. cmd:%s status:%d", c.cmd().c_str(), c.status());

			//给客户端回包，流程结束
			Send2Client(pkg_head, rsp);
		}

	};

	string key_value = key + " " + value;
	ServerEnv::getInstance().getRdx().command<string>({"SET", key_value}, got_reply);

}

void Register::InitProfile(PProfile& profile)
{
	time_t now = YAC_TimeProvider::getInstance()->getNow();
	profile.mutable_base_info()->set_register_time(now);
	profile.mutable_base_info()->set_login_time(now);
}
