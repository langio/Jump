#ifndef _YAC_LOCK_H
#define _YAC_LOCK_H

#include <string>
#include <stdexcept>
#include <cerrno>
#include "util/yac_ex.h"

using namespace std;

namespace util
{
/////////////////////////////////////////////////
/**
 * @file yac_lock.h 
 * @brief  锁类 
 * @author  jarodruan@tencent.com 
 */           
/////////////////////////////////////////////////


/**
* @brief  锁异常
*/
struct YAC_Lock_Exception : public YAC_Exception
{
    YAC_Lock_Exception(const string &buffer) : YAC_Exception(buffer){};
    YAC_Lock_Exception(const string &buffer, int err) : YAC_Exception(buffer, err){};
    ~YAC_Lock_Exception() throw() {};
};

/**
 * @brief  锁模板类其他具体锁配合使用，
 * 构造时候加锁，析够的时候解锁
 */
template <typename T>
class YAC_LockT
{
public:

    /**
	 * @brief  构造函数，构造时枷锁
	 *  
     * @param mutex 锁对象
     */
    YAC_LockT(const T& mutex) : _mutex(mutex)
    {
        _mutex.lock();
        _acquired = true;
    }

    /**
     * @brief  析构，析构时解锁
     */
    virtual ~YAC_LockT()
    {
        if (_acquired)
        {
            _mutex.unlock();
        }
    }

    /**
     * @brief  上锁, 如果已经上锁,则抛出异常
     */
    void acquire() const
    {
        if (_acquired)
        {
            throw YAC_Lock_Exception("thread has locked!");
        }
        _mutex.lock();
        _acquired = true;
    }

    /**
     * @brief  尝试上锁.
     *
     * @return  成功返回true，否则返回false
     */
    bool tryAcquire() const
    {
        _acquired = _mutex.tryLock();
        return _acquired;
    }

    /**
     * @brief  释放锁, 如果没有上过锁, 则抛出异常
     */
    void release() const
    {
        if (!_acquired)
        {
            throw YAC_Lock_Exception("thread hasn't been locked!");
        }
        _mutex.unlock();
        _acquired = false;
    }

    /**
     * @brief  是否已经上锁.
     *
     * @return  返回true已经上锁，否则返回false
     */
    bool acquired() const
    {
        return _acquired;
    }

protected:

    /**
	 * @brief 构造函数
	 * 用于锁尝试操作，与YAC_LockT相似
	 *  
     */
    YAC_LockT(const T& mutex, bool) : _mutex(mutex)
    {
        _acquired = _mutex.tryLock();
    }

private:

    // Not implemented; prevents accidental use.
    YAC_LockT(const YAC_LockT&);
    YAC_LockT& operator=(const YAC_LockT&);

protected:
    /**
     * 锁对象
     */
    const T&        _mutex;

    /**
     * 是否已经上锁
     */
    mutable bool _acquired;
};

/**
 * @brief  尝试上锁
 */
template <typename T>
class YAC_TryLockT : public YAC_LockT<T>
{
public:

    YAC_TryLockT(const T& mutex) : YAC_LockT<T>(mutex, true)
    {
    }
};

/**
 * @brief  空锁, 不做任何锁动作
 */
class YAC_EmptyMutex
{
public:
    /**
	* @brief  写锁.
	*  
    * @return int, 0 正确
    */
    int lock()  const   {return 0;}

    /**
    * @brief  解写锁
    */
    int unlock() const  {return 0;}

    /**
	* @brief  尝试解锁. 
	*  
    * @return int, 0 正确
    */
    bool trylock() const {return true;}
};

};
#endif

