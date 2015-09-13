#include <cassert>
#include <sys/file.h>
#include "util/yac_file_mutex.h"
#include "util/yac_lock.h"
#include <unistd.h>

namespace util
{

YAC_FileMutex::YAC_FileMutex()
{
	_fd = -1;
}

YAC_FileMutex::~YAC_FileMutex()
{
	unlock();
}

void YAC_FileMutex::init(const std::string& filename)
{
	if (filename.empty())
	{
		 throw YAC_FileMutex_Exception("[YAC_FileMutex::init] filename is empty");
	}

	if(_fd > 0)
	{
		close(_fd);
	}
	_fd = open(filename.c_str(), O_RDWR|O_CREAT, 0660);
	if (_fd < 0)
	{
		throw YAC_FileMutex_Exception("[YAC_FileMutex::init] open '" + filename + "' error", errno);
	}
}

int YAC_FileMutex::rlock()
{
	assert(_fd > 0);

	return lock(_fd, F_SETLKW, F_RDLCK, 0, 0, 0);
}

int YAC_FileMutex::unrlock()
{
	return unlock();
}

bool YAC_FileMutex::tryrlock()
{
	return hasLock(_fd, F_RDLCK, 0, 0, 0);
}

int YAC_FileMutex::wlock()
{
	assert(_fd > 0);

	return lock(_fd, F_SETLKW, F_WRLCK, 0, 0, 0);
}

int YAC_FileMutex::unwlock()
{
	return unlock();
}

bool YAC_FileMutex::trywlock()
{
	return hasLock(_fd, F_WRLCK, 0, 0, 0);
}

int YAC_FileMutex::unlock()
{
	return lock(_fd, F_SETLK, F_UNLCK, 0, 0, 0);
}

int YAC_FileMutex::lock(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	struct flock lock;
	lock.l_type 	= type;
	lock.l_start 	= offset;
	lock.l_whence 	= whence;
	lock.l_len 		= len;

	return fcntl(fd, cmd, &lock);
}

bool YAC_FileMutex::hasLock(int fd, int type, off_t offset, int whence, off_t len)
{
	struct flock lock;
	lock.l_type 	= type;
	lock.l_start 	= offset;
	lock.l_whence 	= whence;
	lock.l_len 		= len;

	if(fcntl(fd, F_GETLK, &lock)  == -1)
	{
		throw YAC_FileMutex_Exception("[YAC_FileMutex::hasLock] fcntl error", errno);
    }

	if(lock.l_type == F_UNLCK)
	{
		return false;
	}

	return true;
}

}

