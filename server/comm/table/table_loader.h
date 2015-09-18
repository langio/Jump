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


//protobuf的数据类型，excel中只支持下面几种，另外支持vector、map，最多嵌套两层
//#define DOUBLE "double"			//													对应于c++ double
//#define FLOAT "float"			//													对应于c++ float
//#define INT32 "int32"			//使用可变长编码. 对于负数比较低效，如果负数较多，请使用sint32		对应于c++ int32
//#define INT64 "int64"			//使用可变长编码. 对于负数比较低效，如果负数较多，请使用sint64		对应于c++ int64
//#define UINT32 "uint32"			//使用可变长编码											对应于c++ uint32
//#define UINT64 "uint64"			//使用可变长编码											对应于c++ uint64
//#define SINT32 "sint32"			//使用可变长编码. Signed int value. 编码负数比int32更高效		对应于c++ int32
//#define SINT64 "sint64"			//使用可变长编码. Signed int value. 编码负数比int64更高效		对应于c++ int64
//#define FIXED32 "fixed32"		//恒定四个字节。如果数值几乎总是大于2的28次方，该类型比unit32更高效	对应于c++ uint32
//#define FIXED64 "fixed64"		//恒定四个字节。如果数值几乎总是大于2的56次方，该类型比unit64更高效	对应于c++ uint64
//#define SFIXED32 "sfixed32"		//恒定四个字节											对应于c++ int32
//#define SFIXED64 "sfixed64"		//恒定八个字节											对应于c++ int64
//#define BOOL "bool"				//													对应于c++ bool
//#define STRING "string"			//A string must always contain UTF-8 encoded or 7-bit ASCII text	对应于c++ string
//#define BYTES "bytes"			//包含任意数量顺序的字节									对应于c++ string

//#define VECTOR "vector"
//#define MAP "map"

//对于一个单元格表示复合数据类型来说，分别用逗号（ ，）、分号（;）、竖线（|）来分割，逗号分割最里层的，竖线分割最外层的


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

template<typename value_type>
void getSimpleValue(const Reflection* reflection, Message *msg, const FieldDescriptor* field_descriptor, const string& unit)
{
	value_type v;
	reflection->SetInt64(msg, field_descriptor, 100);
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
bool CommLoad<key, value>::load2Map(const string& conf_file, int32_t index, map<key, value>& map_out, getKeyFunc getKey)
{

	bool bRet = true;
	Book* book = xlCreateBook();
	Message *msg = NULL;

	__BEGIN_PROC__

	if(NULL == book)
	{
		CFG_LOG_ERROR("create xlCreateBook failed!");
		bRet = false;
		break;
	}

	if(book->load(conf_file.c_str()))
	{
		CFG_LOG_ERROR("load file %s failed!", conf_file.c_str());
		bRet = false;
		break;
	}

	Sheet* sheet = book->getSheet(index);

	if(NULL == sheet)
	{
		CFG_LOG_ERROR("getSheet failed, index:%d", index);
		bRet = false;
		break;
	}

	int32_t row_num = sheet->lastRow();
	int32_t col_num = sheet->lastCol();

	//获取message名称，固定位置，在第0行第0列单元格内
	const char* message = sheet->readStr(0,0);
	if(NULL == message)
	{
		CFG_LOG_ERROR("get message name failed!");
		bRet = false;
		break;
	}

	const string message_name("protocol." + string(message));

//	static map<string, int32_t> simple_type;
//	simple_type[FLOAT] = 1;
//	simple_type[INT32] = 1;
//	simple_type[INT64] = 1;
//	simple_type[UINT32] = 1;
//	simple_type[UINT64] = 1;
//	simple_type[BOOL] = 1;
//
//	static map<string, int32_t> complex_type;
//	complex_type[VECTOR] = 1;
//	complex_type[MAP] = 1;

	//从（2,1）开始，处理这一行的所有字段，解析字段中标明的message的字段类型和名字
	map<string, int32_t> fields;		//map<字段名，列号>
	for(int32_t col=1; col<col_num; ++col)
	{
		const char* f = sheet->readStr(2, col);
		if(NULL == f)
		{
			CFG_LOG_ERROR("empty unit. row:2 col:%d", col);
			break;
		}

		fields[string(f)] = col;
	}


	//创建message
	msg = createMessage(message_name);
	if (NULL == msg)
	{
		// 创建失败，可能是消息名错误，也可能是编译后message解析器
		// 没有链接到主程序中。
		CFG_LOG_ERROR("createMessage failed");
		bRet = false;
		break;
	}

	// 获取message的descriptor
	const Descriptor* descriptor = msg->GetDescriptor();
	// 获取message的反射接口，可用于获取和修改字段的值
	const Reflection* reflection = msg->GetReflection();

	//从第4行(row=3)开始，正式读取数据
	for(int32_t row=3; row<row_num; ++row)
	{
		for(int32_t i=0; i<descriptor->field_count(); ++i)
		{
			const FieldDescriptor* field_descriptor = descriptor->field(i);
			assert(NULL != field_descriptor);

			const string& strFieldName = field_descriptor->name();

//			enum CppType {
//			    CPPTYPE_INT32       = 1,     // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
//			    CPPTYPE_INT64       = 2,     // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
//			    CPPTYPE_UINT32      = 3,     // TYPE_UINT32, TYPE_FIXED32
//			    CPPTYPE_UINT64      = 4,     // TYPE_UINT64, TYPE_FIXED64
//			    CPPTYPE_DOUBLE      = 5,     // TYPE_DOUBLE
//			    CPPTYPE_FLOAT       = 6,     // TYPE_FLOAT
//			    CPPTYPE_BOOL        = 7,     // TYPE_BOOL
//			    CPPTYPE_ENUM        = 8,     // TYPE_ENUM
//			    CPPTYPE_STRING      = 9,     // TYPE_STRING, TYPE_BYTES
//			    CPPTYPE_MESSAGE     = 10,    // TYPE_MESSAGE, TYPE_GROUP
//
//			    MAX_CPPTYPE         = 10,    // Constant useful for defining lookup tables
//			                                 // indexed by CppType.
//			  };

			//找到该字段所在的列
			map<string, int32_t>::const_iterator it = fields.find(strFieldName);
			assert(it != fields.end());
			int32_t col = it->second;

			//读取单元格
			string unit = "";
			const char* pu = sheet->readStr(row, col);
			if(NULL != pu)
			{
				unit = string(pu);
			}

			FieldDescriptor::Label lable = field_descriptor->label();
			FieldDescriptor::CppType cpp_type = field_descriptor->cpp_type();

			switch(cpp_type)
			{

				case FieldDescriptor::CPPTYPE_INT32:
				{
					if(lable != FieldDescriptor::LABEL_REPEATED)
					{
						reflection->SetInt32(msg, field_descriptor, YAC_Common::strto<int32_t>(unit));
					}
					else
					{
						vector<int32_t> v = YAC_Common::sepstr<int32_t>(unit, ",");
						for(size_t i=0; i<v.size(); ++i)
						{
							reflection->SetRepeatedInt32(msg, field_descriptor, i, v[i]);
						}
					}
					break;
				}
				case FieldDescriptor::CPPTYPE_UINT32:
				{
					if(lable != FieldDescriptor::LABEL_REPEATED)
					{
						reflection->SetUInt32(msg, field_descriptor, YAC_Common::strto<uint32_t>(unit));
					}
					else
					{
						vector<uint32_t> v = YAC_Common::sepstr<uint32_t>(unit, ",");
						for(size_t i=0; i<v.size(); ++i)
						{
							reflection->SetRepeatedUInt32(msg, field_descriptor, i, v[i]);
						}
					}
					break;
				}
				case FieldDescriptor::CPPTYPE_INT64:
				{
					if(lable != FieldDescriptor::LABEL_REPEATED)
					{
						reflection->SetInt64(msg, field_descriptor, YAC_Common::strto<int64_t>(unit));
					}
					else
					{
						vector<int64_t> v = YAC_Common::sepstr<int64_t>(unit, ",");
						for(size_t i=0; i<v.size(); ++i)
						{
							reflection->SetRepeatedInt64(msg, field_descriptor, i, v[i]);
						}
					}
					break;
				}
				case FieldDescriptor::CPPTYPE_UINT64:
				{
					if(lable != FieldDescriptor::LABEL_REPEATED)
					{
						reflection->SetUInt64(msg, field_descriptor, YAC_Common::strto<uint64_t>(unit));
					}
					else
					{
						vector<uint64_t> v = YAC_Common::sepstr<uint64_t>(unit, ",");
						for(size_t i=0; i<v.size(); ++i)
						{
							reflection->SetRepeatedUInt64(msg, field_descriptor, i, v[i]);
						}
					}
					break;
				}
				case FieldDescriptor::CPPTYPE_STRING:
				{
					if(lable != FieldDescriptor::LABEL_REPEATED)
					{
						reflection->SetString(msg, field_descriptor, unit);
					}
					else
					{
						vector<string> v = YAC_Common::sepstr<string>(unit, ",");
						for(size_t i=0; i<v.size(); ++i)
						{
							reflection->SetRepeatedString(msg, field_descriptor, i, v[i]);
						}
					}
					break;
				}
				case FieldDescriptor::CPPTYPE_FLOAT:
				{
					if(lable != FieldDescriptor::LABEL_REPEATED)
					{
						reflection->SetFloat(msg, field_descriptor, YAC_Common::strto<float>(unit));
					}
					else
					{
						vector<float> v = YAC_Common::sepstr<float>(unit, ",");
						for(size_t i=0; i<v.size(); ++i)
						{
							reflection->SetRepeatedFloat(msg, field_descriptor, i, v[i]);
						}
					}
					break;
				}
				case FieldDescriptor::CPPTYPE_DOUBLE:
				{
					if(lable != FieldDescriptor::LABEL_REPEATED)
					{
						reflection->SetDouble(msg, field_descriptor, YAC_Common::strto<double>(unit));
					}
					else
					{
						vector<double> v = YAC_Common::sepstr<double>(unit, ",");
						for(size_t i=0; i<v.size(); ++i)
						{
							reflection->SetRepeatedDouble(msg, field_descriptor, i, v[i]);
						}
					}
					break;
				}
				case FieldDescriptor::CPPTYPE_MESSAGE:
				{
					break;
				}
				case FieldDescriptor::CPPTYPE_BOOL:
				{
					if(lable != FieldDescriptor::LABEL_REPEATED)
					{
						reflection->SetBool(msg, field_descriptor, YAC_Common::strto<bool>(unit));
					}
					else
					{
						vector<bool> v = YAC_Common::sepstr<bool>(unit, ",");
						for(size_t i=0; i<v.size(); ++i)
						{
							reflection->SetRepeatedBool(msg, field_descriptor, i, v[i]);
						}
					}
					break;
				}
				case FieldDescriptor::CPPTYPE_ENUM:
				{
					break;
				}
			}
		}

		value record = *((value*)msg);
		key map_key;

		map_key = getKey(record);

		map_out[map_key] = record;
	}


	__END_PROC__

	if(book)
	{
		book->release();
	}

	if(msg)
	{
		msg->Clear();
		delete msg;
	}

	return bRet;

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


