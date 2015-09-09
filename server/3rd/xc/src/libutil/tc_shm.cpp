#include "util/tc_shm.h"
#include <cassert>
#include <errno.h>

namespace xutil
{

XC_Shm::XC_Shm(size_t iShmSize, key_t iKey, bool bOwner)
{
    init(iShmSize, iKey, bOwner);
}

XC_Shm::~XC_Shm()
{
    if(_bOwner)
    {
        detach();
    }
}

void XC_Shm::init(size_t iShmSize, key_t iKey, bool bOwner)
{
    assert(_pshm == NULL);

    _bOwner     = bOwner;

    //注意_bCreate的赋值位置:保证多线程用一个对象的时候也不会有问题
    //试图创建
    if ((_shemID = shmget(iKey, iShmSize, IPC_CREAT | IPC_EXCL | 0666)) < 0)
    {
        _bCreate = false;
        //有可能是已经存在同样的key_shm,则试图连接
        if ((_shemID = shmget(iKey, iShmSize, 0666)) < 0)
        {
            throw XC_Shm_Exception("[XC_Shm::init()] shmget error", errno);
        }
    }
    else
    {
        _bCreate    = true;
    }

    //try to access shm
    if ((_pshm = shmat(_shemID, NULL, 0)) == (char *) -1)
    {
        throw XC_Shm_Exception("[XC_Shm::init()] shmat error", errno);
    }

    _shmSize = iShmSize;
    _shmKey = iKey;
}

int XC_Shm::detach()
{
    int iRetCode = 0;
    if(_pshm != NULL)
    {
        iRetCode = shmdt(_pshm);

        _pshm = NULL;
    }

    return iRetCode;
}

int XC_Shm::del()
{
    int iRetCode = 0;
    if(_pshm != NULL)
    {
        iRetCode = shmctl(_shemID, IPC_RMID, 0);

        _pshm = NULL;
    }

    return iRetCode;
}

}
