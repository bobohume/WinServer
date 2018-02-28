#ifndef _DBQUERYGOLD_H_
#define _DBQUERYGOLD_H_

#include "CommLib/ThreadPool.h"

class DB_QueryGold 
	:public ThreadBase
{
public:
	DB_QueryGold();

	virtual int Execute(int ctxId, void* param);

	int AccountId;
	int PlayerId;
	int socketId;
};

class DB_QueryMonthPays 
	:public ThreadBase
{
public:
	DB_QueryMonthPays();

	virtual int Execute(int ctxId, void* param);

	int AccountId;
	int PlayerId;
	int socketId;
	bool isDraw;
	char PlayerName[32];
};

#endif /*_DBQUERYGOLD_H_*/