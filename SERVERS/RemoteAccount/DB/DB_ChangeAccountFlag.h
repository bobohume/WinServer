#ifndef _DB_CHANGE_ACCOUNT_FLAG_H_
#define _DB_CHANGE_ACCOUNT_FLAG_H_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"

class DB_ChangeAccountFlag
	:public ThreadBase
{
public:
	DB_ChangeAccountFlag();

	virtual int Execute(int ctxId, void* param);

	int accountID;
};

#endif /*_DB_CHANGE_ACCOUNT_FLAG_H_*/