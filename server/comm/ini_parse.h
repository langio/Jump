#ifndef INI_PARSE_H_
#define INI_PARSE_H_

#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>

//
typedef std::map<std::string, std::string>      MAP_OF_KEY_VALUE;

//
struct INISection
{
    std::string                 section_name;
    MAP_OF_KEY_VALUE            key_value;
    std::vector<std::string>    key_order;
};

//
class INIParse
{
private:
    //单行最大字节数
    static const unsigned int MAX_NUM_OF_CHAR_PER_LINE  = 4*1024;
    //错误信息的最大长度
    static const unsigned int MAX_ERR_MSG_LEN2           = 4*1024;

public:
    //
    enum ErrCode
    {
        ERR_INI_FILE_NOT_INIT   = 101,      //未初始化
        ERR_CANNOT_OPEN_FILE    = 102,      //文件打开错误
        ERR_KEY_NOT_EXIST       = 103,      //key不存在
    };

public:
    //
    INIParse(const char* filename);
    ~INIParse();

    //检查文件
    bool CheckINI()
    {
        return init_flag_;
    }

    //获取字符串
    const char* GetStringValue(const char* section, const char* key);
    //获取bool值
    bool GetBoolValue(const char* section, const char* key, bool default_value = false);
    //获取int值
    int GetIntValue(const char* section, const char* key, int default_value = 0);
    //获取unsigned int值
    unsigned int GetUIntValue(const char* section, const char* key, unsigned int default_value = 0);
    //获取long值
    long GetLongValue(const char* section, const char* key, long default_value = 0);
    //获取unsigned long值
    unsigned long GetULongValue(const char* section, const char* key, unsigned long default_value = 0);

    //设置字符串
    int SetStringValue(const char* section, const char* key, const char* value);
    //设置bool值
    int SetBoolValue(const char* section, const char* key, bool value);
    //设置int值
    int SetIntValue(const char* section, const char* key, int value);
    //设置unsigned int值
    int SetUIntValue(const char* section, const char* key, unsigned int value);
    //设置long值
    int SetLongValue(const char* section, const char* key, long value);
    //设置unsigned long值
    int SetULongValue(const char* section, const char* key, unsigned long value);

    //增加section
    int AddSection(const char* name, const char* last_name = NULL);
    //增加key和字符串
    int AddStringValue(const char* section, const char* key, const char* value);
    //增加key和bool值
    int AddBoolValue(const char* section, const char* key, bool value);
    //增加int值
    int AddIntValue(const char* section, const char* key, int value);
    //增加unsigned int值
    int AddUIntValue(const char* section, const char* key, unsigned int value);
    //增加long值
    int AddLongValue(const char* section, const char* key, long value);
    //增加unsigned long值
    int AddULongValue(const char* section, const char* key, unsigned long value);

    //保存ini文件
    int Save(const char* filename = NULL);

protected:
    //解析ini文件
    bool Parse(const char* filename);
    //去除空白字符
    void TrimLine(char* line, char*& new_line);
    //记录错误信息
    void LogErrMsg(const char* format, ...);

private:
    //文件名
    std::string                 file_name_;
    //初始化标志位
    bool                        init_flag_;
    //section的数组
    std::vector<INISection*>    sections_;
    //错误信息
    char                        err_msg_[MAX_ERR_MSG_LEN2+1];

};


#endif /* INI_PARSE_H_ */
