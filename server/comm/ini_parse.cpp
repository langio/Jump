#include "ini_parse.h"
#include <cassert>

INIParse::INIParse(const char* filename) :
    file_name_(filename)
{
    sections_.clear();
    init_flag_  = Parse(filename);
}

INIParse::~INIParse()
{
    for (size_t i = 0; i < sections_.size(); ++i)
    {
        if (sections_[i] != NULL)
        {
            delete sections_[i];
            sections_[i] = NULL;
        }
    }

    sections_.clear();
}

bool INIParse::Parse(const char* filename)
{
    FILE * file = fopen (filename, "r");
    if (file == NULL)
    {
        LogErrMsg("cannot open %s", filename);
        return false;
    }

    char line[MAX_NUM_OF_CHAR_PER_LINE + 1] = {0};
    while (fgets(line, MAX_NUM_OF_CHAR_PER_LINE, file) != NULL)
    {
        char* new_line = NULL;
        TrimLine(line, new_line);
        size_t new_line_len = strlen(new_line);

        //注释行
        if (new_line[0] == '#' || new_line[0] == ';' || new_line_len == 0)
        {
            continue;
        }
        //节
        else if (new_line[0] == '[' && new_line[new_line_len-1] == ']')
        {
            char* section_name = NULL;
            new_line[new_line_len - 1] = 0;
            TrimLine(&new_line[1], section_name);
            if (section_name[0] != '\0')
            {
                INISection* section = new INISection;
                section->section_name   = section_name;
                sections_.push_back(section);
            }
        }
        //Key-Value
        else
        {
            //没有section，插入一个默认的
            if (sections_.size() == 0)
            {
                INISection* section = new INISection;
                section->section_name   = "";
                sections_.push_back(section);
            }
            INISection* section = sections_[sections_.size()-1];

            char* key_str   = new_line;
            char* value_str = strstr(new_line, "=");
            if (value_str != NULL)
            {
                *value_str  = 0;
                ++value_str;
            }

            char* trim_key_str = NULL;
            TrimLine(key_str, trim_key_str);
            char* trim_value_str = NULL;
            TrimLine(value_str, trim_value_str);
            std::pair<MAP_OF_KEY_VALUE::iterator, bool> result =
                    section->key_value.insert(MAP_OF_KEY_VALUE::value_type(trim_key_str, trim_value_str));
            if (!result.second)
            {
                LogErrMsg("insert (%s:%s) failed.", trim_key_str, trim_value_str);
                return false;
            }
            else
            {
                //printf("section[%s] %s=%s", section->section_name.c_str(), trim_key_str, trim_value_str);
            }

            section->key_order.push_back(trim_key_str);
        }
    }

    fclose (file);

    return true;
}

int INIParse::Save(const char* filename)
{
    if (!init_flag_)
    {
        return ERR_INI_FILE_NOT_INIT;
    }

    FILE * file = fopen (filename == NULL ? file_name_.c_str() : filename, "w");
    if (file == NULL)
    {
        LogErrMsg("cannot open %s", file_name_.c_str());
        return ERR_CANNOT_OPEN_FILE;
    }


    for (size_t i = 0; i < sections_.size(); ++i)
    {
        INISection* section = sections_[i];
        if (!section->section_name.empty())
        {
            fprintf(file, "[%s]\n", section->section_name.c_str());
        }

        for (size_t j = 0; j < section->key_order.size(); ++j)
        {
            fprintf(file, "%s\t\t= %s\n", section->key_order[j].c_str(), section->key_value[section->key_order[j]].c_str());
        }
    }

    fclose(file);

    return 0;
}

void INIParse::LogErrMsg(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(err_msg_, sizeof(err_msg_), format, argptr);
    err_msg_[MAX_ERR_MSG_LEN2] = 0;
    va_end(argptr);
}

void INIParse::TrimLine(char* line, char*& new_line)
{
    new_line    = line;

    //
    size_t len = strlen(line);
    if (len == 0)
        return;

    /* trime right first */
    int n = len - 1;
    for (; n >= 0; --n)
    {
        // trim space and tab
        if (line[n] == ' ' || line[n] == '\t' || line[n] == '\r' || line[n] == '\n')
        {
            line[n] = 0;
        }
        else
        {
            break;
        }
    }

    if (n == 0)
        return;

    /* trim left */
    int new_len = n + 1;
    for (n = 0; n < new_len; ++n)
    {
        if(line[n] != ' ' &&  line[n] != '\t' && line[n] != '\r' && line[n] != '\n')
        {
            break;
        }
    }

    new_line = &line[n];
}

//获取字符串
const char* INIParse::GetStringValue(const char* section, const char* key)
{
    if (!init_flag_)
    {
        return NULL;
    }

    for (size_t i = 0; i < sections_.size(); ++i)
    {
        INISection* sec = sections_[i];
        if (strcasecmp(sec->section_name.c_str(), section) == 0)
        {
            MAP_OF_KEY_VALUE::iterator iter = sec->key_value.find(key);
            if (iter != sec->key_value.end())
            {
                return iter->second.c_str();
            }
        }
    }

    return NULL;
}

//获取bool值
bool INIParse::GetBoolValue(const char* section, const char* key, bool default_value)
{
    const char* v = GetStringValue(section, key);
    if (v != NULL && strlen(v) != 0)
    {
        return static_cast<bool>(strtoll(v, NULL, 0));
    }

    return default_value;
}

//获取int值
int INIParse::GetIntValue(const char* section, const char* key, int default_value)
{
    const char* v = GetStringValue(section, key);
    if (v != NULL && strlen(v) != 0)
    {
        return static_cast<int>(strtoll(v, NULL, 0));
    }

    return default_value;
}
//获取unsigned int值
unsigned int INIParse::GetUIntValue(const char* section, const char* key, unsigned int default_value)
{
    const char* v = GetStringValue(section, key);
    if (v != NULL && strlen(v) != 0)
    {
        return static_cast<unsigned int>(strtoll(v, NULL, 0));
    }

    return default_value;
}
//获取long值
long INIParse::GetLongValue(const char* section, const char* key, long default_value)
{
    const char* v = GetStringValue(section, key);
    if (v != NULL && strlen(v) != 0)
    {
        return static_cast<long>(strtoll(v, NULL, 0));
    }

    return default_value;
}
//获取unsigned long值
unsigned long INIParse::GetULongValue(const char* section, const char* key, unsigned long default_value)
{
    const char* v = GetStringValue(section, key);
    if (v != NULL && strlen(v) != 0)
    {
        return static_cast<unsigned long>(strtoll(v, NULL, 0));
    }

    return default_value;
}

//
int INIParse::SetStringValue(const char* section, const char* key, const char* value)
{
    if (!init_flag_)
    {
        return ERR_INI_FILE_NOT_INIT;
    }

    for (size_t i = 0; i < sections_.size(); ++i)
    {
        INISection* sec = sections_[i];
        if (strcasecmp(sec->section_name.c_str(), section) == 0)
        {
            MAP_OF_KEY_VALUE::iterator iter = sec->key_value.find(key);
            if (iter != sec->key_value.end())
            {
                iter->second = value;
                return 0;
            }
        }
    }

    return ERR_KEY_NOT_EXIST;
}

//设置bool值
int INIParse::SetBoolValue(const char* section, const char* key, bool value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%s", value == true ? "true" : "false");
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return SetStringValue(section, key, buff);
}
//设置int值
int INIParse::SetIntValue(const char* section, const char* key, int value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%d", value);
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return SetStringValue(section, key, buff);
}
//设置unsigned int值
int INIParse::SetUIntValue(const char* section, const char* key, unsigned int value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%u", value);
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return SetStringValue(section, key, buff);
}
//设置long值
int INIParse::SetLongValue(const char* section, const char* key, long value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%ld", value);
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return SetStringValue(section, key, buff);
}
//设置unsigned long值
int INIParse::SetULongValue(const char* section, const char* key, unsigned long value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%lu", value);
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return SetStringValue(section, key, buff);
}

int INIParse::AddSection(const char* name, const char* last_name)
{
    //先找一下，避免重复
    for (size_t i = 0; i < sections_.size(); ++i)
    {
        INISection* sec = sections_[i];
        if (strcasecmp(sec->section_name.c_str(), name) == 0)
        {
            return 0;
        }
    }

    INISection* section     = new INISection;
    assert(section != NULL);
    section->section_name   = name;

    bool if_insert = false;

    if (last_name)
    {
        for (size_t i = 0; i < sections_.size(); ++i)
        {
            INISection* sec = sections_[i];
            if (strcasecmp(sec->section_name.c_str(), last_name) == 0 && (i+1) != sections_.size())
            {
                sections_.insert(sections_.begin() + i + 1, section);
                if_insert = true;
                break;
            }
        }
    }

    if (!if_insert)
    {
        sections_.push_back(section);
    }

    return 0;
}

//增加key和字符串
int INIParse::AddStringValue(const char* section, const char* key, const char* value)
{
    if (!init_flag_)
    {
        return ERR_INI_FILE_NOT_INIT;
    }

    for (size_t i = 0; i < sections_.size(); ++i)
    {
        INISection* sec = sections_[i];
        if (strcasecmp(sec->section_name.c_str(), section) == 0)
        {
            MAP_OF_KEY_VALUE::iterator iter = sec->key_value.find(key);
            if (iter != sec->key_value.end())
            {
                iter->second = value;
                return 0;
            }
            else
            {
                sec->key_value.insert(MAP_OF_KEY_VALUE::value_type(key, value));
                sec->key_order.push_back(key);
                return 0;
            }
        }
    }

    return ERR_KEY_NOT_EXIST;
}
//增加key和bool值
int INIParse::AddBoolValue(const char* section, const char* key, bool value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%s", value == true ? "true" : "false");
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return AddStringValue(section, key, buff);
}
//增加int值
int INIParse::AddIntValue(const char* section, const char* key, int value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%d", value);
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return AddStringValue(section, key, buff);
}
//增加unsigned int值
int INIParse::AddUIntValue(const char* section, const char* key, unsigned int value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%u", value);
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return AddStringValue(section, key, buff);
}
//增加long值
int INIParse::AddLongValue(const char* section, const char* key, long value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%ld", value);
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return AddStringValue(section, key, buff);
}
//增加unsigned long值
int INIParse::AddULongValue(const char* section, const char* key, unsigned long value)
{
    char buff[MAX_NUM_OF_CHAR_PER_LINE+1] = {0};
    snprintf(buff, sizeof(buff), "%lu", value);
    buff[MAX_NUM_OF_CHAR_PER_LINE] = 0;

    return AddStringValue(section, key, buff);
}

