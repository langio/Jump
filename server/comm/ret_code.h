#ifndef __RET_CODE_H_
#define __RET_CODE_H_

//#include <unistd.h>

const int32_t RET_SYS_ERR = -1;
const int32_t RET_OK = 0;

//100以下系统使用
const int32_t RET_REDIS_ERR_CONN =  1;				//连接redis时出错
const int32_t RET_REDIS_ERR_GET = 2;				//get时发出错误
const int32_t RET_REDIS_ERR_SET = 3;				//set时发出错误
const int32_t RET_REDIS_ERR_INCR = 4;				//incr时发生错误

//注册时相关错误码
const int32_t RET_REG_ERR_DUPLICATE_NAME = 101;			//该昵称已被使用

#endif
