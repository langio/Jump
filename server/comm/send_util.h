#ifndef __SEND_UTIL_H_
#define __SEND_UTIL_H_

#include <google/protobuf/message.h>
#include "comm_def.h"
#include "public.h"

const int32_t MAX_BUFF = 102400;

bool sendToClinet(const PkgHead& pkg_head, const google::protobuf::Message& message);

#endif

