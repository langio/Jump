#ifndef __PLAYER_DATA_MGR_H_
#define __PLAYER_DATA_MGR_H_

#include "comm_util.h"
#include "hash_mem_pool.h"
#include "util/yac_shm.h"
#include "util/yac_sem_mutex.h"
#include <atomic>

using namespace std;
using namespace util;

class PlayerDataMgr
{
    typedef HashMemPool<uint64_t, PProfile> PlayerDataPool;

public:

    static PlayerDataMgr& getInstance()
	{
		static PlayerDataMgr _instance;
		return _instance;
	}

    size_t GetMaxSize(uint32_t bucket, uint32_t node);

    int32_t Init();

    PProfile * Add(PlayerID player_id, bool & exist);

    /*
     * 游戏上层逻辑取在线玩家数据，保证数据已经加载,并且玩家在线
     */
    PProfile *GetProfile(PlayerID player_id);

    int32_t Del(PlayerID player_id);

    size_t Ref(PProfile *profile);

    PProfile* Deref(size_t p);


private:
    PlayerDataMgr():_count(0){}
    PlayerDataMgr(const PlayerDataMgr&);
    PlayerDataMgr& operator=(const PlayerDataMgr&);


    YAC_Shm _shm;
    YAC_SemMutex _sem_mutex;	//读写锁

    PlayerDataPool _player_data_pool;

    atomic_int _count;	//Init只能被调用1次，用count来计算
};


#endif
