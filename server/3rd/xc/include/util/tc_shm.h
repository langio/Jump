#ifndef	__XC_SHM_H__
#define __XC_SHM_H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "util/tc_ex.h"

namespace xutil
{
/////////////////////////////////////////////////
/** 
 * @file  tc_shm.h 
 * @brief  共享内存封装类. 
 *  
 */
/////////////////////////////////////////////////


/**
* @brief 共享内存异常类.
*/
struct XC_Shm_Exception : public XC_Exception
{
    XC_Shm_Exception(const string &buffer) : XC_Exception(buffer){};
    XC_Shm_Exception(const string &buffer, int err) : XC_Exception(buffer, err){};
    ~XC_Shm_Exception() throw() {};
};

/** 
* @brief  共享内存连接类，说明: 
* 1 用于连接共享内存, 共享内存的权限是 0666 
* 2 _bOwner=false: 析够时不detach共享内存 
* 3 _bOwner=true: 析够时detach共享内存
*/
class XC_Shm
{
public:

    /**
	* @brief 构造函数.
	*  
	* @param bOwner  是否拥有共享内存,默认为false 
    */
    XC_Shm(bool bOwner = false) : _bOwner(bOwner), _pshm(NULL) {}

    /**
	* @brief 构造函数. 
	*  
    * @param iShmSize 共享内存大小
    * @param iKey     共享内存Key
    * @throws         XC_Shm_Exception
    */
    XC_Shm(size_t iShmSize, key_t iKey, bool bOwner = false);

    /**
    * @brief 析构函数.
    */
    ~XC_Shm();

    /**
	* @brief 初始化. 
	*  
    * @param iShmSize   共享内存大小
    * @param iKey       共享内存Key
    * @param bOwner     是否拥有共享内存
    * @throws           XC_Shm_Exception
    * @return Ξ
    */
    void init(size_t iShmSize, key_t iKey, bool bOwner = false);

	/** 
	* @brief 判断共享内存的类型，生成的共享内存,还是连接上的共享内存
	* 如果是生成的共享内存,此时可以根据需要做初始化 
	*  
    * @return  true,生成共享内存; false, 连接上的共享内存
    */
    bool iscreate()     {return _bCreate;}

    /**
	* @brief  获取共享内存的指针.
	*  
    * @return   void* 共享内存指针
    */
    void *getPointer() {return _pshm;}

    /**
	* @brief  获取共享内存Key.
	*  
    * @return key_t* ,共享内存key
    */
    key_t getkey()  {return _shmKey;}

    /**
	* @brief  获取共享内存ID.
	* 
    * @return int ,共享内存Id
    */
    int getid()     {return _shemID;}

    /**
	*  @brief  获取共享内存大小.
	*  
    * return size_t,共享内存大小
    */
    size_t size()   {return _shmSize;}

	/** 
	*  @brief 解除共享内存，在当前进程中解除共享内存
    * 共享内存在当前进程中无效
    * @return int
    */
    int detach();

	/** 
	 *  @brief 删除共享内存.
	 * 
     * 完全删除共享内存
     */
    int del();

protected:

    /**
     * 是否拥有共享内存
     */
    bool            _bOwner;

    /**
    * 共享内存大小
    */
    size_t          _shmSize;

    /**
    * 共享内存key
    */
    key_t           _shmKey;

    /**
    * 是否是生成的共享内存
    */
    bool            _bCreate;

    /**
    * 共享内存
    */
    void            *_pshm;

    /**
    * 共享内存id
    */
    int             _shemID;

};

}

#endif
