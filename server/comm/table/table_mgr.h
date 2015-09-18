#ifndef __TABLE_MGR_H_
#define __TABLE_MGR_H_

#include "table_loader.h"

static const string TABLE_PATH = "../cfg/";

class TableMgr
{

public:
    static TableMgr* getInstance()
    {
    	static TableMgr instance;
        return &instance;
    }

	const TableLoader* getTableLoader() const
	{
		return &_tableLoader;
	}

	bool reload(const int32_t confMacroArray[], int32_t size);

private:
    TableMgr() {};
    TableMgr(const TableMgr&);
    TableMgr &operator=(const TableMgr&);
    bool reload(const int32_t iConfMacro);

private:
    TableLoader _tableLoader;
};


#endif

