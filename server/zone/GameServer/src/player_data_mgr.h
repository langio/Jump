#ifndef __PLAYER_DATA_MGR_H_
#define __PLAYER_DATA_MGR_H_

#include "comm_util.h"
#include "hash_mem_pool.h"

class PlayerDataMgr
{
    typedef HashMemPool<uint64_t, PProfile> PlayerDataPool;

public:

    static PlayerDataMgr& getInstance()
	{
		static PlayerDataMgr _instance;
		return _instance;
	}

    size_t GetMaxSize(unsigned int bucket, unsigned int node);

    int Init(void* mem, uint32_t bucket_num, uint32_t max_node, bool check_header = false);

    PProfile * Add(PlayerID player_id, bool & exist);

    /*
     * 游戏上层逻辑取在线玩家数据，保证数据已经加载,并且玩家在线
     */
    PProfile *GetProfile(PlayerID player_id);

    PProfile * Get(PlayerID player_id, bool need_login);
    int Del(PlayerID player_id);

    size_t Ref(PProfile *profile);

    PProfile* Deref(size_t p);


private:
    PlayerDataMgr(){}
    PlayerDataMgr(const PlayerDataMgr&);
    PlayerDataMgr& operator=(const PlayerDataMgr&);


    PlayerDataPool _player_data_pool;
};


#endif
