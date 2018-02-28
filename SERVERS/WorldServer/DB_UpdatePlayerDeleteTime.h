#ifndef _DBUPDATEPLAYERDELETETIME_
#define _DBUPDATEPLAYERDELETETIME_

#include "CommLib/ThreadPool.h"

class DB_UpdatePlayerDeleteTime : public ThreadBase
{
public:
    DB_UpdatePlayerDeleteTime(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32 m_playerId;
    int m_time;
};

#endif /*_DBSETACCOUNTSTATUS_*/