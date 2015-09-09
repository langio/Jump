#ifndef __XC_SEM_MUTEX_H
#define __XC_SEM_MUTEX_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "util/xc_lock.h"

namespace xutil
{
/////////////////////////////////////////////////
/** 
* @file xc_sem_mutex.h 
* @brief 信号量锁类. 
*  
*/              
/////////////////////////////////////////////////

/**
* @brief 信号量锁异常类
*/
struct XC_SemMutex_Exception : public XC_Lock_Exception
{
    XC_SemMutex_Exception(const string &buffer) : XC_Lock_Exception(buffer){};
    XC_SemMutex_Exception(const string &buffer, int err) : XC_Lock_Exception(buffer, err){};
    ~XC_SemMutex_Exception() throw() {};
};

/**
* @brief 进程间锁, 提供两种锁机制:共享锁和排斥锁. 
*  
* 1 对于相同的key, 不同进程初始化时连接到相同的sem上
* 2 采用IPC的信号量实现
* 3 信号量采用了SEM_UNDO参数, 当进程结束时会自动调整信号量
*/
class XC_SemMutex
{
public:
    /**
     * @brief 构造函数
     */
    XC_SemMutex();

    /**
	* @brief 构造函数. 
	*  
    * @param iKey, key
    * @throws XC_SemMutex_Exception
    */
    XC_SemMutex(key_t iKey);

    /**
	* @brief 初始化. 
	*  
    * @param iKey, key
    * @throws XC_SemMutex_Exception
    * @return 无
     */
    void init(key_t iKey);

    /**
	* @brief 获取共享内存Key. 
	*  
    * @return key_t ,共享内存key
    */
    key_t getkey() const {return _semKey;}

    /**
	* @brief 获取共享内存ID. 
	*  
    * @return int ,共享内存Id
    */
    int getid() const   {return _semID;}

    /**
	* @brief 加读锁.
	* 
    *@return int
    */
    int rlock() const;

    /**
	* @brief 解读锁. 
	*  
    * @return int
    */
    int unrlock() const;

    /**
	* @brief 尝试读锁. 
	*  
    * @return bool : 加锁成功则返回false, 否则返回false
    */
    bool tryrlock() const;

    /**
	* @brief 加写锁. 
	*  
    * @return int
    */
    int wlock() const;

    /**
    * @brief 解写锁
    */
    int unwlock() const;

    /**
	* @brief 尝试写锁. 
	*  
    * @throws XC_SemMutex_Exception
    * @return bool : 加锁成功则返回false, 否则返回false
    */
    bool trywlock() const;

    /**
	* @brief 写锁. 
	*  
    * @return int, 0 正确
    */
    int lock() const        {return wlock();};

    /**
    * @brief 解写锁
    */
    int unlock() const      {return unwlock();};

    /**
	* @brief  尝试解锁. 
	*  
    * @throws XC_SemMutex_Exception
    * @return int, 0 正确
    */
    bool trylock() const    {return trywlock();};

protected:
    /**
     * 信号量ID
     */
    int _semID;

    /**
     * 信号量key
     */
    key_t _semKey;
};

}

#endif
