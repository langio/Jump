#ifndef	__YAC_MEM_VECTOR_H__
#define __YAC_MEM_VECTOR_H__

#include "util/yac_ex.h"
#include <sstream>
#include <string.h>

namespace util
{
/////////////////////////////////////////////////
/** 
* @file yac_mem_vector.h 
* @brief  共享内存数组类. 
*  
* @author  jarodruan@tencent.com
*/            
/////////////////////////////////////////////////

/**
* @brief 异常类
*/
struct YAC_MemVectorException : public YAC_Exception
{
	YAC_MemVectorException(const string &buffer) : YAC_Exception(buffer){};
    YAC_MemVectorException(const string &buffer, int err) : YAC_Exception(buffer, err){};
    ~YAC_MemVectorException() throw(){};
};

/**
* @brief 共享内存随机访问数据块.
*  
* 每块数据快大小相等，模板对象T只能是简单的数据类型 ，
*  
* 需要具备bit-copy的语义才行 
*/
template<typename T>
class YAC_MemVector
{
public:

    /**
    * @brief 构造函数
    */
    YAC_MemVector() : _pHead(NULL) ,_pData(NULL)
    {
    }

    /**
     * @brief 
     * @param mv
     */
    YAC_MemVector(const YAC_MemVector<T> &mv)
    : _pHead(mv._pHead) ,_pData(mv._pData)
    {

    }

    /**
     * @brief 
     * @param mv
     *
     * @return bool
     */
    bool operator==(const YAC_MemVector<T> &mv)
    {
        return _pHead == mv._pHead && _pData == mv._pData;
    }

    /**
     * @brief 
     * @param mv
     *
     * @return bool
     */
    bool operator!=(const YAC_MemVector<T> &mv)
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
    class YAC_MemVectorIterator : public std::iterator<std::random_access_iterator_tag, T>
    {
    public:
        /**
         * @brief 构造函数
         * @param pmv
         * @param iIndex
         */
        YAC_MemVectorIterator(YAC_MemVector *pmv, size_t iIndex) : _pmv(pmv), _iIndex(iIndex)
        {
        }

        /**
         * @brief 前置++
         *
         * @return YAC_MemVectorIterator&
         */
        YAC_MemVectorIterator& operator++()
        {
            ++_iIndex;
            return *this;
        }

        /**
         * @brief 后置++
         */
        YAC_MemVectorIterator operator++(int)
        {
            YAC_MemVectorIterator tmp = *this;

            ++_iIndex;
            return tmp;
        }

        /**
         * @brief 
         * @param mv
         *
         * @return YAC_MemVectorIterator
         */
        bool operator==(const YAC_MemVectorIterator& mv)
        {
            return _iIndex == mv._iIndex && _pmv == mv._pmv;
        }

        /**
         * @brief 
         * @param mv
         *
         * @return YAC_MemVectorIterator
         */
        bool operator!=(const YAC_MemVectorIterator& mv)
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
        YAC_MemVector    *_pmv;

        /**
         *
         */
        size_t          _iIndex;
    };

    typedef YAC_MemVectorIterator iterator;

    /**
     *
     *
     * @return YAC_MemVectorIterator
     */
    YAC_MemVectorIterator begin()	{ return YAC_MemVectorIterator(this, 0); }

    /**
     *
     *
     * @return YAC_MemVectorIterator
     */
    YAC_MemVectorIterator end()		{ return YAC_MemVectorIterator(this, _pHead->_iBlockCount); }

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
			s << string("[YAC_MemVector::get] index beyond : index = ") << iIndex << " > " << _pHead->_iBlockCount;

            throw YAC_MemVectorException(s.str());
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
void YAC_MemVector<T>::create(void *pAddr, size_t iSize)
{
    size_t iBlockSize = sizeof(T);

    if(iSize <= sizeof(tagMemQueueHead) || ((iSize - sizeof(tagMemQueueHead)) / iBlockSize == 0))
    {
        throw YAC_MemVectorException("[YAC_MemVector::create] memory size not enough.");
    }

    init(pAddr);

    memset(pAddr, 0x00, iSize);

    _pHead->_iSize          = iSize;
    _pHead->_iBlockCount    = (iSize - sizeof(tagMemQueueHead)) / iBlockSize;
    _pHead->_iBlockSize     = iBlockSize;
}

template<typename T>
void YAC_MemVector<T>::clear()
{
    assert(_pHead);

    memset(_pData, 0x00, _pHead->_iBlockSize * _pHead->_iBlockCount);
}

template<typename T>
string YAC_MemVector<T>::desc() const
{
    ostringstream s;
    s << "[YAC_MemVector] [_iSize=" << _pHead->_iSize << "] "
      << "[_iBlockCount="  << _pHead->_iBlockCount << "] "
      << "[_iBlockSize="  << _pHead->_iBlockSize << "] "
      << endl;
    s << "[~YAC_MemVector]";

    return s.str();
}

}

#endif
