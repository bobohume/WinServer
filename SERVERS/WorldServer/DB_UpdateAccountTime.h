#ifndef _DBUPDATEACCOUNTTIME_
#define _DBUPDATEACCOUNTTIME_

#include "CommLib/ThreadPool.h"

class DB_UpdateAccountTime : public ThreadBase
{
public:
    DB_UpdateAccountTime(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32 m_accountId;
    int m_lastTime;
    int m_dTotalOfflineTime;
    int m_dTotalOnlineTime;
};

#endif /*_DBUPDATEACCOUNTTIME_*/