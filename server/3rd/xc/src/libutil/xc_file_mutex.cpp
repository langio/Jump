#include <cassert>
#include <sys/file.h>
#include "util/xc_file_mutex.h"
#include "util/xc_lock.h"

namespace xutil
{
	
XC_FileMutex::XC_FileMutex() 
{
	_fd = -1;
}

XC_FileMutex::~XC_FileMutex()
{
	unlock();
}

void XC_FileMutex::init(const std::string& filename)
{
	if (filename.empty())
	{
		 throw XC_FileMutex_Exception("[XC_FileMutex::init] filename is empty");
	}

	if(_fd > 0)
	{
		close(_fd);
	}
	_fd = open(filename.c_str(), O_RDWR|O_CREAT, 0660);
	if (_fd < 0) 
	{
		throw XC_FileMutex_Exception("[XC_FileMutex::init] open '" + filename + "' error", errno);
	}
}

int XC_FileMutex::rlock()
{
	assert(_fd > 0);

	return lock(_fd, F_SETLKW, F_RDLCK, 0, 0, 0);
}

int XC_FileMutex::unrlock() 
{
	return unlock();
}

bool XC_FileMutex::tryrlock()
{
	return hasLock(_fd, F_RDLCK, 0, 0, 0);
}

int XC_FileMutex::wlock()
{
	assert(_fd > 0);

	return lock(_fd, F_SETLKW, F_WRLCK, 0, 0, 0);
}

int XC_FileMutex::unwlock() 
{
	return unlock();
}

bool XC_FileMutex::trywlock()
{
	return hasLock(_fd, F_WRLCK, 0, 0, 0);
}

int XC_FileMutex::unlock()
{
	return lock(_fd, F_SETLK, F_UNLCK, 0, 0, 0);
}

int XC_FileMutex::lock(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	struct flock lock;
	lock.l_type 	= type;
	lock.l_start 	= offset;
	lock.l_whence 	= whence;
	lock.l_len 		= len;

	return fcntl(fd, cmd, &lock);
}

bool XC_FileMutex::hasLock(int fd, int type, off_t offset, int whence, off_t len)
{
	struct flock lock;
	lock.l_type 	= type;
	lock.l_start 	= offset;
	lock.l_whence 	= whence;
	lock.l_len 		= len;

	if(fcntl(fd, F_GETLK, &lock)  == -1)
	{
		throw XC_FileMutex_Exception("[XC_FileMutex::hasLock] fcntl error", errno);
    }

	if(lock.l_type == F_UNLCK)
	{
		return false;
	}
	
	return true;
}

}

