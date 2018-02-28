#ifndef _DBGMQUERYACCOUNTGOLD_H_
#define _DBGMQUERYACCOUNTGOLD_H_

#include "CommLib/ThreadPool.h"

class DB_GMQueryAccountGold 
	:public ThreadBase
{
public:
	DB_GMQueryAccountGold();

	virtual int Execute(int ctxId, void* param);

	int AccountId;
	int sid;
	int GMAccountID;
	int socketId;
};

// class DB_GMUpdateAccountGold 
// 	:public ThreadBase
// {
// public:
// 	DB_GMUpdateAccountGold();
// 
// 	virtual int Execute(int ctxId, void* param);
// 
// 	int AccountId;
// 	int sid;
// 	int GMAccountID;
// 	int socketId;
// 	int gold;
// };

#endif /*_DBGMQUERYACCOUNTGOLD_H_*/