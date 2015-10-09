#ifndef __TABLE_LOADER_H_
#define __TABLE_LOADER_H_

#include "common_load.h"
#include "conf_table.pb.h"

//需要加载的配置的宏定义
#define CONFIG_TEST		0
#define CONFIG_TEST1	1


class TableLoader
{
public:

    TableLoader();

	bool load(const string &sPath, const int iConfMacro);

	static int32_t getTestKey(const conf_test& stTest)
	{
		return stTest.id();
	}

	static int32_t getTestKey1(const conf_test1& stTest)
	{
		return stTest.id();
	}


private:
};

#endif


