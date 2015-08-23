#ifndef _SERVICE_DISPATCHER_H
#define _SERVICE_DISPATCHER_H

class dispatcher
{
public:
	static dispatcher& getInstance()
	{
		static dispatcher m_dispatcher;
		return m_dispatcher;
	};

	void dispatch(const void * msg, size_t sz);

private:
	dispatcher(){}
	~dispatcher(){}
};

#endif
