#include "table_mgr.h"

bool TableMgr::reload(const int32_t confMacroArray[], int32_t size)
{
    bool result = true;

    for (int32_t i = 0; i < size; ++i)
    {
        result &= reload(confMacroArray[i]);
    }


    return result;
}

bool TableMgr::reload(const int iConfMacro)
{

    if(_tableLoader.load(TABLE_PATH, iConfMacro))
    {
        return true;
    }
    else
    {
        CFG_LOG_ERROR("TableMgr::reload error. Macro:%d", iConfMacro);
        return false;
    }

    return false;
}
