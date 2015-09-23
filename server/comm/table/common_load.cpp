#include "common_load.h"

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

void setValue(const Reflection* reflection, Message *msg, const FieldDescriptor* field_descriptor, FieldDescriptor::Label lable, FieldDescriptor::CppType cpp_type, const string& unit, int32_t depth)
{
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
