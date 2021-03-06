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
        uint32_t _uid;
        uint32_t _zone_id;
        //uint16_t role_id;
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
        std::string tmp = std::to_string(static_cast<unsigned long long>(_uid))
//        + "|" + std::to_string(static_cast<unsigned long long>(role_id))
        + "|" + std::to_string(static_cast<unsigned long long>(_zone_id));
        return tmp;
    }

    void Init(const PPlayerId &player_id)
    {
    	_uid = player_id.uid();
//        role_id = player_id.role_id();
    	_zone_id = player_id.zone_id();
    }

    void Init(uint32_t uid, uint32_t zone_id)
    {
    	_uid = uid;
		_zone_id = zone_id;
    }

    void ToPlayerId(PPlayerId *player_id) const
    {
    	player_id->set_uid(_uid);
    	player_id->set_zone_id(_zone_id);
//    	player_id->set_role_id(role_id);
    }

}PlayerID;

#endif
