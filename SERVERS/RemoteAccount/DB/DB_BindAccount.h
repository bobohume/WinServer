#ifndef _DB_BINDACCOUNT_H_
#define _DB_BINDACCOUNT_H_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"

class DB_BindAccount 
	:public ThreadBase
{
public:
	DB_BindAccount();

	virtual int Execute(int ctxId, void* param);

	int account;
	char m_AccountName[50];
	char m_Password[32];
	int m_Error;
};

#endif /*_DBCREATEPLAYER_H_*/