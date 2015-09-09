#ifndef __XC_ENCODER_H_
#define __XC_ENCODER_H_

#include <vector>

#include "util/xc_ex.h"

namespace xutil
{
/////////////////////////////////////////////////
/** 
* @file xc_encoder.h 
* @brief  转码类 
*  
* 
* 
*/
/////////////////////////////////////////////////

/**
*  @brief  转码异常类
*/
struct XC_Encoder_Exception : public XC_Exception
{
	XC_Encoder_Exception(const string &buffer) : XC_Exception(buffer){};
    XC_Encoder_Exception(const string &buffer, int err) : XC_Exception(buffer, err){};
	~XC_Encoder_Exception() throw(){};
};

/**
*  @brief 该类提供基本常用编码的转换.
*  
*   	  Gbk到utf8之间的相互转码函数，通过静态函数提供.
*  
*   	  1：GBK ==> UTF8的转换
*  
*   	  2：UTF8 ==> GBK的转换
*/
class XC_Encoder
{
public:
    /**
	* @brief  gbk 转换到 utf8.
	*  
    * @param sOut        输出buffer
    * @param iMaxOutLen  输出buffer最大的长度/sOut的长度
    * @param sIn         输入buffer
    * @param iInLen      输入buffer长度
    * @throws            XC_Encoder_Exception
    * @return 
    */
    static void  gbk2utf8(char *sOut, int &iMaxOutLen, const char *sIn, int iInLen);

    /**
	* @brief  gbk 转换到 utf8. 
	*  
    * @param sIn   输入buffer*
    * @throws      XC_Encoder_Exception
    * @return      转换后的utf8编码
    */
    static string gbk2utf8(const string &sIn);

    /**
	* @brief  gbk 转换到 utf8. 
	*  
    * @param sIn    输入buffer
    * @param vtStr  输出gbk的vector
    * @throws       XC_Encoder_Exception
    * @return
    */
    static void gbk2utf8(const string &sIn, vector<string> &vtStr);

    /**
	* @brief  utf8 转换到 gbk. 
	*  
    * @param sOut       输出buffer
    * @param iMaxOutLen 输出buffer最大的长度/sOut的长度
    * @param sIn        输入buffer
    * @param iInLen     输入buffer长度
    * @throws           XC_Encoder_Exception
    * @return
    */
    static void utf82gbk(char *sOut, int &iMaxOutLen, const char *sIn, int iInLen);

    /**
	* @brief  utf8 转换到 gbk. 
	*  
    * @param sIn  输入buffer
    * @throws     XC_Encoder_Exception
    * @return    转换后的gbk编码
    */
    static string utf82gbk(const string &sIn);

	/**	
	* @brief  将string的\n替换掉,转义字符串中的某个字符 
	*  
	* 缺省:\n 转换为 \r\0; \r转换为\,
	*  
	* 主要用于将string记录在一行，通常用于写bin-log的地方;
	* @param str   待转换字符串
	* @param f     需要转义的字符
	* @param t     转义后的字符
	* @param u     借用的转义符
	* @return str  转换后的字符串
	*/
	static string transTo(const string& str, char f = '\n', char t = '\r', char u = '\0');

	/**
	* @brief  从替换的数据恢复源数据,将 transTo 的字符串还原， 
	*  
	*  缺省:\r\0 还原为\n，\r\r还原为,
	*  
	*  主要用于将string记录在一行，通常用于写bin-log的地方
	* @param str  待还原的字符串(必须是transTo后得到的字符串)
	* @param f    被转义的字符
	* @param t    转义后的字符
	* @param u    借用的转义符
	* @return str 还原后的字符串
	*/
	static string transFrom(const string& str, char f = '\n', char t = '\r', char u = '\0');
};

}


#endif


