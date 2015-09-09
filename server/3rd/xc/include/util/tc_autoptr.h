#ifndef __XC_AUTOPTR_H
#define __XC_AUTOPTR_H

#include "util/tc_atomic.h"
#include "util/tc_ex.h"

namespace xutil
{
///////////////////////////////////////////////////////
/** 
* @file tc_autoptr.h 
* @brief 智能指针类(修改至ICE源码, 智能指针不能相互引用, 否则内存泄漏). 
*  
*/              
//////////////////////////////////////////////////////

/**
* @brief 空指针异常
*/
struct XC_AutoPtrNull_Exception : public XC_Exception
{
    XC_AutoPtrNull_Exception(const string &buffer) : XC_Exception(buffer){};
    ~XC_AutoPtrNull_Exception() throw(){};
};

/**
 *  @brief 智能指针基类.
 *  
 *  所有需要智能指针支持的类都需要从该对象继承，
 *  
 *  内部采用引用计数XC_Atomic实现，对象可以放在容器中；
 */
template<class  T>
class XC_HandleBaseT
{
public:

     /** 原子计数类型*/
    typedef T atomic_type;

    /**
     * @brief 复制.
     *
     * @return XC_HandleBase&
     */
    XC_HandleBaseT& operator=(const XC_HandleBaseT&)
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
    XC_HandleBaseT() : _atomic(0), _bNoDelete(false)
    {
    }

    /**
     * @brief 拷贝构造
     */
    XC_HandleBaseT(const XC_HandleBaseT&) : _atomic(0), _bNoDelete(false)
    {
    }

    /**
     * @brief 析够
     */
    virtual ~XC_HandleBaseT()
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
inline void XC_HandleBaseT<int>::incRef() 
{ 
    //__sync_fetch_and_add(&_atomic,1);
    ++_atomic; 
}

template<> 
inline void XC_HandleBaseT<int>::decRef()
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
inline int XC_HandleBaseT<int>::getRef() const        
{ 
    //return __sync_fetch_and_sub(const_cast<volatile int*>(&_atomic), 0);
    return _atomic; 
} 

typedef XC_HandleBaseT<XC_Atomic> XC_HandleBase;

/**
 * @brief 智能指针模板类. 
 *  
 * 可以放在容器中,且线程安全的智能指针. 
 *  
 * 通过它定义智能指针，该智能指针通过引用计数实现， 
 *  
 * 可以放在容器中传递. 
 *  
 * template<typename T> T必须继承于XC_HandleBase 
 */
template<typename T>
class XC_AutoPtr
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
    XC_AutoPtr(T* p = 0)
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
    XC_AutoPtr(const XC_AutoPtr<Y>& r)
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
    XC_AutoPtr(const XC_AutoPtr& r)
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
    ~XC_AutoPtr()
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
     * @return XC_AutoPtr&
     */
    XC_AutoPtr& operator=(T* p)
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
     * @return XC_AutoPtr&
     */
    template<typename Y>
    XC_AutoPtr& operator=(const XC_AutoPtr<Y>& r)
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
     * @return XC_AutoPtr&
     */
    XC_AutoPtr& operator=(const XC_AutoPtr& r)
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
     * @return XC_AutoPtr
     */
    template<class Y>
    static XC_AutoPtr dynamicCast(const XC_AutoPtr<Y>& r)
    {
        return XC_AutoPtr(dynamic_cast<T*>(r._ptr));
    }

    /**
	 * @brief 将其他原生类型的指针转换成当前类型的智能指针. 
	 *  
     * @param Y
	 * @param p 
     * @return XC_AutoPtr
     */
    template<class Y>
    static XC_AutoPtr dynamicCast(Y* p)
    {
        return XC_AutoPtr(dynamic_cast<T*>(p));
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
    void swap(XC_AutoPtr& other)
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
XC_AutoPtr<T>::throwNullHandleException() const
{
    throw XC_AutoPtrNull_Exception("autoptr null handle error");
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
inline bool operator==(const XC_AutoPtr<T>& lhs, const XC_AutoPtr<U>& rhs)
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
inline bool operator!=(const XC_AutoPtr<T>& lhs, const XC_AutoPtr<U>& rhs)
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
inline bool operator<(const XC_AutoPtr<T>& lhs, const XC_AutoPtr<U>& rhs)
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
