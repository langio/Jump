#ifndef __XC_FILE_H_
#define __XC_FILE_H_

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include "util/xc_ex.h"
#include "util/xc_common.h"

namespace xutil
{
/////////////////////////////////////////////////
/** 
 * @file xc_file.h 
 * @brief  文件处理类. 
 *  
 */
/////////////////////////////////////////////////


/**
* @brief 文件异常类. 
*  
*/
struct XC_File_Exception : public XC_Exception
{
    XC_File_Exception(const string &buffer) : XC_Exception(buffer){};
    XC_File_Exception(const string &buffer, int err) : XC_Exception(buffer, err){};
    ~XC_File_Exception() throw(){};
};

/**
* @brief 常用文件名处理函数. 
*  
*/
class XC_File
{
public:

    /**
	* @brief 获取文件大小, 如果文件不存在, 则返回0. 
	*  
    * @param  sFullFileName 文件全路径(所在目录和文件名)
    * @return               ofstream::pos_type类型文件大小
    */
    static ifstream::pos_type getFileSize(const string &sFullFileName);

    /**
	 * @brief 判断是否为绝对路径, 忽略空格以'/'开头. 
	 *  
	 * @param sFullFileName 文件全路径(所在目录和文件名)
	 * @return              ture是绝对路径，false代表非绝对路径 
     */
    static bool isAbsolute(const string &sFullFileName);

    /**
    * @brief 判断给定路径的文件是否存在. 
	* 如果文件是符号连接,则以符号连接判断而不是以符号连接指向的文件判断 
    * @param sFullFileName 文件全路径
    * @param iFileType     文件类型, 缺省S_IFREG
	* @return           true代表存在，fals代表不存在
    */
    static bool isFileExist(const string &sFullFileName, mode_t iFileType = S_IFREG);

    /**
    * @brief 判断给定路径的文件是否存在.  
    * 注意: 如果文件是符号连接,则以符号连接指向的文件判断
    * @param sFullFileName  文件全路径
    * @param iFileType      文件类型, 缺省S_IFREG
    * @return               true-存在，fals-不存在
	*/ 
    static bool isFileExistEx(const string &sFullFileName, mode_t iFileType = S_IFREG);

    /**
	 * @brief 规则化目录名称, 把一些不用的去掉, 例如./等. 
	 *  
	 * @param path 目录名称
     * @return        规范后的目录名称
     */
    static string simplifyDirectory(const string& path);

    /**
	* @brief 创建目录, 如果目录已经存在, 则也返回成功. 
	*  
    * @param sFullPath 要创建的目录名称
    * @param iFlag     权限, 默认 S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP| S_IXGRP | S_IROTH |  S_IXOTH
	* @return bool  true-创建成功 ，false-创建失败 
    */
    static bool makeDir(const string &sDirectoryPath, mode_t iFlag = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH |  S_IXOTH);

    /**
	 *@brief 循环创建目录, 如果目录已经存在, 则也返回成功. 
	 * 
     * @param sFullPath 要创建的目录名称
	 * @param iFlag   权限, 默认 S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH |  S_IXOTH
     * @return           true-创建成功，false-创建失败 
     */

    static bool makeDirRecursive(const string &sDirectoryPath, mode_t iFlag = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH |  S_IXOTH);

    /**
	 * @brief 设置文件是否可执行. 
	 *  
     * @param sFullFileName 文件全路径
	 * @param canExecutable true表示可执行, false代表不可之行 
     * @return                 成功返回0, 其他失败
     */
    static int setExecutable(const string &sFullFileName, bool canExecutable);

    /**
	 * @brief 判断文件是否可执行. 
	 *  
	 * @param sFullFileName 文件全路径
     * @return                 true-可执行, false-不可执行 
     */
    static bool canExecutable(const string &sFullFileName);

    /**
	 * @brief 删除一个文件或目录. 
	 *  
     * @param sFullFileName 文件或者目录的全路径
     * @param bRecursive    如果是目录是否递归删除
     * @return              0-成功，失败可以通过errno查看失败的原因
     */
    static int removeFile(const string &sFullFileName, bool bRecursive);

    /**
	* @brief 读取文件到string  
	* 文件存在则返回文件数据，不存在或者读取文件错误的时候, 返回为空
    * @param sFullFileName 文件名称
    * @return              文件数据
    */
    static string load2str(const string &sFullFileName);

    /**
	* @brief 写文件. 
	*  
    * @param sFullFileName 文件名称
    * @param sFileData     文件内容
    * @return 
    */
    static void save2file(const string &sFullFileName, const string &sFileData);

    /**
	 * @brief 写文件. 
	 *  
     * @param sFullFileName  文件名
     * @param sFileData      数据指针
	 * @param length      写入长度 
     * @return               0-成功,-1-失败
     */
    static int save2file(const string &sFullFileName, const char *sFileData, size_t length);

    /**
     * @brief 获取前当可执行文件路径.
     *
     * @return string 可执行文件的路径全名称
     */
    static string getExePath();

    /**
	* @brief 提取文件名称
       *从一个完全文件名中去掉路径，例如:/usr/local/temp.gif获取temp.gif
    *@param sFullFileName  文件的完全名称
    *@return string        提取后的文件名称
    */
    static string extractFileName(const string &sFullFileName);

    /**
    * @brief 从一个完全文件名中提取文件的路径.
	*  
    * 例如1: "/usr/local/temp.gif" 获取"/usr/local/"
    * 例如2: "temp.gif" 获取 "./"
    * @param sFullFileName 文件的完全名称
    * @return              提取后的文件路径
    */
    static string extractFilePath(const string &sFullFileName);

    /**
    * @brief 提取文件扩展名.
	* 
    * 例如1: "/usr/local/temp.gif" 获取"gif"
    * 例如2: "temp.gif" 获取"gif"
    *@param sFullFileName 文件名称
    *@return              文件扩展名
    */
    static string extractFileExt(const string &sFullFileName);

    /**
    * @brief 提取文件名称,去掉扩展名.  
    * 例如1: "/usr/local/temp.gif" 获取"/usr/local/temp"
    * 例如2: "temp.gif" 获取"temp"
    * @param sFullFileName 文件名称
    * @return              去掉扩展名的文件名称
    */
    static string excludeFileExt(const string &sFullFileName);

    /**
	* @brief 替换文件扩展名 
	*  
	* 改变文件类型，如果无扩展名,则加上扩展名 =?1: 
	* 例如1："/usr/temp.gif" 替 换 "jpg" 得到"/usr/temp.jpg" 
	* 例如2: "/usr/local/temp" 替 换 "jpg" 得到"/usr/local/temp.jpg"
    * @param sFullFileName 文件名称
    * @param sExt          扩展名
    * @return              替换扩展名后的文件名
    */
    static string replaceFileExt(const string &sFullFileName, const string &sExt);

    /**
    * @brief 从一个url中获取完全文件名.
	*  
    * 获取除http://外,第一个'/'后面的所有字符
    * 例如1:http://www.qq.com/tmp/temp.gif 获取tmp/temp.gif
    * 例如2:www.qq.com/tmp/temp.gif 获取tmp/temp.gif
    * 例如3:/tmp/temp.gif 获取tmp/temp.gif
    * @param sUrl url字符串
    * @return     文件名称
    */
    static string extractUrlFilePath(const string &sUrl);

    /**
    * @brief 遍历文件时确定是否选择.
	*  
    * @return 1-选择, 0-不选择
    */
    typedef int (*FILE_SELECT)(const dirent *);

    /**
	* @brief 扫描一个目录. 
	*  
    * @param sFilePath     需要扫描的路径
    * @param vtMatchFiles  返回的文件名矢量表
    * @param f             匹配函数,为NULL表示所有文件都获取
    * @param iMaxSize      最大文件个数,iMaxSize <=0时,返回所有匹配文件
    * @return              文件个数
    */
    static size_t scanDir(const string &sFilePath, vector<string> &vtMatchFiles, FILE_SELECT f = NULL, int iMaxSize = 0);

    /**
	 * @brief 遍历目录, 获取目录下面的所有文件和子目录. 
	 *  
     * @param path       需要遍历的路径
     * @param files      目标路径下面所有文件
     * @param bRecursive 是否递归子目录
     *
     **/
    static void listDirectory(const string &path, vector<string> &files, bool bRecursive);
    
     /**
	 * @brief 复制文件或目录.   
	 * 将文件或者目录从sExistFile复制到sNewFile  
     * @param sExistFile 复制的文件或者目录源路径
     * @param sNewFile   复制的文件或者目录目标路径
     * @param bRemove    是否先删除sNewFile再copy ，防止Textfile busy导致复制失败 
     * @return 
     */
    static void copyFile(const string &sExistFile, const string &sNewFile,bool bRemove = false);
};

}
#endif // XC_FILE_H
