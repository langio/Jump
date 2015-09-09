#ifndef __XC_PACK_H_
#define __XC_PACK_H_

#include <netinet/in.h>
#include <vector>
#include <map>
#include <string>
#include "util/xc_ex.h"

namespace xutil
{

/////////////////////////////////////////////////
/** 
* @file xc_pack.h 
* @brief  二进制打包解包类. 
*  
* 二进制的组包解包类，通过<<、>>操作符实现. 
*  
* 对string，采用了压缩格式：长度<255: 1个字节长度, 内容；长度>=255, 4个字节(长度), 内容. 
*  
* 所有整形/长度值都采用字节序, 非线程安全； 
*  
*/             
/////////////////////////////////////////////////

/**
* @brief  组包异常类
*/
struct XC_PackIn_Exception : public XC_Exception
{
	XC_PackIn_Exception(const string &buffer) : XC_Exception(buffer){};
    XC_PackIn_Exception(const string &buffer, int err) : XC_Exception(buffer, err){};
    ~XC_PackIn_Exception() throw(){};
};

/**
 * @brief  解包异常
 */
struct XC_PackOut_Exception : public XC_Exception
{
	XC_PackOut_Exception(const string &buffer) : XC_Exception(buffer){};
    XC_PackOut_Exception(const string &buffer, int err) : XC_Exception(buffer, err){};
    ~XC_PackOut_Exception() throw(){};
};

/**
* @brief  组包类, 用户组成一个二进制包
*/
class XC_PackIn
{
public:

    /**
     * @brief  构造函数
     */
    XC_PackIn() : _pii(this, true, string::npos)
    {

    }

protected:
    /**
     *
     */
    class XC_PackInInner
    {
    public:
        /**
         * @brief  
         * @param pi
         */
        XC_PackInInner(XC_PackIn *ppi, bool bInsert, string::size_type nPos = string::npos)
        : _ppi(ppi)
        , _bInsert(bInsert)
        , _nPos(nPos)
        {

        }

        /**
         * @brief  
         */
        ~XC_PackInInner()
        {
            if(_nPos == string::npos)
            {
                 return;
            }

            if(_nPos > _buffer.length())
            {
                throw XC_PackIn_Exception("XC_PackIn cur has beyond error.");
            }

            if(_bInsert)
            {
                _ppi->getBuffer().insert(_nPos, _buffer.c_str(), _buffer.length());
            }
            else
            {
                _ppi->getBuffer().replace(_nPos, _buffer.length(), _buffer);
            }
        }

        /**
         *
         */
        void clear() { _buffer = ""; }

        /**
         * @brief  
         *
         * @return size_t
         */
        size_t length() const   { return _buffer.length(); }

        /**
         * @brief  
         *
         * @return const string&
         */
        const string &topacket() const { return _buffer; }

        /**
         *
         *
         * @return string&
         */
        string& getBuffer() { return _buffer; }

        /**
         *
         * @param b
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (bool t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (char t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (unsigned char t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (short t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (unsigned short t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (int t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (unsigned int t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (long t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (unsigned long t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (long long t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (unsigned long long t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (float t);

        /**
         * @brief  
         * @param t
         *
         * @return XC_PackIn&
         */
        XC_PackInInner& operator << (double t);

        /**
        * @brief  添加0结束字符串, 结束符'\0'也会copy进去
        * @param sBuffer
        * return void
        */
        XC_PackInInner& operator << (const char *sBuffer);

        /**
        * @brief  添加二进制字符串
        * 长度>=255: 1个字节(255) 长度, 内容
        * 长度<255, 1个字节(长度), 内容
        * @param sBuffer, buffer指针
        * @param iLen, 字节数
        * return void
        */
        XC_PackInInner& operator << (const string& sBuffer);

        /**
         * @brief  
         * @param pi
         *
         * @return XC_PackInInner&
         */
        XC_PackInInner& operator << (const XC_PackIn& pi);

    protected:
        XC_PackIn   *_ppi;
        bool        _bInsert;
        string::size_type   _nPos;
        string      _buffer;
    };

public:
    /**
    * @brief  清除组包buffer的内容
    */
    void clear() { _pii.clear(); }

    /**
    * @brief  组包长度
    * @return size_t
    */
    size_t length() const   { return _pii.length(); }

    /**
    * @brief  返回当前包体内容
    * @return string
    */
    const string& topacket() const { return _pii.topacket(); }

    /**
     * @brief  
     *
     * @return string&
     */
    string& getBuffer() {return _pii.getBuffer(); }

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackIn&
     */
    template<typename T>
    XC_PackIn& operator << (T t)
    {
        _pii << t;
        return *this;
    }

    /**
     * @brief  
     *
     * @return XC_PackIn&
     */
    XC_PackInInner insert(string::size_type nPos)
    {
        return XC_PackInInner(this, true, nPos);
    }

    /**
     * @brief  替换
     * @param iCur
     *
     * @return XC_PackIn&
     */
    XC_PackInInner replace(string::size_type nPos)
    {
        return XC_PackInInner(this, false, nPos);
    }

protected:

    /**
     * @brief  
     */
    XC_PackInInner  _pii;
};

/**
* @brief  解包类, 用户解一个二进制包
*/
class XC_PackOut
{
public:

    /**
    * @brief  构造函数, 用于解包
    * @param pBuffer : 需要解包的buffer, 该buffer需要在整个解包中有效
    * @param iLength : pBuffer长度
    */
    XC_PackOut(const char *pBuffer, size_t iLength)
    {
        init(pBuffer, iLength);
    }

    /**
    * @brief  构造函数
    */
    XC_PackOut(){};

    /**
    * @brief  初始化, 用于解包
    * @param pBuffer : 需要解包的buffer, 该buffer需要在整个解包中有效
    * @param iLength : pBuffer长度
    */
    void init(const char *pBuffer, size_t iLength)
    {
        _pbuffer    = pBuffer;
        _length     = iLength;
        _pos        = 0;
    }

    /**
     * @brief  判断是否已经解包到末尾了
     * 
     * @return bool
     */
    bool isEnd();

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (bool &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (char &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (unsigned char &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (short &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (unsigned short &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (int &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (unsigned int &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (long &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (unsigned long &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (long long &t);

    /**
     * @brief  
     * @param t
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (unsigned long long &t);

    /**
     * @brief  
     * @param f
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (float &f);

    /**
     * @brief  
     * @param f
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (double &f);

    /**
     * @brief  
     * @param sBuffer
     *
     * @return XC_PackOut&
     */
    XC_PackOut& operator >> (char *sBuffer);

    /**
    * @brief  添加二进制字符串(先记录长度
    * 长度>=255: 1个字节(255) 长度, 内容
    * 长度<255, 1个字节(长度), 内容
    * @param sBuffer, buffer指针
    * @param iLen, 字节数
    * return void
    */
    XC_PackOut& operator >> (string& sBuffer);

protected:

    /**
    * 解包时的buffer
    */
    const char      *_pbuffer;

    /**
    * 解包时的buffer长度
    */
    size_t 			_length;

    /**
    * 组包时的当前位置
    */
    size_t 			_pos;
};

//////////////////////////////////////////////////////////

/**
 * @brief  bool编码
 * @param i
 *
 * @return XC_PackIn&
 */
inline XC_PackIn& encode(XC_PackIn& pi, bool i)
{
    pi << i;
	return pi;
}

/**
 * @brief  int编码
 * @param i
 *
 * @return XC_PackIn&
 */
inline XC_PackIn& encode(XC_PackIn& pi,int i)
{
    pi << i;
	return pi;
}

/**
 * @brief  byte编码
 * @param i
 *
 * @return XC_PackIn&
 */
inline XC_PackIn& encode(XC_PackIn& pi, char i)
{
    pi << i;
	return pi;
}

/**
 * @brief  short编码
 * @param i
 *
 * @return XC_PackIn&
 */
inline XC_PackIn& encode(XC_PackIn& pi, short i)
{
    pi << i;
	return pi;
}

/**
 * @brief  string编码
 * @param s
 *
 * @return XC_PackIn&
 */
inline XC_PackIn& encode(XC_PackIn& pi, const string &i)
{
    pi << i;
	return pi;
}

/**
 * @brief  float编码
 * @param f
 *
 * @return XC_PackIn&
 */
inline XC_PackIn& encode(XC_PackIn& pi, float i)
{
    pi << i;
	return pi;
}

/**
 * @brief  double编码
 * @param f
 *
 * @return XC_PackIn&
 */
inline XC_PackIn& encode(XC_PackIn& pi, double i)
{
    pi << i;
	return pi;
}

/**
 * @brief  长整形编码
 * @param f
 *
 * @return XC_PackIn&
 */
inline XC_PackIn& encode(XC_PackIn& pi, long long i)
{
    pi << i;
	return pi;
}

/**
 * @brief  结构编码
 * @param T
 * @param t
 *
 * @return XC_PackIn&
 */
template<typename T>
inline XC_PackIn& encode(XC_PackIn& pi, const T &i)
{
    return i.encode(pi);
}

/**
 * @brief  vector编码
 * @param T
 * @param t
 *
 * @return XC_PackIn&
 */
template<typename T>
inline XC_PackIn& encode(XC_PackIn& pi, const vector<T> &t)
{
	encode(pi, (int)t.size());
    for(size_t i = 0; i < t.size(); i++)
    {
        encode(pi, t[i]);
    }
    return pi;
}

/**
 * @brief  Map编码
 * @param K
 * @param V
 * @param t
 *
 * @return XC_PackIn&
 */
template<typename K, typename V>
inline XC_PackIn& encode(XC_PackIn& pi, const map<K, V> &t)
{
	encode(pi, (int)t.size());
    typename map<K, V>::const_iterator it = t.begin();
    while(it != t.end())
    {
		encode(pi, it->first);
		encode(pi, it->second);
        ++it;
    }

    return pi;
}

//////////////////////////////////////////////////////////

/**
 * @brief  bool解码
 * @param oPtr
 * @param t
 */
inline void decode(XC_PackOut &po, bool &t)
{
	po >> t;
}

/**
 * @brief  short解码
 * @param oPtr
 * @param t
 */
inline void decode(XC_PackOut &po, short &t)
{
	po >> t;
}

/**
 * @brief  int解码
 * @param oPtr
 * @param t
 */
inline void decode(XC_PackOut &po, int &t)
{
	po >> t;
}

/**
 * @brief  byte解码
 * @param oPtr
 * @param t
 */
inline void decode(XC_PackOut &po, char &t)
{
	po >> t;
}

/**
 * @brief  string解码
 * @param oPtr
 * @param t
 */
inline void decode(XC_PackOut &po, string &t)
{
	po >> t;
}

/**
 * @brief  float解码
 * @param oPtr
 * @param t
 */
inline void decode(XC_PackOut &po, float &t)
{
	po >> t;
}

/**
 * @brief  double解码
 * @param oPtr
 * @param t
 */
inline void decode(XC_PackOut &po, double &t)
{
	po >> t;
}

/**
 * @brief  long long解码
 * @param oPtr
 * @param t
 */
inline void decode(XC_PackOut &po, long long &t)
{
	po >> t;
}

/**
 * @brief  结构解码
 * @param T
 * @param oPtr
 * @param t
 */
template<typename T>
inline void decode(XC_PackOut &po, T &t)
{
	t.decode(po);
}

/**
 * @brief  vector解码
 * @param T
 * @param oPtr
 * @param t
 */
template<typename T>
inline void decode(XC_PackOut &po, vector<T> &t)
{
	int n;
	po >> n;
	for(int i = 0; i < n; i++)
	{
        T tt;
        decode(po, tt);

        t.push_back(tt);
    }
}

/**
 * @brief  map解码
 * @param K
 * @param V
 * @param oPtr
 * @param t
 */
template<typename K, typename V>
inline void decode(XC_PackOut &po, map<K, V> &t)
{
	int n;
	po >> n;
	for(int i = 0; i < n; i++)
	{
        K k;
		V v;
        decode(po, k);
        decode(po, v);
		t[k] = v;
    }
}

}


#endif
