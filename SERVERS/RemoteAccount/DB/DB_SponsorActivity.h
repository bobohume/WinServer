#ifndef _DB_SPONSOR_ACTIVITY_H_
#define _DB_SPONSOR_ACTIVITY_H_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"

class DB_SponsorActivity
	: public ThreadBase
{
public:
	DB_SponsorActivity();

	virtual int Execute(int ctxId, void* param);

	int socketID;
	int accountID;
	int playerID;
	SPONSOR_TYPE sponsor;
};

#endif /*_DB_SPONSOR_ACTIVITY_H_*/