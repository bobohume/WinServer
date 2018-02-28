#ifndef __DBBONUSACTIVEFLAG_H__
#define __DBBONUSACTIVEFLAG_H__

#include "CommLib/ThreadPool.h"

class DB_BonusActiveFlag:public ThreadBase
{
public:
	DB_BonusActiveFlag();
	virtual int Execute(int ctxId, void* param);
	int PlayerId;
	int ActiveFlag;
	int AccountId;
};

#endif //__DBBONUSACTIVEFLAG_H__