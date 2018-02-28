#ifndef _DB_OTHER_LOGIN_H_
#define _DB_OTHER_LOGIN_H_

#include "CommLib/ThreadPool.h"
#include "RemoteAccount/PollAccount.h"

class DB_OtherLogin
	:public ThreadBase
{
public:
	DB_OtherLogin(const PollInfo& info);

	virtual int Execute(int ctxId, void* param);

	PollInfo info;
};

#endif /*_DB_OTHER_LOGIN_H_*/