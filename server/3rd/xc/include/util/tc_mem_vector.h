#ifndef	__XC_MEM_VECTOR_H__
#define __XC_MEM_VECTOR_H__

#include "util/tc_ex.h"
#include <sstream>
#include <string.h>

namespace xutil
{
/////////////////////////////////////////////////
/** 
* @file tc_mem_vector.h 
* @brief  共享内存数组类. 
*  
*/            
/////////////////////////////////////////////////

/**
* @brief 异常类
*/
struct XC_MemVectorException : public XC_Exception
{
	XC_MemVectorException(const string &buffer) : XC_Exception(buffer){};
    XC_MemVectorException(const string &buffer, int err) : XC_Exception(buffer, err){};
    ~XC_MemVectorException() throw(){};
};

/**
* @brief 共享内存随机访问数据块.
*  
* 每块数据快大小相等，模板对象T只能是简单的数据类型 ，
*  
* 需要具备bit-copy的语义才行 
*/
template<typename T>
class XC_MemVector
{
public:

    /**
    * @brief 构造函数
    */
    XC_MemVector() : _pHead(NULL) ,_pData(NULL)
    {
    }

    /**
     * @brief 
     * @param mv
     */
    XC_MemVector(const XC_MemVector<T> &mv)
    : _pHead(mv._pHead) ,_pData(mv._pData)
    {

    }

    /**
     * @brief 
     * @param mv
     *
     * @return bool
     */
    bool operator==(const XC_MemVector<T> &mv)
    {
        return _pHead == mv._pHead && _pData == mv._pData;
    }

    /**
     * @brief 
     * @param mv
     *
     * @return bool
     */
    bool operator!=(const XC_MemVector<T> &mv)
    {
        return _pHead != mv._pHead || _pData != mv._pData;
    }

    /**
     * @brief 计算需要的内存空间
	 * @param iCount  数据个数 
     * @return size_t  内存空间大小
     */
    static size_t calcMemSize(size_t iCount)
    {
        return sizeof(T) * iCount + sizeof(tagMemQueueHead);
    }

    /**
    * @brief 初始化
    * @param pAddr       指令队列空间的指针
    * @param iSize       空间的指针
    * @param iBlockSize 每个block的大小
    */
    void create(void *pAddr, size_t iSize);

    /**
    * @brief 连接上队列
    * @param pAddr 指令队列空间的指针
    */
    void connect(void *pAddr) { init(pAddr); assert(_pHead->_iBlockSize == sizeof(T)); }

    /**
    * @brief 元素个数
    * @return size_t，队列长度
    */
    size_t size() { return _pHead->_iBlockCount; }

    /**
    * @brief 共享内存长度
    * @return size_t : 共享内存长度
    */
    size_t getMemSize() { return _pHead->_iSize; }

    /**
     * @brief 重建
     */
    void clear();

    /**
     * @brief 
     * @param simple
     *
     * @return string
     */
    string desc() const;

    /**
     * @brief 迭代器
     */
    class XC_MemVectorIterator : public std::iterator<std::random_access_iterator_tag, T>
    {
    public:
        /**
         * @brief 构造函数
         * @param pmv
         * @param iIndex
         */
        XC_MemVectorIterator(XC_MemVector *pmv, size_t iIndex) : _pmv(pmv), _iIndex(iIndex)
        {
        }

        /**
         * @brief 前置++
         *
         * @return XC_MemVectorIterator&
         */
        XC_MemVectorIterator& operator++()
        {
            ++_iIndex;
            return *this;
        }

        /**
         * @brief 后置++
         */
        XC_MemVectorIterator operator++(int)
        {
            XC_MemVectorIterator tmp = *this;

            ++_iIndex;
            return tmp;
        }

        /**
         * @brief 
         * @param mv
         *
         * @return XC_MemVectorIterator
         */
        bool operator==(const XC_MemVectorIterator& mv)
        {
            return _iIndex == mv._iIndex && _pmv == mv._pmv;
        }

        /**
         * @brief 
         * @param mv
         *
         * @return XC_MemVectorIterator
         */
        bool operator!=(const XC_MemVectorIterator& mv)
        {
            return _iIndex != mv._iIndex || _pmv != mv._pmv;
        }

        /**
         * @brief
         *
         * @return T&
         */
        T& operator*() const { return (*_pmv)[_iIndex]; }

        /**
         * @brief
         *
         * @return T*
         */
        T* operator->() const { return &(*_pmv)[_iIndex]; }

    private:
        /**
         *
         */
        XC_MemVector    *_pmv;

        /**
         *
         */
        size_t          _iIndex;
    };

    typedef XC_MemVectorIterator iterator;

    /**
     *
     *
     * @return XC_MemVectorIterator
     */
    XC_MemVectorIterator begin()	{ return XC_MemVectorIterator(this, 0); }

    /**
     *
     *
     * @return XC_MemVectorIterator
     */
    XC_MemVectorIterator end()		{ return XC_MemVectorIterator(this, _pHead->_iBlockCount); }

    /**
     * @brief 获取数据
     * @param pData
     * @param iDataLen
     */
    T& operator[](size_t iIndex)
    {
        if(iIndex >= _pHead->_iBlockCount)
        {
			ostringstream s;
			s << string("[XC_MemVector::get] index beyond : index = ") << iIndex << " > " << _pHead->_iBlockCount;

            throw XC_MemVectorException(s.str());
        }

        return *(T*)((char*)_pData + iIndex * _pHead->_iBlockSize);
    }

    /**
     * @brief 获取头地址
     *
     * @return void*
     */
    void *getAddr() { return (void*)_pHead; }

    /**
    *  @brief 队列控制结构
    */
    struct tagMemQueueHead
    {
        size_t _iSize;          //内存大小
        size_t _iBlockCount;    //元素个数
        size_t _iBlockSize;     //区块大小
    }__attribute__((packed));

protected:

    /**
     * @brief
     * @param pAddr
     */
    void init(void *pAddr)
    {
        _pHead = static_cast<tagMemQueueHead*>(pAddr);
        _pData = (char*)_pHead + sizeof(tagMemQueueHead);
    }


    /**
    * 队列控制快
    */
    tagMemQueueHead *_pHead;

    /**
    * 共享内存地址
    */
    void            *_pData;
};

template<typename T>
void XC_MemVector<T>::create(void *pAddr, size_t iSize)
{
    size_t iBlockSize = sizeof(T);

    if(iSize <= sizeof(tagMemQueueHead) || ((iSize - sizeof(tagMemQueueHead)) / iBlockSize == 0))
    {
        throw XC_MemVectorException("[XC_MemVector::create] memory size not enough.");
    }

    init(pAddr);

    memset(pAddr, 0x00, iSize);

    _pHead->_iSize          = iSize;
    _pHead->_iBlockCount    = (iSize - sizeof(tagMemQueueHead)) / iBlockSize;
    _pHead->_iBlockSize     = iBlockSize;
}

template<typename T>
void XC_MemVector<T>::clear()
{
    assert(_pHead);

    memset(_pData, 0x00, _pHead->_iBlockSize * _pHead->_iBlockCount);
}

template<typename T>
string XC_MemVector<T>::desc() const
{
    ostringstream s;
    s << "[XC_MemVector] [_iSize=" << _pHead->_iSize << "] "
      << "[_iBlockCount="  << _pHead->_iBlockCount << "] "
      << "[_iBlockSize="  << _pHead->_iBlockSize << "] "
      << endl;
    s << "[~XC_MemVector]";

    return s.str();
}

}

#endif
