#ifndef _DB_THIRD_ACTIVATE_H_
#define _DB_THIRD_ACTIVATE_H_

#include "CommLib/ThreadPool.h"
#include "RemoteAccount/PollAccount.h"

class DB_ThirdActivate 
	:public ThreadBase
{
public:
	DB_ThirdActivate(const PollInfo& info);

	virtual int Execute(int ctxId, void* param);

	PollInfo info;
};

#endif /*_DB_THIRD_ACTIVATE_H_*/