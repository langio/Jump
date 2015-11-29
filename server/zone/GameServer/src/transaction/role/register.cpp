#include "register.pb.h"
#include "register.h"


int32_t Register::Init()
{
	int32_t iRet = 0;

	__BEGIN_PROC__

	if (!_req.ParseFromArray(_pkg_body, _pkg_head.body_len))
	{
		LOG_ERROR(_ctx, "reg_req ParseFromArray failed!");
		iRet = -1;
		break;
	}

	__END_PROC__

	return iRet;
}


int32_t Register::Enter()
{
	LOG_DEBUG(_ctx, "cmd:0x%x body_len:%d client_fd:%d\n", _pkg_head.cmd, _pkg_head.body_len, _pkg_head.client_fd);

	__BEGIN_PROC__

	if(Init() != 0)
	{
		break;
	}

	//从redis拉取uid, uid和昵称全局唯一
	{
		//验证name是否唯一
		auto got_name_reply = [&](Command<string>& c)
		{

			if(c.status() == c.NIL_REPLY)
			{
				//name不存在，获取uid
				GetUid();

			}
			else
			{
				if(c.ok())
				{
					//昵称已被使用，给客户端回包，流程结束
					LOG_DEBUG(_ctx, "name duplicate. %s", _req.name().c_str());

					_pkg_head.ret = RET_REG_ERR_DUPLICATE_NAME;
				}
				else
				{
					//给客户端回包，流程结束
					LOG_ERROR(_ctx, "get name err. cmd:%s status:%d", c.cmd().c_str(), c.status());

					_pkg_head.ret =  RET_REDIS_ERR_GET;
				}

				Send2Client(_pkg_head, _rsp);
			}

		};

		string key = _req.name();
		string value = "1";
		ServerEnv::getInstance().getRdx().command<string>({"GETSET", key, value}, got_name_reply);
	}



	__END_PROC__

	LOG_DEBUG(_ctx, "reg_req:\n%s", _req.DebugString().c_str());

	return 0;
}


void Register::GetUid()
{
	auto got_reply = [&](Command<string>& c1)
	{
		if(c1.ok())
		{
			int32_t uid = UIDBASE + S2I(c1.reply());
			SetProfile(uid);
		}
		else
		{
			LOG_ERROR(_ctx, "GetUid failed. cmd:%s status:%d", c1.cmd().c_str(), c1.status());

			//给客户端回包，流程结束
			_pkg_head.ret = RET_REDIS_ERR_INCR;
			Send2Client(_pkg_head, _rsp);
		}


	};

	string key = UIDKEY;
	ServerEnv::getInstance().getRdx().command<string>({"INCR", key}, got_reply);
}

void Register::SetProfile(int32_t uid)
{
	//初始化账号信息
	PReginfo reginfo;
	reginfo.set_account(_req.account());
	reginfo.set_passwd(_req.passwd());
	reginfo.set_uid(uid);
	reginfo.set_zone_id(_req.zone_id());
	reginfo.set_name(_req.name());

	string reginfo_key = _req.account();
	string reginfo_value = reginfo.SerializeAsString();


	//初始化profile
	PPlayerId *player_id = _profile.mutable_player_id();

	player_id->set_uid(uid);
	player_id->set_zone_id(_req.zone_id());
	_profile.mutable_base_info()->set_name(_req.name());

	InitProfile();
	string profile_key = PROFILEPREFIX + I2S(player_id->uid()) + "_" + I2S(player_id->zone_id());
	string profile_value = _profile.SerializeAsString();

	auto got_reply = [&](Command<string>& c)
	{
		if(c.ok())
		{
			//本地缓存profile
			PlayerID id;
			id.ToPlayerId(player_id);
			PProfile *p_profile = PlayerDataMgr::getInstance().GetProfile(id);
			*p_profile = _profile;

			_pkg_head.ret = RET_OK;
			PProfile *rsp_profile = _rsp.mutable_profile();
			*rsp_profile = _profile;

			//给客户端回包，流程结束
			Send2Client(_pkg_head, _rsp);
		}
		else
		{
			LOG_ERROR(_ctx, "SetProfile failed. cmd:%s status:%d", c.cmd().c_str(), c.status());

			//给客户端回包，流程结束
			_pkg_head.ret = RET_REDIS_ERR_SET;
			Send2Client(_pkg_head, _rsp);
		}

	};


	ServerEnv::getInstance().getRdx().command<string>({"MSET", reginfo_key, reginfo_value, profile_key, profile_value}, got_reply);

}

void Register::InitProfile()
{
	time_t now = YAC_TimeProvider::getInstance()->getNow();
	_profile.mutable_base_info()->set_register_time(now);
	_profile.mutable_base_info()->set_login_time(now);
}
