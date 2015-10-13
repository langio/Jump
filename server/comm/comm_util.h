#ifndef __COMM_UTIL_H_
#define __COMM_UTIL_H_

//此文件中定义协议相关的方法、宏等

#include "profile.pb.h"

using namespace S;

typedef union PlayerID
{
    uint64_t id;
    struct
    {
        uint32_t uin;
        uint16_t role_id;
        uint16_t svr_id;
    };

    bool operator==(const PlayerID& lhs) const
    {
        return lhs.id == id;
    }

    bool operator<(const PlayerID& lhs) const
    {
        return id < lhs.id;
    }

    const std::string ToString()
    {
        std::string tmp = std::to_string(static_cast<unsigned long long>(uin))
        + "|" + std::to_string(static_cast<unsigned long long>(role_id))
        + "|" + std::to_string(static_cast<unsigned long long>(svr_id));
        return tmp;
    }

    void Init(const PPlayerId &player_id)
    {
        uin = player_id.uin();
        role_id = player_id.role_id();
        svr_id = player_id.svr_id();
    }

    void ToPlayerId(const PPlayerId *player_id) const
    {
    	player_id->set_uin(uin);
    	player_id->set_svr_id(svr_id);
    	player_id->set_role_id(role_id);
    }

}PlayerID;

#endif
