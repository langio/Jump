#ifndef __SERVER_ENV_H_
#define __SERVER_ENV_H_

#include "send_util.h"
#include "redox.hpp"

using namespace redox;

class ServerEnv
{
public:
    static ServerEnv& getInstance()
    {
        static ServerEnv env;
        return env;
    }

    bool init();

    Redox& getRdx(){ return _rdx;}

private:
    ServerEnv(){}
    ServerEnv(const ServerEnv&);
    ServerEnv& operator=(const ServerEnv&);

    Redox _rdx;
};

#endif

