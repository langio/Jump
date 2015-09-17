#ifndef __TABLE_LOADER_H_
#define __TABLE_LOADER_H_

#include "libxl.h"
#include "comm_def.h"
#include "conf_table.pb.h"
#include <stdio.h>
#include "util/yac_common.h"

#define CFG_LOG_ERROR(msg, args...) printf(msg, ##args)

using namespace libxl;
using namespace protocol;
using namespace google::protobuf;
using namespace util;


//protobuf的数据类型
#define DOUBLE "double"			//													对应于c++ double
#define FLOAT "float"			//													对应于c++ float
#define INT32 "int32"			//使用可变长编码. 对于负数比较低效，如果负数较多，请使用sint32		对应于c++ int32
#define INT64 "int64"			//使用可变长编码. 对于负数比较低效，如果负数较多，请使用sint64		对应于c++ int64
#define UINT32 "uint32"			//使用可变长编码											对应于c++ uint32
#define UINT64 "uint64"			//使用可变长编码											对应于c++ uint64
#define SINT32 "sint32"			//使用可变长编码. Signed int value. 编码负数比int32更高效		对应于c++ int32
#define SINT64 "sint64"			//使用可变长编码. Signed int value. 编码负数比int64更高效		对应于c++ int64
#define FIXED32 "fixed32"		//恒定四个字节。如果数值几乎总是大于2的28次方，该类型比unit32更高效	对应于c++ uint32
#define FIXED64 "fixed64"		//恒定四个字节。如果数值几乎总是大于2的56次方，该类型比unit64更高效	对应于c++ uint64
#define SFIXED32 "sfixed32"		//恒定四个字节											对应于c++ int32
#define SFIXED64 "sfixed64"		//恒定八个字节											对应于c++ int64
#define BOOL "bool"				//													对应于c++ bool
#define STRING "string"			//A string must always contain UTF-8 encoded or 7-bit ASCII text	对应于c++ string
#define BYTES "bytes"			//包含任意数量顺序的字节									对应于c++ string


//Message使用完之后需要delete
Message* createMessage(const string &typeName)
{
    Message *message = NULL;

    // 查找message的descriptor
    const Descriptor *descriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
    if (descriptor)
    {
    	// 创建default message(prototype)
        const Message *prototype = MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (NULL != prototype)
        {
            // 创建一个可修改的message
            message = prototype->New();
        }
    }

    return message;
}

template<typename key, typename value>
class CommLoad
{
public:
    typedef key (*getKeyFunc)(const value&);

    static bool load2Map(const string& excel_file, int32_t index, map<key, value>& mOut, getKeyFunc getKey = defaultGetKey);

private:
    static key defaultGetKey(const value& v);
};

template<typename key, typename value>
key CommLoad<key, value>::defaultGetKey(const value& v)
{
    return v.id;
}

template<typename key, typename value>
bool CommLoad<key, value>::load2Map(const string& conf_file, int32_t index, map<key, value>& mOut, getKeyFunc getKey)
{

	Book* book = xlCreateBook();

	__BEGIN_PROC__

	if(NULL == book)
	{
		CFG_LOG_ERROR("create xlCreateBook failed!");
		false;
	}

	if(book->load(conf_file.c_str()))
	{
		CFG_LOG_ERROR("load file %s failed!", conf_file.c_str());

		break;
	}

	Sheet* sheet = book->getSheet(index);

	if(NULL == sheet)
	{
		CFG_LOG_ERROR("getSheet failed, index:%d", index);
		break;
	}

	int32_t row_num = sheet->lastRow();
	int32_t col_num = sheet->lastCol();

	//获取message名称，固定位置，在第0行第0列单元格内
	const char* message = sheet->readStr(0,0);
	if(NULL == message)
	{
		CFG_LOG_ERROR("get message name failed!");
		break;
	}

	const string message_name("protocol." + string(message));

	//从（2,1）开始，处理这一行的所有字段，解析字段中标明的message的字段类型和名字
	map<string, string> fields;		//map<字段名，类型>
	for(int32_t col=1; col<col_num; ++col)
	{
		const char* f = sheet->readStr(2, col);
		if(NULL == f)
		{
			CFG_LOG_ERROR("empty unit. row:2 col:%d", col);
			break;
		}

		//字段的类型名和字段名是使用空格分割的
		vector<string> v = YAC_Common::sepstr<string>(string(f), " ");
		if(v.size() != 2)
		{
			CFG_LOG_ERROR("invalid unit content:%s", f);
		}

		fields[v[1]] = v[0];
	}

	double d = sheet->readNum(3, 1);
	cout << d << endl;

	int h = sheet->lastRow();
	cout << h << endl;


	__END_PROC__

	book->release();

	return true;

//    __TRY__
//
//    //cout << "debug: " << __FILE__ << ":" << __LINE__ << "|" << sJceFileName << endl;
//        mOut.clear();
//
//        string fileData = TC_File::load2str(sJceFileName);
//        JceInputStream<BufferReader> isk;
//        isk.setBuffer(fileData.c_str(), fileData.length());
//        key mapKey;
//        while (isk.hasEnd() != true)
//        {
//            value stRecord;
//            stRecord.readFrom(isk);
//            mapKey = getKey(stRecord);
//            mOut[mapKey] = stRecord;
//
//            ostringstream os;
//            stRecord.display(os);
//
//            //TODO 20141004 toutzhang: 配合 GM 和客户端，解决数据比 jce 新的问题
//            //isk.skipToStructEnd();
//        }
//
//        return true;
//
//    __CATCH_INFO(sJceFileName.c_str())
//    return false;
}


#endif


