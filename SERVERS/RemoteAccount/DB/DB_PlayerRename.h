#ifndef _DB_PLAYERERNAME_H_
#define _DB_PLAYERERNAME_H_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"

class DB_PlayerRename
	:public ThreadBase
{
public:
	DB_PlayerRename();

	virtual int Execute(int ctxId, void* param);

	int accountID;
	int playerID;
	int socketId;
	char name[COMMON_STRING_LENGTH];
};

#endif /*_DBCREATEPLAYER_H_*/