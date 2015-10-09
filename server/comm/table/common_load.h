#ifndef __COMMON_LOAD_H_
#define __COMMON_LOAD_H_

#include "libxl.h"
#include "comm_def.h"
#include "conf_table.pb.h"
//#include <google/protobuf/stubs/common.h>
//#include <google/protobuf/generated_message_util.h>
//#include <google/protobuf/message.h>
//#include <google/protobuf/repeated_field.h>
//#include <google/protobuf/extension_set.h>
//#include <google/protobuf/unknown_field_set.h>
#include <stdio.h>
#include <fstream>
#include "util/yac_common.h"


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

    static bool loadCSV2Map(const string& csv_file, getKeyFunc getKey = defaultGetKey);

    static void printMap();

private:
    static key defaultGetKey(const value& v);

    static map<key, value> _conf;
};

template<typename key, typename value>
key CommLoad<key, value>::defaultGetKey(const value& v)
{
    return v.id;
}

template<typename key, typename value>
map<key, value> CommLoad<key, value>::_conf;

template<typename key, typename value>
bool CommLoad<key, value>::loadCSV2Map(const string& csv_file, getKeyFunc getKey)
{

	bool bRet = true;

	__TRY__

	Message* msg = NULL;

	__BEGIN_PROC__

	char cur_path[256]={0};
	if(getcwd(cur_path, sizeof(cur_path)) != NULL)
	{
		CFG_LOG_ERROR("path:%s\n", cur_path);
	}
	string full_csv_file = string(cur_path) + "/" + csv_file;

	ifstream fin(full_csv_file.c_str());
	if(!fin.is_open() || fin.bad())
	{
		CFG_LOG_ERROR("open file %s failed!", full_csv_file.c_str());
		bRet = false;
		break;
	}

	int32_t row = 1;

	//获取message名称，固定位置，在第一行
	string line;
	if(getline(fin, line) == 0)
	{
		CFG_LOG_ERROR("empty file %s", full_csv_file.c_str());
		bRet = false;
		break;
	}
	++row;

	vector<string> vUnits = YAC_Common::sepstr<string>(line, ",");
	if(vUnits.size() == 0)
	{
		CFG_LOG_ERROR("invalid file %s", full_csv_file.c_str());
		bRet = false;
		break;
	}
	const string message_name("protocol." + vUnits[0]);

	//第二行的字段描述是给策划看的
	if(getline(fin, line) == 0)
	{
		CFG_LOG_ERROR("file %s has no planner fields!", full_csv_file.c_str());
		bRet = false;
		break;
	}
	CFG_LOG_ERROR("row:%d %s", row, line.c_str());
	++row;

	//获取字段名，字段名放在第三行
	if(getline(fin, line) == 0)
	{
		CFG_LOG_ERROR("file %s has no programer fields!", full_csv_file.c_str());
		bRet = false;
		break;
	}
	CFG_LOG_ERROR("row:%d %s", row, line.c_str());
	++row;

	vUnits = YAC_Common::sepstr<string>(line, ",");
	if(vUnits.size() == 0)
	{
		CFG_LOG_ERROR("invalid file %s", full_csv_file.c_str());
		bRet = false;
		break;
	}

	map<string, int32_t> mFields;		//map<字段名，列号>
	for(size_t i=0; i<vUnits.size(); ++i)
	{
		mFields[vUnits[i]] = i;

		CFG_LOG_ERROR("field:%s col:%lu", vUnits[i].c_str(), i);
	}


	//创建message
	msg = createMessage(message_name);
	if (NULL == msg)
	{
		// 创建失败，可能是消息名错误，也可能是编译后message解析器
		// 没有链接到主程序中。
		CFG_LOG_ERROR("createMessage failed! msg_name:%s", message_name.c_str());
		bRet = false;
		break;
	}

	// 获取message的descriptor
	const Descriptor* descriptor = msg->GetDescriptor();
	// 获取message的反射接口，可用于获取和修改字段的值
	const Reflection* reflection = msg->GetReflection();

	//从第四行开始是数据
	while(getline(fin, line))
	{
		vector<string> vUnits = YAC_Common::sepstr<string>(line, ",", true);

		if(vUnits.size() < mFields.size())
		{
			CFG_LOG_ERROR("file %s content is invalid. row:%d", full_csv_file.c_str(), row);
			bRet = false;
			break;
		}

		for(int32_t i=0; i<descriptor->field_count(); ++i)
		{
			const FieldDescriptor* field_descriptor = descriptor->field(i);
			assert(NULL != field_descriptor);

			const string& strFieldName = field_descriptor->name();

			//找到该字段所在的列
			map<string, int32_t>::const_iterator it = mFields.find(strFieldName);

//			CFG_LOG_ERROR("row:%d field:%s", row, strFieldName.c_str());

			assert(it != mFields.end());
			int32_t col = it->second;

			//读取单元格
			string unit = vUnits[col];

			FieldDescriptor::Label lable = field_descriptor->label();
			FieldDescriptor::CppType cpp_type = field_descriptor->cpp_type();

			setValue(reflection, msg, field_descriptor, lable, cpp_type, unit, 2);
		}

		value* realType = dynamic_cast<value*>(msg);
		value record = *(realType);
		key map_key;

		map_key = getKey(record);

		_conf[map_key] = record;

		++row;
	}

	__END_PROC__

	if(msg)
	{
		msg->Clear();
		delete msg;
	}

	return bRet;

	__CATCH_INFO(csv_file.c_str())

	return false;

}


template<typename key, typename value>
void CommLoad<key, value>::printMap()
{
	typename map<key, value>::const_iterator it = _conf.begin();
	for(; it != _conf.end(); ++it)
	{
		cout << "key:" << it->first << endl;
		cout << "value:" << endl << it->second.DebugString() << endl;
	}
}

#endif

