#include "util/yac_mmap.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cassert>
#include <unistd.h>

namespace util
{

YAC_Mmap::YAC_Mmap(bool bOwner)
: _bOwner(bOwner)
, _pAddr(NULL)
, _iLength(0)
, _bCreate(false)
{
}

YAC_Mmap::~YAC_Mmap()
{
    if(_bOwner)
    {
        munmap();
    }
}

void YAC_Mmap::mmap(size_t length, int prot, int flags, int fd, off_t offset)
{
    if(_bOwner)
    {
        munmap();
    }
    _pAddr      = ::mmap(NULL, length, prot, flags, fd, offset);
    if(_pAddr == (void*)-1)
    {
        _pAddr = NULL;
        throw YAC_Mmap_Exception("[YAC_Mmap::mmap] mmap error", errno);
    }
    _iLength    = length;
    _bCreate   = false;
}

void YAC_Mmap::mmap(const char *file, size_t length)
{
    assert(length > 0);
    if(_bOwner)
    {
        munmap();
    }
    //注意_bCreate的赋值位置:保证多线程用一个对象的时候也不会有问题
    int fd = open(file, O_CREAT|O_EXCL|O_RDWR, 0666);
    if(fd == -1)
    {
        if(errno != EEXIST)
        {
            throw YAC_Mmap_Exception("[YAC_Mmap::mmap] fopen file '" + string(file) + "' error", errno);
        }
        else
        {
            fd = open(file, O_CREAT|O_RDWR, 0666);
            if(fd == -1)
            {
                throw YAC_Mmap_Exception("[YAC_Mmap::mmap] fopen file '" + string(file) + "' error", errno);
            }
            _bCreate = false;
        }
    }
    else
    {
        _bCreate = true;
    }

    lseek(fd, length-1, SEEK_SET);
    write(fd,"\0",1);

    _pAddr      = ::mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(_pAddr == (void*)-1)
    {
        _pAddr = NULL;
        close(fd);
        throw YAC_Mmap_Exception("[YAC_Mmap::mmap] mmap file '" + string(file) + "' error", errno);
    }
    _iLength    = length;
    if(fd != -1)
    {
       close(fd);
    }
}

void YAC_Mmap::munmap()
{
    if(_pAddr == NULL)
    {
        return;
    }

    int ret = ::munmap(_pAddr, _iLength);
    if(ret == -1)
    {
        throw YAC_Mmap_Exception("[YAC_Mmap::munmap] munmap error", errno);
    }

    _pAddr      = NULL;
    _iLength    = 0;
    _bCreate   = false;
}

void YAC_Mmap::msync(bool bSync)
{
    int ret = 0;
    if(bSync)
    {
        ret = ::msync(_pAddr, _iLength, MS_SYNC | MS_INVALIDATE);
    }
    else
    {
        ret = ::msync(_pAddr, _iLength, MS_ASYNC | MS_INVALIDATE);
    }
    if(ret != 0)
    {
        throw YAC_Mmap_Exception("[YAC_Mmap::msync] msync error", errno);
    }
}

}

