#ifndef __YAC_OPTION_H
#define __YAC_OPTION_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

namespace util
{
/////////////////////////////////////////////////
/** 
 * @file yac_option.h 
 * @brief 命令行参数解析类.
 *  
 */
            
/////////////////////////////////////////////////
/**
 * @brief 命令解析类，通常用于解析命令行参数
 *
 * 支持以下形式的参数:  ./main.exe --name=value --with abc def 
 */
class YAC_Option
{
public:
    /**
     * @brief 构造函数
     */
    YAC_Option(){};

    /**
	 * @brief 解码. 
	 *  
     * @param argc 参数个数
     * @param argv 参数数组
     *
     */
    void decode(int argc, char *argv[]);

    /**
	 * @brief 是否存在某个--标识的参数. 
	 *  
	 * @param sName  要判断的标识
     * @return bool 存在返回true，否则返回false
     */
    bool hasParam(const string &sName);

    /**
	 * @brief 获取某个--表示的参数，如果参数不存在或者参数值为空 , 
	 *  	  都返回""
	 * @param sName   标识
     * @return string 标识的参数值
     */
    string getValue(const string &sName);

    /**
     * @brief 获取所有--标识的参数.
     *
     * @return map<string,string> map类型的标识和参数值的对应关系
     */
    map<string, string>& getMulti();

    /**
	 * @brief 获取所有普通的参数, 例子中的abc, 
	 *  	  def，参数按照顺序在vector中
     * @return vector<string> 顺序存放参数的vector
     */
    vector<string>& getSingle();

protected:

    /**
	 * @brief 解析字符串，取出标识和其对应的参数值， 
	 *  	  对型如--name=value 的字符串进行解析，取出name和vaue
     * @param s 要解析的字符串
     */
    void parse(const string &s);

protected:
    /**
     *存放标识和其对应参数的对应关系，例如：对于--name=value，存放name和value
     */
    map<string, string> _mParam;

    /**
     *存放普通参数的vetor
     */
    vector<string>      _vSingle;
};

}

#endif

