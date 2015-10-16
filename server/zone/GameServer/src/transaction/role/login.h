#ifndef __LOGIN_H_
#define __LOGIN_H_

#include "base_transaction.h"

class Login : public BaseTransaction
{
public:
	Login(){};
	~Login(){};

	int32_t DoLogin();
};

#endif
