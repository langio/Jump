#ifndef __XC_DYN_OBJECT_H
#define __XC_DYN_OBJECT_H

#include <iostream>

namespace xutil
{
/////////////////////////////////////////////////
/** 
 * @file tc_dyn_object.h
 * @brief 动态生成类. 
 *  
*/             
/////////////////////////////////////////////////


/**
 * @brief 模拟MFC,动态生成类
 */
class XC_DYN_Object;
class XC_DYN_RuntimeClass;

struct XC_DYN_RuntimeClass
{
    const char* m_lpszClassName;
    int m_nObjectSize;
    XC_DYN_Object* (* m_pfnCreateObject)();
    XC_DYN_RuntimeClass* m_pBaseClass;
    XC_DYN_RuntimeClass* m_pNextClass;

    static XC_DYN_RuntimeClass* pFirstClass;

    XC_DYN_Object* createObject();

    static XC_DYN_RuntimeClass* load(const char *szClassName);    
};

struct XC_DYN_Init
{
    XC_DYN_Init(XC_DYN_RuntimeClass* pNewClass)
    {
        pNewClass->m_pNextClass = XC_DYN_RuntimeClass::pFirstClass;
        XC_DYN_RuntimeClass::pFirstClass = pNewClass;
    }
};

class XC_DYN_Object
{
public:
    XC_DYN_Object(){};
    virtual ~XC_DYN_Object(){};

    virtual XC_DYN_RuntimeClass* GetRuntimeClass() const;

    bool isKindOf(const XC_DYN_RuntimeClass* pClass) const;
private:

    XC_DYN_Object(const XC_DYN_Object& objectSrc);
    void operator=(const XC_DYN_Object& objectSrc);

public:
    static XC_DYN_RuntimeClass classXC_DYN_Object;
};

#define RUNTIME_CLASS(class_name) ((XC_DYN_RuntimeClass*)(&class_name::class##class_name))

#define DECLARE_DYNCREATE(class_name) \
public: \
    static XC_DYN_RuntimeClass class##class_name; \
    virtual XC_DYN_RuntimeClass* GetRuntimeClass() const; \
    static XC_DYN_Object* createObject();

#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
    XC_DYN_Object* class_name::createObject() \
        { return new class_name; } \
    XC_DYN_RuntimeClass class_name::class##class_name = { \
        #class_name, \
        sizeof(class_name), \
        &class_name::createObject, \
        RUNTIME_CLASS(base_class_name), \
        NULL }; \
    static XC_DYN_Init _init_##class_name(&class_name::class##class_name);   \
    XC_DYN_RuntimeClass* class_name::GetRuntimeClass() const \
        { return RUNTIME_CLASS(class_name); } 

#define XC_DYN_CreateObject(class_name) \
(XC_DYN_RuntimeClass::load(class_name) == NULL ? NULL : XC_DYN_RuntimeClass::load(class_name)->createObject())
    
}

#endif

