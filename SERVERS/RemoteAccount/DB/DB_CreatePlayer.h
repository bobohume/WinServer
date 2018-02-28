#ifndef _DBCREATEPLAYER_H_
#define _DBCREATEPLAYER_H_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"

class DB_CreatePlayer 
	:public ThreadBase
{
public:
	DB_CreatePlayer();

	virtual int Execute(int ctxId, void* param);

	int areaId;
	int account;
	int socketId;
	CreatePlayerInfo info;
};

#endif /*_DBCREATEPLAYER_H_*/