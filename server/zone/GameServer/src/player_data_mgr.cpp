#include "player_data_mgr.h"
#include "comm_def.h"
#include "ini_parse.h"

void* GetShm(key_t shm_key, size_t shm_size, bool &is_exist)
{
	LOG_DEBUG(0, "开始获取共享内存, shm_key=0x%08x, shm_size=%zu", shm_key, shm_size);
	is_exist = true;
	int shm_id = shmget(shm_key, shm_size, 0644);
	if (shm_id == -1)
	{
		is_exist = false;
		shm_id = shmget(shm_key, shm_size, 0644 | IPC_CREAT | IPC_EXCL);
		if (shm_id == -1)
		{
			//既然获取不了该共享内存，但是又不能创建共享内存，那么只能返回错误了
			LOG_ERROR(0, "获取共享内存失败:%s", strerror(errno));
			return NULL;
		}
	}

	//在连接这块内存之前,先做一次检查,看看是否有其它进程在用
	struct shmid_ds ds_buf;
	if (shmctl(shm_id, IPC_STAT, &ds_buf) < 0)
	{
		LOG_DEBUG(0, "获取共享内存的信息失败:%s", strerror(errno));
	    return NULL;
	}
	if (ds_buf.shm_nattch > 0)
	{
		LOG_DEBUG(0, "共享内存目前有%lu个进程正在使用,创建内存的pid=%d,最后操作内存的pid=%d", ds_buf.shm_nattch, ds_buf.shm_cpid, ds_buf.shm_lpid);
	    return NULL;
	}

	//连接共享内存
	void *mem_begin = shmat(shm_id, NULL, 0);
	if (mem_begin == (void *) -1)
	{
		LOG_DEBUG(0, "连接共享内存失败:%s", strerror(errno));
		return NULL;
	}

	LOG_DEBUG(0, "获取共享内存成功, shm_key=0x%08x, shm_size=%zu, is_exit:%d", shm_key, shm_size, is_exist);

	return mem_begin;
}

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

		bool check_header = false;
		void* mem = NULL;

		uint32_t player_max_bucket = ini.GetUIntValue("RUN_ARGS", "player_max_bucket");
		uint32_t player_max_node = ini.GetUIntValue("RUN_ARGS", "player_max_node");
		size_t max_shm_size = GetMaxSize(player_max_bucket, player_max_node);
//		uint32_t shm_key = ini.GetUIntValue("SHM_KEY", "palyer_data_mgr_key");
//		_shm.init(max_shm_size, shm_key);
//		check_header = !_shm.iscreate();
//		mem = _shm.getPointer();

		//不使用共享内存,与其所带来的收益(服务重启不用加载玩家数据)相比，其维护成本(数据一致性)可能会更高
		mem = new char[max_shm_size];
		check_header = false;

		ret = (int32_t)_player_data_pool.Init(mem, player_max_bucket, player_max_node, check_header);

		LOG_DEBUG(0, "_player_data_pool.Init|%d|%u|%u|%p", ret, player_max_bucket, player_max_node, mem);

		uint32_t palyer_data_mutex_key = ini.GetUIntValue("SHM_KEY", "palyer_data_mutex_key");
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

    PProfile *tmp = _player_data_pool.Alloc(player_id.id);
    profile = new(tmp) PProfile;

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

    //释放protobuf自己管理的资源
    profile->~PProfile();

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
