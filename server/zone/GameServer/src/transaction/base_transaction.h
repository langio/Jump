#ifndef __BASE_TRANSACTION_H_
#define __BASE_TRANSACTION_H_

class BaseTransaction
{
public:
	BaseTransaction():_finished(false){}
	virtual ~BaseTransaction(){}

protected:
	void SetFinished(){_finished = true;}

	bool IsFinished(){return _finished;}

private:
	bool _finished;
};

#endif

