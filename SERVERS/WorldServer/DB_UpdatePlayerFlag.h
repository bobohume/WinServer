#ifndef _DB_UPDATEPLAYERFLAG_H_
#define _DB_UPDATEPLAYERFLAG_H_

#include "CommLib/ThreadPool.h"

class DB_UpdatePlayerFlag : public ThreadBase
{
public:
    DB_UpdatePlayerFlag(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32 mPlayerId;
	U32 mPlayerFlag;
};

#endif /*_DBUPDATEACCOUNTTIME_*/