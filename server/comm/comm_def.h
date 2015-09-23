#ifndef __COMM_DEF_H_
#define __COMM_DEF_H_

#include <string>
#include <iostream>
#include "util/yac_common.h"

using namespace std;

#define __BEGIN_PROC__  do{
#define __END_PROC__ }while(0);

#define LOG_ERROR(ctx, msg, args...) skynet_error(ctx, msg, ##args)


// 捕捉异常
#define __TRY__ try\
{

// 处理异常
#define __CATCH__ }\
catch (std::exception const& e)\
{\
    printf("exception:%s|%s|%d\n",e.what(), __FILE__, __LINE__);\
}\
catch (...)\
{\
    printf("catch unknown exception|%s|%d\n", __FILE__, __LINE__);\
}

#define __CATCH_INFO(info) }\
catch (std::exception const& e)\
{\
    printf("exception:%s|%s|%d|%s\n",e.what(), __FILE__, __LINE__, info);\
}\
catch (...)\
{\
    printf("catch unknown exception|%s|%d|%s\n", __FILE__, __LINE__, info);\
}

// 常用缩写
#define I2S(i) YAC_Common::tostr<int>(i)
#define U2S(i) YAC_Common::tostr<unsigned>(i)
#define LL2S(i) YAC_Common::tostr<long long>(i)
#define S2I(s) YAC_Common::strto<int>(s)
#define S2U(s) YAC_Common::strto<unsigned>(s)
#define S2LL(s) YAC_Common::strto<long long>(s)
#define S2B(s) ((s == "Y") ? true : false)
#define B2S(b) (b ? "Y" : "N")
#define SEPSTR(s1, s2) YAC_Common::sepstr<string>(s1, s2)
#define SEPSTR_T(s1, s2) YAC_Common::sepstr<string>(s1, s2, true)
#define SEPINT(s1, s2) YAC_Common::sepstr<int>(s1, s2)
#define TRIM(s) YAC_Common::trim(s)
#define BIN2S(s) YAC_Common::bin2str(s)
#define PRT(format, args...)	printf(format, ##args)

// 通用函数类
class CommFunc
{

public:

	template<typename T>
	static int32_t sizeOf(const T& t)
	{
		return sizeof(t) / sizeof(T);
	}


};

#endif
