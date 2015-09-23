#include "table_loader.h"

bool TableLoader::load(const string &sPath, const int iConfMacro)
{
    bool ret = false;

    switch (iConfMacro)
    {
        case CONFIG_TEST:
        {
            ret = test_load(sPath + "excel/test.xls");
            break;
        }
    }

    return ret;
}

