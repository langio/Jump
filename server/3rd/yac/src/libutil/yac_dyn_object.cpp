#include "util/yac_dyn_object.h"
#include <string.h>

namespace util
{
/**********************************YAC_DYN_RuntimeClass定义***********************************/

YAC_DYN_RuntimeClass* YAC_DYN_RuntimeClass::pFirstClass = NULL;

YAC_DYN_Object* YAC_DYN_RuntimeClass::createObject()
{
    if (m_pfnCreateObject == NULL)
    {
        return NULL;
    }

    YAC_DYN_Object* pObject = (*m_pfnCreateObject)();
    {
        return pObject;
    }
}

YAC_DYN_RuntimeClass* YAC_DYN_RuntimeClass::load(const char *szClassName)
{
    YAC_DYN_RuntimeClass* pClass;

    for (pClass = pFirstClass; pClass != NULL; pClass = pClass->m_pNextClass)
    {
        if (strcmp(szClassName, pClass->m_lpszClassName) == 0)
        return pClass;
    }

    return NULL;
}

/**********************************szYAC_DYN_Object定义***********************************/

YAC_DYN_RuntimeClass YAC_DYN_Object::classYAC_DYN_Object =
{
    (char*)"YAC_DYN_Object", 
    sizeof(YAC_DYN_Object), 
    NULL, 
    NULL, 
    NULL 
};

static YAC_DYN_Init _init_YAC_DYN_Object(&YAC_DYN_Object::classYAC_DYN_Object);

YAC_DYN_RuntimeClass* YAC_DYN_Object::GetRuntimeClass() const
{
	return &YAC_DYN_Object::classYAC_DYN_Object;
}

bool YAC_DYN_Object::isKindOf(const YAC_DYN_RuntimeClass* pClass) const
{
    YAC_DYN_RuntimeClass* pClassThis = GetRuntimeClass();
    while (pClassThis != NULL)
    {
        if (pClassThis == pClass)
        {
            return true;
        }

        pClassThis = pClassThis->m_pBaseClass;
    }
    return false;
}

}
