#include "util/tc_dyn_object.h"
#include <string.h>

namespace xutil
{
/**********************************XC_DYN_RuntimeClass定义***********************************/

XC_DYN_RuntimeClass* XC_DYN_RuntimeClass::pFirstClass = NULL;

XC_DYN_Object* XC_DYN_RuntimeClass::createObject()
{
    if (m_pfnCreateObject == NULL)
    {
        return NULL;
    }

    XC_DYN_Object* pObject = (*m_pfnCreateObject)();
    {
        return pObject;
    }
}

XC_DYN_RuntimeClass* XC_DYN_RuntimeClass::load(const char *szClassName)
{
    XC_DYN_RuntimeClass* pClass;

    for (pClass = pFirstClass; pClass != NULL; pClass = pClass->m_pNextClass)
    {
        if (strcmp(szClassName, pClass->m_lpszClassName) == 0)
        return pClass;
    }

    return NULL;
}

/**********************************szXC_DYN_Object定义***********************************/

XC_DYN_RuntimeClass XC_DYN_Object::classXC_DYN_Object =
{
    (char*)"XC_DYN_Object", 
    sizeof(XC_DYN_Object), 
    NULL, 
    NULL, 
    NULL 
};

static XC_DYN_Init _init_XC_DYN_Object(&XC_DYN_Object::classXC_DYN_Object);

XC_DYN_RuntimeClass* XC_DYN_Object::GetRuntimeClass() const
{
	return &XC_DYN_Object::classXC_DYN_Object;
}

bool XC_DYN_Object::isKindOf(const XC_DYN_RuntimeClass* pClass) const
{
    XC_DYN_RuntimeClass* pClassThis = GetRuntimeClass();
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
