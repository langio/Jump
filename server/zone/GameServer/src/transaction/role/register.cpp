#include "register.pb.h"
#include "register.h"

int32_t Register::Init()
{
	return 0;
}

//std::function<void(Command<ReplyT> &);



int32_t Register::Enter(struct skynet_context * ctx, const PkgHead& pkg_head, const char* pkg_body)
{
	LOG_DEBUG(ctx, "cmd:0x%x body_len:%d client_fd:%d\n", pkg_head.cmd, pkg_head.body_len, pkg_head.client_fd);

	PkgHead head = pkg_head;
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
		auto got_name_reply = [&](Command<string>& c)
		{

			if(c.status() == c.NIL_REPLY)
			{
				//name不存在，获取uid
				GetUid(ctx, head, req, rsp);

			}
			else
			{
				if(c.ok())
				{
					//昵称已被使用，给客户端回包，流程结束
					LOG_DEBUG(ctx, "name duplicate. %s", req.name().c_str());

					head.ret = RET_REG_ERR_DUPLICATE_NAME;
					//rsp.set_ret(RET_REG_ERR_DUPLICATE_NAME);
				}
				else
				{
					//给客户端回包，流程结束
					LOG_ERROR(ctx, "get name err. cmd:%s status:%d", c.cmd().c_str(), c.status());

					head.ret =  RET_REDIS_ERR_GET;
					//rsp.set_ret(RET_REDIS_ERR_GET);
				}

				Send2Client(head, rsp);
			}

		};

		string key = req.name();
		string value = "1";
		ServerEnv::getInstance().getRdx().command<string>({"GETSET", key, value}, got_name_reply);
	}



	__END_PROC__

	LOG_ERROR(ctx, "reg_req:\n%s", req.DebugString().c_str());

	return 0;
}


void Register::GetUid(struct skynet_context * ctx, PkgHead& head, const reg_req& req, reg_rsp& rsp)
{
	auto got_reply = [&](Command<string>& c1)
	{
		if(c1.ok())
		{
			int32_t uid = UIDBASE + S2I(c1.reply());
			SetProfile(ctx, head, req, rsp, uid);
		}
		else
		{
			LOG_ERROR(ctx, "GetUid failed. cmd:%s status:%d", c1.cmd().c_str(), c1.status());

			//给客户端回包，流程结束
			head.ret = RET_REDIS_ERR_INCR;
			//rsp.set_ret(RET_REDIS_ERR_INCR);
			Send2Client(head, rsp);
		}


	};

	string key = UIDKEY;
	ServerEnv::getInstance().getRdx().command<string>({"INCR", key}, got_reply);
}

void Register::SetProfile(struct skynet_context * ctx, PkgHead& head, const reg_req& req, reg_rsp& rsp, int32_t uid)
{
	PProfile profile;

	PPlayerId *player_id = profile.mutable_player_id();

	player_id->set_uid(uid);
	player_id->set_zone_id(req.zone_id());
	profile.mutable_base_info()->set_name(req.name());

	InitProfile(profile);
	string key = PROFILEPREFIX + I2S(player_id->uid()) + "_" + I2S(player_id->zone_id());
	string value = profile.SerializeAsString();

	auto got_reply = [&](Command<string>& c)
	{
		if(c.ok())
		{
			//本地缓存profile
			PlayerID id;
			id.ToPlayerId(player_id);
			PProfile *p_profile = PlayerDataMgr::getInstance().GetProfile(id);
			*p_profile = profile;

			//给客户端回包，流程结束
			Send2Client(head, rsp);
		}
		else
		{
			LOG_ERROR(ctx, "SetProfile failed. cmd:%s status:%d", c.cmd().c_str(), c.status());

			//给客户端回包，流程结束
			head.ret = RET_REDIS_ERR_SET;
			Send2Client(head, rsp);
		}

	};


	ServerEnv::getInstance().getRdx().command<string>({"SET", key, value}, got_reply);

}

void Register::InitProfile(PProfile& profile)
{
	time_t now = YAC_TimeProvider::getInstance()->getNow();
	profile.mutable_base_info()->set_register_time(now);
	profile.mutable_base_info()->set_login_time(now);
}
