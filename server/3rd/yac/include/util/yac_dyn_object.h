#ifndef __YAC_DYN_OBJECT_H
#define __YAC_DYN_OBJECT_H

#include <iostream>

namespace util
{
/////////////////////////////////////////////////
/** 
 * @file yac_dyn_object.h
 * @brief 动态生成类. 
 *  
*/             
/////////////////////////////////////////////////


/**
 * @brief 模拟MFC,动态生成类
 */
class YAC_DYN_Object;
class YAC_DYN_RuntimeClass;

struct YAC_DYN_RuntimeClass
{
    const char* m_lpszClassName;
    int m_nObjectSize;
    YAC_DYN_Object* (* m_pfnCreateObject)();
    YAC_DYN_RuntimeClass* m_pBaseClass;
    YAC_DYN_RuntimeClass* m_pNextClass;

    static YAC_DYN_RuntimeClass* pFirstClass;

    YAC_DYN_Object* createObject();

    static YAC_DYN_RuntimeClass* load(const char *szClassName);    
};

struct YAC_DYN_Init
{
    YAC_DYN_Init(YAC_DYN_RuntimeClass* pNewClass)
    {
        pNewClass->m_pNextClass = YAC_DYN_RuntimeClass::pFirstClass;
        YAC_DYN_RuntimeClass::pFirstClass = pNewClass;
    }
};

class YAC_DYN_Object
{
public:
    YAC_DYN_Object(){};
    virtual ~YAC_DYN_Object(){};

    virtual YAC_DYN_RuntimeClass* GetRuntimeClass() const;

    bool isKindOf(const YAC_DYN_RuntimeClass* pClass) const;
private:

    YAC_DYN_Object(const YAC_DYN_Object& objectSrc);
    void operator=(const YAC_DYN_Object& objectSrc);

public:
    static YAC_DYN_RuntimeClass classYAC_DYN_Object;
};

#define RUNTIME_CLASS(class_name) ((YAC_DYN_RuntimeClass*)(&class_name::class##class_name))

#define DECLARE_DYNCREATE(class_name) \
public: \
    static YAC_DYN_RuntimeClass class##class_name; \
    virtual YAC_DYN_RuntimeClass* GetRuntimeClass() const; \
    static YAC_DYN_Object* createObject();

#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
    YAC_DYN_Object* class_name::createObject() \
        { return new class_name; } \
    YAC_DYN_RuntimeClass class_name::class##class_name = { \
        #class_name, \
        sizeof(class_name), \
        &class_name::createObject, \
        RUNTIME_CLASS(base_class_name), \
        NULL }; \
    static YAC_DYN_Init _init_##class_name(&class_name::class##class_name);   \
    YAC_DYN_RuntimeClass* class_name::GetRuntimeClass() const \
        { return RUNTIME_CLASS(class_name); } 

#define YAC_DYN_CreateObject(class_name) \
(YAC_DYN_RuntimeClass::load(class_name) == NULL ? NULL : YAC_DYN_RuntimeClass::load(class_name)->createObject())
    
}

#endif

