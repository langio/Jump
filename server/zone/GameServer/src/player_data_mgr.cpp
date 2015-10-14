#include "player_data_mgr.h"
#include "comm_def.h"


size_t PlayerDataMgr::GetMaxSize(unsigned int bucket, unsigned int node)
{
    return _player_data_pool.GetMaxSize(bucket, node);
}

int PlayerDataMgr::Init(void* mem, uint32_t bucket_num, uint32_t max_node, bool check_header)
{
    return _player_data_pool.Init(mem, bucket_num, max_node, check_header);
}


PProfile * PlayerDataMgr::Add(PlayerID player_id, bool & exist)
{
    exist = true;
    PProfile *profile = _player_data_pool.Get(player_id.id);
    if (profile != NULL)
    {
        return profile;
    }
    exist = false;
    return (PProfile *) _player_data_pool.Alloc(player_id.id);
}

/*
 * 游戏上层逻辑取在线玩家数据，保证数据已经加载,并且玩家在线
 */
PProfile *PlayerDataMgr::GetProfile(PlayerID player_id)
{
	PProfile *profile = _player_data_pool.Get(player_id.id);
    if (profile == NULL)
    {
        return NULL;
    }
    return profile;
}

PProfile * PlayerDataMgr::Get(PlayerID player_id, bool need_login)
{
	PProfile *profile = _player_data_pool.Get(player_id.id);
    if (profile == NULL)
    {
        return NULL;
    }
    return profile;
}

int PlayerDataMgr::Del(PlayerID player_id)
{
    //如果玩家还在链表里面，那么此时要强制删除该玩家
	PProfile *profile = Get(player_id, false);
    if (profile == NULL)
    {
        LOG_ERROR(0, "找不到玩家");
        return -1;
    }


    return _player_data_pool.Free(player_id.id);
}

size_t PlayerDataMgr::Ref(PProfile *profile)
{
    return _player_data_pool.Ref(profile);
}

PProfile* PlayerDataMgr::Deref(size_t p)
{
    return (PProfile*) _player_data_pool.Deref(p);
}
