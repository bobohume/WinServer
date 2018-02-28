#ifndef _DBQUERYBONUS_H_
#define _DBQUERYBONUS_H_

#include "CommLib/ThreadPool.h"

class DB_QueryBonus 
	:public ThreadBase
{
public:
	DB_QueryBonus();

	virtual int Execute(int ctxId, void* param);

	int playerId;
	int iBonusType;
	int iAccountId;
	int socketId;
};

#endif /*_DBGETITEMBONUS_H_*/