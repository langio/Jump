#ifndef __COMMON_LOAD_H_
#define __COMMON_LOAD_H_

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


//对于一个单元格表示复合数据类型来说，分别用逗号（ ，）、分号（;）、竖线（|）来分割，逗号分割最里层的，竖线分割最外层的

Message* createMessage(const string &typeName);
void setValue(const Reflection* reflection, Message *msg, const FieldDescriptor* field_descriptor, FieldDescriptor::Label lable, FieldDescriptor::CppType cpp_type, const string& unit, int32_t depth);

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
bool CommLoad<key, value>::load2Map(const string& excel_file, int32_t index, map<key, value>& map_out, getKeyFunc getKey)
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

	if(book->load(excel_file.c_str()))
	{
		CFG_LOG_ERROR("load file %s failed!", excel_file.c_str());
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

			setValue(reflection, msg, field_descriptor, lable, cpp_type, unit, 1);
		}

		value* realType = dynamic_cast<value*>(msg);
		value record = *(realType);
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

