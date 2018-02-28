#ifndef _DBDRAWGOLD_H_
#define _DBDRAWGOLD_H_

#include "CommLib/ThreadPool.h"

class DB_DrawGold 
	:public ThreadBase
{
public:
	DB_DrawGold();

	virtual int Execute(int ctxId, void* param);

	int AccountId;
	int PlayerId;
	int TakeGold;
	int socketId;
};

#endif /*_DBDRAWGOLD_H_*/