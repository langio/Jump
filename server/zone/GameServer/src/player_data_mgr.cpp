#include "player_data_mgr.h"
#include "comm_def.h"
#include "ini_parse.h"


size_t PlayerDataMgr::GetMaxSize(uint32_t bucket, uint32_t node)
{
    return _player_data_pool.GetMaxSize(bucket, node);
}

int32_t PlayerDataMgr::Init()
{
	int32_t ret = 0;

	if(0 == _count)
	{
		string ini_file("../cfg/game.ini");
		INIParse ini(ini_file.c_str());

		__BEGIN_PROC__

		if(!ini.CheckINI())
		{
			LOG_ERROR(0, "check ini file %s failed", ini_file.c_str());
			ret = -1;
			break;
		}

		uint32_t player_max_bucket = ini.GetUIntValue("RUN_ARGS", "player_max_bucket");
		uint32_t player_max_node = ini.GetUIntValue("RUN_ARGS", "player_max_node");
		size_t max_shm_size = GetMaxSize(player_max_bucket, player_max_node);
		uint32_t shm_key = ini.GetUIntValue("game.ini", "palyer_data_mgr_key");
		_shm.init(max_shm_size, shm_key);

		bool check_header = !_shm.iscreate();

		ret = (int32_t)_player_data_pool.Init(_shm.getPointer(), player_max_bucket, player_max_node, check_header);

		uint32_t palyer_data_mutex_key = ini.GetUIntValue("RUN_ARGS", "palyer_data_mutex_key");
		_sem_mutex.init(palyer_data_mutex_key);

		__END_PROC__

		++_count;
	}

    return ret;
}


PProfile * PlayerDataMgr::Add(PlayerID player_id, bool & exist)
{
    exist = true;

    _sem_mutex.rlock();

    PProfile *profile = _player_data_pool.Get(player_id.id);
    if (profile != NULL)
    {
    	_sem_mutex.unrlock();
        return profile;
    }
    _sem_mutex.unrlock();


    _sem_mutex.wlock();

    exist = false;

    profile = _player_data_pool.Alloc(player_id.id);

    _sem_mutex.unwlock();

    return profile;
}

/*
 * 游戏上层逻辑取在线玩家数据，保证数据已经加载,并且玩家在线
 */
PProfile *PlayerDataMgr::GetProfile(PlayerID player_id)
{
	PProfile *profile = NULL;

	_sem_mutex.rlock();
	__BEGIN_PROC__

	profile = _player_data_pool.Get(player_id.id);
    if (profile == NULL)
    {
        break;
    }

    __END_PROC__
    _sem_mutex.unrlock();

    return profile;
}


int32_t PlayerDataMgr::Del(PlayerID player_id)
{
	int32_t ret = 0;

    //如果玩家还在链表里面，那么此时要强制删除该玩家
	PProfile *profile = GetProfile(player_id);
    if (profile == NULL)
    {
        LOG_ERROR(0, "找不到玩家");
        ret = -1;
    }

    _sem_mutex.wlock();
    ret = _player_data_pool.Free(player_id.id);
    _sem_mutex.unwlock();

    return ret;
}

size_t PlayerDataMgr::Ref(PProfile *profile)
{
    return _player_data_pool.Ref(profile);
}

PProfile* PlayerDataMgr::Deref(size_t p)
{
    return (PProfile*) _player_data_pool.Deref(p);
}
