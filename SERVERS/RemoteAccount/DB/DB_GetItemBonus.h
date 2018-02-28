#ifndef _DBGETITEMBONUS_H_
#define _DBGETITEMBONUS_H_

#include "CommLib/ThreadPool.h"

class DB_GetItemBonus 
	:public ThreadBase
{
public:
	DB_GetItemBonus();

	virtual int Execute(int ctxId, void* param);

	int iPlayerId;
	int iUId;
	int iBonusType;
	int iAccountId;
	int socketId;
};

#endif /*_DBGETITEMBONUS_H_*/