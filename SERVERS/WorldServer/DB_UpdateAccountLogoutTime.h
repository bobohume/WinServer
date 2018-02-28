#ifndef _DBUPDATEACCOUNTLOGOUTTIME_
#define _DBUPDATEACCOUNTLOGOUTTIME_

#include "CommLib/ThreadPool.h"

class DB_UpdateAccountLogoutTime : public ThreadBase
{
public:
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32 m_accountId;
    int m_time;
};

#endif /*_DBUPDATEACCOUNTLOGOUTTIME_*/