#include <string>
#include <iostream>

using namespace std;

#define __BEGIN_PROC__  do{
#define __END_PROC__ }while(0);

#define LOG_ERROR(ctx, msg, args...) skynet_error(ctx, msg, ##args)

