#include "table_loader.h"

TableLoader::TableLoader()
{}

bool TableLoader::load(const string &sPath, const int iConfMacro)
{
    bool ret = false;

    __TRY__

    switch (iConfMacro)
    {
        case CONFIG_TEST:
        {
        	ret = CommLoad<int32_t, conf_test>::loadCSV2Map(sPath + "excel/test.csv", getTestKey);
        	CommLoad<int32_t, conf_test>::printMap();

            break;
        }
        case CONFIG_TEST1:
        {
        	ret = CommLoad<int32_t, conf_test1>::loadCSV2Map(sPath + "excel/test1.csv", getTestKey1);
        	CommLoad<int32_t, conf_test1>::printMap();

        	break;
        }
    }


	return ret;

	__CATCH__

    return false;
}


