#ifndef __TABLE_LOADER_H_
#define __TABLE_LOADER_H_

#include "common_load.h"

//需要加载的配置的宏定义
#define CONFIG_TEST		0


class TableLoader
{
public:

    TableLoader();

bool load(const string &sPath, const int iConfMacro);

bool test_load(const string& file_name);

};

#endif


