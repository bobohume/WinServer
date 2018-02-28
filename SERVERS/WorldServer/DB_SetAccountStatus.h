#ifndef _DBSETACCOUNTSTATUS_
#define _DBSETACCOUNTSTATUS_

#include "CommLib/ThreadPool.h"

class DB_SetAccountStatus : public ThreadBase
{
public:
    DB_SetAccountStatus(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32 m_accountId;
    int m_status;
};

#endif /*_DBSETACCOUNTSTATUS_*/