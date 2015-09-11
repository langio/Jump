#ifndef __YAC_AUTOPTR_H
#define __YAC_AUTOPTR_H

#include "util/yac_atomic.h"
#include "util/yac_ex.h"

namespace util
{
///////////////////////////////////////////////////////
/** 
* @file yac_autoptr.h 
* @brief 智能指针类(修改至ICE源码, 智能指针不能相互引用, 否则内存泄漏). 
*  
* @author jarodruan@tencent.com 
*/              
//////////////////////////////////////////////////////

/**
* @brief 空指针异常
*/
struct YAC_AutoPtrNull_Exception : public YAC_Exception
{
    YAC_AutoPtrNull_Exception(const string &buffer) : YAC_Exception(buffer){};
    ~YAC_AutoPtrNull_Exception() throw(){};
};

/**
 *  @brief 智能指针基类.
 *  
 *  所有需要智能指针支持的类都需要从该对象继承，
 *  
 *  内部采用引用计数YAC_Atomic实现，对象可以放在容器中；
 */
template<class  T>
class YAC_HandleBaseT
{
public:

     /** 原子计数类型*/
    typedef T atomic_type;

    /**
     * @brief 复制.
     *
     * @return YAC_HandleBase&
     */
    YAC_HandleBaseT& operator=(const YAC_HandleBaseT&)
    {
        return *this;
    }

    /**
     * @brief 增加计数
     */
    void incRef() { _atomic.inc_fast(); }

    /**
     * @brief 减少计数, 当计数==0时, 且需要删除数据时, 释放对象
     */
    void decRef()
    {
        if(_atomic.dec_and_test() && !_bNoDelete)
        {
            _bNoDelete = true;
            delete this;
        }
    }

    /**
     * @brief 获取计数.
     *
     * @return int 计数值
     */
    int getRef() const        { return _atomic.get(); }

    /**
	 * @brief 设置不自动释放. 
	 *  
     * @param b 是否自动删除,true or false
     */
    void setNoDelete(bool b)  { _bNoDelete = b; }

protected:

    /**
     * @brief 构造函数
     */
    YAC_HandleBaseT() : _atomic(0), _bNoDelete(false)
    {
    }

    /**
     * @brief 拷贝构造
     */
    YAC_HandleBaseT(const YAC_HandleBaseT&) : _atomic(0), _bNoDelete(false)
    {
    }

    /**
     * @brief 析够
     */
    virtual ~YAC_HandleBaseT()
    {
    }

protected:

    /**
     * 计数
     */
    atomic_type   _atomic;

    /**
     * 是否自动删除
     */
    bool        _bNoDelete;
};

template<>
inline void YAC_HandleBaseT<int>::incRef() 
{ 
    //__sync_fetch_and_add(&_atomic,1);
    ++_atomic; 
}

template<> 
inline void YAC_HandleBaseT<int>::decRef()
{
    //int c = __sync_fetch_and_sub(&_atomic, 1);
    //if(c == 1 && !_bNoDelete)
    if(--_atomic == 0 && !_bNoDelete)
    {
        _bNoDelete = true;
        delete this;
    }
}

template<> 
inline int YAC_HandleBaseT<int>::getRef() const        
{ 
    //return __sync_fetch_and_sub(const_cast<volatile int*>(&_atomic), 0);
    return _atomic; 
} 

typedef YAC_HandleBaseT<YAC_Atomic> YAC_HandleBase;

/**
 * @brief 智能指针模板类. 
 *  
 * 可以放在容器中,且线程安全的智能指针. 
 *  
 * 通过它定义智能指针，该智能指针通过引用计数实现， 
 *  
 * 可以放在容器中传递. 
 *  
 * template<typename T> T必须继承于YAC_HandleBase 
 */
template<typename T>
class YAC_AutoPtr
{
public:

    /**
     * 元素类型
     */
    typedef T element_type;

    /**
	 * @brief 用原生指针初始化, 计数+1. 
	 *  
     * @param p
     */
    YAC_AutoPtr(T* p = 0)
    {
        _ptr = p;

        if(_ptr)
        {
            _ptr->incRef();
        }
    }

    /**
	 * @brief 用其他智能指针r的原生指针初始化, 计数+1. 
	 *  
     * @param Y
     * @param r
     */
    template<typename Y>
    YAC_AutoPtr(const YAC_AutoPtr<Y>& r)
    {
        _ptr = r._ptr;

        if(_ptr)
        {
            _ptr->incRef();
        }
    }

    /**
	 * @brief 拷贝构造, 计数+1. 
	 *  
     * @param r
     */
    YAC_AutoPtr(const YAC_AutoPtr& r)
    {
        _ptr = r._ptr;

        if(_ptr)
        {
            _ptr->incRef();
        }
    }

    /**
     * @brief 析构
     */
    ~YAC_AutoPtr()
    {
        if(_ptr)
        {
            _ptr->decRef();
        }
    }

    /**
	 * @brief 赋值, 普通指针. 
	 *  
	 * @param p 
     * @return YAC_AutoPtr&
     */
    YAC_AutoPtr& operator=(T* p)
    {
        if(_ptr != p)
        {
            if(p)
            {
                p->incRef();
            }

            T* ptr = _ptr;
            _ptr = p;

            if(ptr)
            {
                ptr->decRef();
            }
        }
        return *this;
    }

    /**
	 * @brief 赋值, 其他类型智能指针. 
	 *  
     * @param Y
	 * @param r 
     * @return YAC_AutoPtr&
     */
    template<typename Y>
    YAC_AutoPtr& operator=(const YAC_AutoPtr<Y>& r)
    {
        if(_ptr != r._ptr)
        {
            if(r._ptr)
            {
                r._ptr->incRef();
            }

            T* ptr = _ptr;
            _ptr = r._ptr;

            if(ptr)
            {
                ptr->decRef();
            }
        }
        return *this;
    }

    /**
	 * @brief 赋值, 该类型其他执政指针. 
	 *  
	 * @param r 
     * @return YAC_AutoPtr&
     */
    YAC_AutoPtr& operator=(const YAC_AutoPtr& r)
    {
        if(_ptr != r._ptr)
        {
            if(r._ptr)
            {
                r._ptr->incRef();
            }

            T* ptr = _ptr;
            _ptr = r._ptr;

            if(ptr)
            {
                ptr->decRef();
            }
        }
        return *this;
    }

    /**
	 * @brief 将其他类型的智能指针换成当前类型的智能指针. 
	 *  
     * @param Y
	 * @param r 
     * @return YAC_AutoPtr
     */
    template<class Y>
    static YAC_AutoPtr dynamicCast(const YAC_AutoPtr<Y>& r)
    {
        return YAC_AutoPtr(dynamic_cast<T*>(r._ptr));
    }

    /**
	 * @brief 将其他原生类型的指针转换成当前类型的智能指针. 
	 *  
     * @param Y
	 * @param p 
     * @return YAC_AutoPtr
     */
    template<class Y>
    static YAC_AutoPtr dynamicCast(Y* p)
    {
        return YAC_AutoPtr(dynamic_cast<T*>(p));
    }

    /**
     * @brief 获取原生指针.
     *
     * @return T*
     */
    T* get() const
    {
        return _ptr;
    }

    /**
     * @brief 调用.
     *
     * @return T*
     */
    T* operator->() const
    {
        if(!_ptr)
        {
            throwNullHandleException();
        }

        return _ptr;
    }

    /**
     * @brief 引用.
     *
     * @return T&
     */
    T& operator*() const
    {
        if(!_ptr)
        {
            throwNullHandleException();
        }

        return *_ptr;
    }

    /**
     * @brief 是否有效.
     *
     * @return bool
     */
    operator bool() const
    {
        return _ptr ? true : false;
    }

    /**
	 * @brief  交换指针. 
	 *  
     * @param other
     */
    void swap(YAC_AutoPtr& other)
    {
        std::swap(_ptr, other._ptr);
    }

protected:

    /**
     * @brief 抛出异常
     */
    void throwNullHandleException() const;

public:
    T*          _ptr;

};

/**
 * @brief 抛出异常. 
 *  
 * @param T
 * @param file
 * @param line
 */
template<typename T> inline void
YAC_AutoPtr<T>::throwNullHandleException() const
{
    throw YAC_AutoPtrNull_Exception("autoptr null handle error");
}

/**
 * @brief ==判断. 
 *  
 * @param T
 * @param U
 * @param lhs
 * @param rhs
 *
 * @return bool
 */
template<typename T, typename U>
inline bool operator==(const YAC_AutoPtr<T>& lhs, const YAC_AutoPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l == *r;
    }
    else
    {
        return !l && !r;
    }
}

/**
 * @brief 不等于判断. 
 *  
 * @param T
 * @param U
 * @param lhs
 * @param rhs
 *
 * @return bool
 */
template<typename T, typename U>
inline bool operator!=(const YAC_AutoPtr<T>& lhs, const YAC_AutoPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l != *r;
    }
    else
    {
        return l || r;
    }
}

/**
 * @brief 小于判断, 用于放在map等容器中. 
 *  
 * @param T
 * @param U
 * @param lhs
 * @param rhs
 *
 * @return bool
 */
template<typename T, typename U>
inline bool operator<(const YAC_AutoPtr<T>& lhs, const YAC_AutoPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l < *r;
    }
    else
    {
        return !l && r;
    }
}

}

#endif
