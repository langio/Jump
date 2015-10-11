#ifndef __SERVER_ENV_H_
#define __SERVER_ENV_H_

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

private:
    ServerEnv(){}
    ServerEnv(const ServerEnv&);
    ServerEnv& operator=(const ServerEnv&);

    Redox _rdx;
};

#endif
