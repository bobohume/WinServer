#ifndef _DB_USECODE_H_
#define _DB_USECODE_H_

#include "CommLib/ThreadPool.h"
#include "Common/BonusBase.h"

class DB_UseCode 
	:public ThreadBase
{
public:
	DB_UseCode();

	virtual int Execute(int ctxId, void* param);

	int accountID;
	int playerID;
	char sn[MAX_BONUS_CODESTRING];
	int itemID;
	int itemCount;
};

#endif /*_DB_USECODE_H_*/