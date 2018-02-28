#ifndef _DBONLOGIN_H_
#define _DBONLOGIN_H_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"
#include "../PollAccount.h"

class DB_OnLogin:public ThreadBase
{
public:
	DB_OnLogin(const PollInfo& info);
	virtual int Execute(int ctxId, void* param);
	PollInfo info;
};

#endif /*_DBONLOGIN_H_*/