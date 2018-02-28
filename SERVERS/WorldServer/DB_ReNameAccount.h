#ifndef _DB_RENAME_ACCOUNT_H_
#define _DB_RENAME_ACCOUNT_H_

#include "CommLib/ThreadPool.h"

class DB_ReNameAccount : public ThreadBase
{
public:
    DB_ReNameAccount(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32 m_accountId;
	char m_AccountName[50];
};

#endif /*_DBUPDATEACCOUNTTIME_*/