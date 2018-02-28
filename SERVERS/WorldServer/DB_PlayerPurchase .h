#ifndef _DB_Player_Purchase_H_
#define _DB_Player_Purchase_H_

#include "CommLib/ThreadPool.h"

class DB_PlayerPurchase : public ThreadBase
{
public:
	DB_PlayerPurchase();
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);

	U32 nPlayerId;
	U32 nGold;
	U32 nMoney;
};

#endif /*_DBSETACCOUNTSTATUS_*/