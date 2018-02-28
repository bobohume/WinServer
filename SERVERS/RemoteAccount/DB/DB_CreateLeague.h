#ifndef _DBCREATELEAGUE_H_
#define _DBCREATELEAGUE_H_

#include "CommLib/ThreadPool.h"

class DB_CreateLeague
	:public ThreadBase
{
public:
	DB_CreateLeague();

	virtual int Execute(int ctxId, void* param);

	char name[COMMON_STRING_LENGTH];
	int areaId;
	int account;
	char buffer[256];
	int socketId;
};

#endif /*_DBCREATELEAGUE_H_*/