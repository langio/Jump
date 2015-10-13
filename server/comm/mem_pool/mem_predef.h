/******************************************************************************************
Description         : 拷贝自sail的zenlib，原名是zen_mmappredef，由于这个实现实际上只是基于已分配内存的，
                                                          而不管是不是共享内存，所以改了一下名字。
Modification History:
******************************************************************************************/
#ifndef _MEM_PREDEF_H_
#define _MEM_PREDEF_H_

#include <iostream>
#include <cassert>

//素数表
#define __MEM_PRIME_LIST_BODY {          \
    53ul,         97ul,          193ul,         389ul,        769ul,        1543ul,  \
    3079ul,       6151ul,        12289ul,       21101ul,      31957ul,      42953ul, \
    53657ul,      63761ul,       73009ul,       83207ul,      93199ul,      101051ul, \
    111187ul,     120851ul,      130699ul,      141283ul,     150383ul,     161503ul, \
    170899ul,     181639ul,      191657ul,      203431ul,     252817ul,     310823ul, \
    408869ul,     510989ul,      603881ul,      710531ul,     803171ul,     909613ul, \
    1008853ul,    1572869ul,     3145739ul,     6291469ul,    12582917ul,   25165843ul, \
    50331653ul,   100663319ul,   201326611ul,   402653189ul,  805306457ul,  1610612741ul,\
    3221225473ul, 4294967291ul  \
};



//定义Hash函数的函数对象(仿函数),不是我非要当"特立独行的猪",非要抄而不用STL,hashtable还不是标准STL
template <class keytpe> struct mem_hash {};

inline size_t _mem_hash_string(const char* str)
{
    unsigned long hashval = 0;
    for ( ; *str; ++str)
        hashval = 5*hashval + *str;
    return size_t(hashval);
}

template<> struct mem_hash<char*>
{
    size_t operator()(const char* str) const { return _mem_hash_string(str); }
};

template<> struct mem_hash<const char*>
{
    size_t operator()(const char* str) const { return _mem_hash_string(str); }
};

template<> struct mem_hash<std::string>
{
    size_t operator()(const std::string &str) const { return _mem_hash_string(str.c_str()); }
};

template<> struct mem_hash<const std::string>
{
    size_t operator()(const std::string &str) const { return _mem_hash_string(str.c_str()); }
};

template<> struct mem_hash<char> {
    size_t operator()(char x) const { return static_cast<size_t>(x); }
};
template<> struct mem_hash<unsigned char> {
    size_t operator()(unsigned char x) const { return static_cast<size_t>(x); }
};

template<> struct mem_hash<short> {
    size_t operator()(short x) const { return static_cast<size_t>(x); }
};
template<> struct mem_hash<unsigned short> {
    size_t operator()(unsigned short x) const { return static_cast<size_t>(x); }
};
template<> struct mem_hash<int> {
    size_t operator()(int x) const { return static_cast<size_t>(x); }
};
template<> struct mem_hash<unsigned int> {
    size_t operator()(unsigned int x) const { return static_cast<size_t>(x); }
};
template<> struct mem_hash<long> {
    size_t operator()(long x) const { return static_cast<size_t>(x); }
};
template<> struct mem_hash<unsigned long> {
    size_t operator()(unsigned long x) const { return static_cast<size_t>(x); }
};

//identity也不是标准STL,偷,偷,偷, identity其实就是萃取自己
template <class T> struct mem_identity
{
    const T& operator()(const T& x) const { return x; }
};

//
template <class _Pair> struct mem_select1st
{
    typename _Pair::first_type&
        operator()(_Pair& __x) const
    { return __x.first; }

    const typename _Pair::first_type&
        operator()(const _Pair& __x) const
    { return __x.first; }
};

//
template <class _Pair> struct mem_select2st
{
    typename _Pair::second_type&
        operator()(_Pair& __x) const
    { return __x.second; }

    const typename _Pair::second_type&
        operator()(const _Pair& __x) const
    { return __x.second; }
};

//
class _memory_base
{
public:

    //空序号指针标示
    static const size_t  _INVALID_POINT = static_cast<size_t>(-1);

protected:


    //内存基础地质
    char     *mem_base_;

    _memory_base():
        mem_base_(NULL)
    {
    };

    _memory_base(char * basepoint):
        mem_base_(basepoint)
    {
    };

    ~_memory_base()
    {
    }

    //dump所有的内存数据
    void dump()
    {
    }
};


class _mem_list_index
{
public:
    size_t  idx_next_;
    size_t  idx_prev_;

    _mem_list_index():
        idx_next_(_memory_base::_INVALID_POINT),
        idx_prev_(_memory_base::_INVALID_POINT)
    {
    }
    _mem_list_index(const size_t& nxt,const size_t& prv):
        idx_next_(nxt),
        idx_prev_(prv)
    {
    }
    ~_mem_list_index()
    {
    }
};

#endif // _MEM_PREDEF_H_
