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

//	for(int32_t j=0; j<10000000; ++j)
//	{
//		PlayerID id;
//		id._uid = j;
//		id._zone_id = 1;
//		bool exist;
//		PProfile *p_profile = PlayerDataMgr::getInstance().Add(id, exist);
//		p_profile->Clear();
//
//		PTokenMoney *token_money = p_profile->mutable_token_money();
//		for(int32_t i=0; i<10000; ++i)
//		{
//			token_money->add_testfield(i);
//		}
//
//		if(j%10000 == 0)
//		{
//			printf("mem pool add repeated ok\n");
//		}
//		PlayerDataMgr::getInstance().Del(id);
//
//		if(j%10000 == 0)
//		{
//			printf("mem pool del ok\n");
//		}
//	}
//
//	return 0;


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

		string key = NICKPREFIX + _req.name();
		string value = "1";
		ServerEnv::getInstance().getRdx().command<string>({"GETSET", key, value}, got_name_reply);
	}



	__END_PROC__

	LOG_DEBUG(_ctx, "reg_req:\n%s", _req.DebugString().c_str());

	return 0;
}


void Register::GetUid()
{
	auto got_reply = [&](Command<int32_t>& c1)
	{
		if(c1.ok())
		{
			int32_t uid = UIDBASE + c1.reply();
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
	ServerEnv::getInstance().getRdx().command<int32_t>({"INCR", key}, got_reply);
}

void Register::SetProfile(int32_t uid)
{
	//初始化账号信息
	PReginfo reginfo;
	InitReginfo(reginfo, uid);

	string reginfo_key = REGINFOPREFIX + _req.account();
	string reginfo_value = reginfo.SerializeAsString();


	//初始化profile
	InitProfile(uid);

	PPlayerId *player_id = _p_profile->mutable_player_id();
	//string profile_key = PROFILEPREFIX + I2S(player_id->uid()) + "_" + I2S(player_id->zone_id());
	string profile_key = CommFunc::getProfileKey(player_id->uid(), player_id->zone_id());
	string profile_value = _p_profile->SerializeAsString();


	auto got_reply = [&](Command<string>& c)
	{
		if(c.ok())
		{
			_pkg_head.ret = RET_OK;
			PProfile *rsp_profile = _rsp.mutable_profile();
			*rsp_profile = *_p_profile;

			//给客户端回包，流程结束
			Send2Client(_pkg_head, _rsp);
		}
		else
		{
			PPlayerId *player_id = _p_profile->mutable_player_id();
			PlayerID id;
			id.Init(*player_id);
			PlayerDataMgr::getInstance().Del(id);

			LOG_ERROR(_ctx, "SetProfile failed. cmd:%s status:%d", c.cmd().c_str(), c.status());

			//给客户端回包，流程结束
			_pkg_head.ret = RET_REDIS_ERR_SET;
			Send2Client(_pkg_head, _rsp);
		}

	};


	ServerEnv::getInstance().getRdx().command<string>({"MSET", reginfo_key, reginfo_value, profile_key, profile_value}, got_reply);

}

void Register::InitReginfo(PReginfo& reginfo, int32_t uid)
{
	reginfo.set_account(_req.account());
	reginfo.set_passwd(_req.passwd());
	reginfo.set_uid(uid);
	reginfo.set_zone_id(_req.zone_id());
	reginfo.set_name(_req.name());
}

int32_t Register::InitProfile(int32_t uid)
{

	int32_t ret = 0;

	PPlayerId init_id;
	init_id.set_uid(uid);
	init_id.set_zone_id(_req.zone_id());

	__BEGIN_PROC__

	PlayerID id;
	id.Init(init_id);
	bool exist;
	_p_profile = PlayerDataMgr::getInstance().Add(id, exist);
	if(NULL == _p_profile)
	{
		LOG_ERROR(_ctx, "alloc profile failed");
		ret = -1;
		break;
	}
	else if(exist)
	{
		LOG_ERROR(_ctx, "profile exist. %s", id.ToString().c_str());
		ret = -1;
		break;
	}

	time_t now = YAC_TimeProvider::getInstance()->getNow();

	//玩家id
	PPlayerId *player_id = _p_profile->mutable_player_id();
	player_id->set_uid(uid);
	player_id->set_zone_id(_req.zone_id());

	//账户充值消费信息
	PAccount *account = _p_profile->mutable_account();
	account->set_history_total(0);
	account->set_balance(0);
	account->set_last_recharge_time(0);
	account->set_last_consume_time(0);

	//游戏内代币
	PTokenMoney *token_money = _p_profile->mutable_token_money();
	token_money->set_gold(0);


	//玩家基础信息
	PBaseInfo *base_info = _p_profile->mutable_base_info();
	base_info->set_name(_req.name());
	base_info->set_register_time(now);
	base_info->set_login_time(now);
	base_info->set_gender(0);

	__END_PROC__

	return ret;

}
