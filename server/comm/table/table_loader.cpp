#include "table_loader.h"

TableLoader::TableLoader()
{}

bool TableLoader::load(const string &sPath, const int iConfMacro)
{
    bool ret = false;

    switch (iConfMacro)
    {
        case CONFIG_TEST:
        {
            ret = test_load(sPath + "excel/test.csv");
            break;
        }
    }

    return ret;
}


bool TableLoader::test_load(const string& csv_name)
{
    bool ret = true;
    __TRY__

    __BEGIN_PROC__

	ret = CommLoad<google::protobuf::int32, conf_test>::loadCSV2Map(csv_name, 0, m_test_conf, getTestKey);


	//cout << __FILE__ << "|" << __LINE__ << "|" << __FUNCTION__ << endl << CommFunc::printMapJce<Int32_t, JSuitConfTable>(mSuit);

	__END_PROC__

	return ret;

    __CATCH__

    return false;
}

