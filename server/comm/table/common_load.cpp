#include "common_load.h"

//复合字段中的分割副不使用逗号(,)、冒号(:)、横线(-)和斜杠(/)，因为csv文件适用逗号分割不同字段，日期格式的字段中可能包含冒号(:)、横线(-)和斜杠(/)
const string separator1 = ";";
const string separator2 = "|";
const string separator3 = "\\";

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
	reflection->ClearField(msg, field_descriptor);

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
				vector<int32_t> v = YAC_Common::sepstr<int32_t>(unit, separator1);
				for(size_t i=0; i<v.size(); ++i)
				{
					reflection->AddInt32(msg, field_descriptor, v[i]);
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
				vector<uint32_t> v = YAC_Common::sepstr<uint32_t>(unit, separator1);
				for(size_t i=0; i<v.size(); ++i)
				{
					reflection->AddUInt32(msg, field_descriptor, v[i]);
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
				vector<int64_t> v = YAC_Common::sepstr<int64_t>(unit, separator1);
				for(size_t i=0; i<v.size(); ++i)
				{
					reflection->AddInt64(msg, field_descriptor, v[i]);
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
				vector<uint64_t> v = YAC_Common::sepstr<uint64_t>(unit, separator1);
				for(size_t i=0; i<v.size(); ++i)
				{
					reflection->AddUInt64(msg, field_descriptor, v[i]);
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
				vector<string> v = YAC_Common::sepstr<string>(unit, separator1);
				for(size_t i=0; i<v.size(); ++i)
				{
					reflection->AddString(msg, field_descriptor, v[i]);
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
				vector<float> v = YAC_Common::sepstr<float>(unit, separator1);
				for(size_t i=0; i<v.size(); ++i)
				{
					reflection->AddFloat(msg, field_descriptor, v[i]);
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
				vector<double> v = YAC_Common::sepstr<double>(unit, separator1);
				for(size_t i=0; i<v.size(); ++i)
				{
					reflection->AddDouble(msg, field_descriptor, v[i]);
				}
			}
			break;
		}
		case FieldDescriptor::CPPTYPE_MESSAGE:
		{
			string separator = separator2;
			if(3 == depth)
			{
				separator = separator3;
			}

			vector<string> v = YAC_Common::sepstr<string>(unit, separator2);
			size_t field_count = field_descriptor->message_type()->field_count();
			assert(v.size() == field_count);

			if(lable != FieldDescriptor::LABEL_REPEATED)
			{
				//按顺序依次对应字段
				for (size_t i = 0; i < field_count; i++)
				{
					Message* m = reflection->MutableMessage(msg, field_descriptor, MessageFactory::generated_factory());
					const Reflection* ref = m->GetReflection();
					const FieldDescriptor* f = field_descriptor->message_type()->field(i);

					FieldDescriptor::Label l = f->label();
					FieldDescriptor::CppType c = f->cpp_type();

					setValue(ref, m, f, l, c, v[i], depth + 1);
				}
			}
			else
			{

			}


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
				vector<bool> v = YAC_Common::sepstr<bool>(unit, separator1);
				for(size_t i=0; i<v.size(); ++i)
				{
					reflection->AddBool(msg, field_descriptor, v[i]);
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
