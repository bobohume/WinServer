#ifndef _DB_DELETELEAGUE_H_
#define _DB_DELETELEAGUE_H_

#include "CommLib/ThreadPool.h"

class DB_DeleteLeague
	:public ThreadBase
{
public:
	DB_DeleteLeague();

	virtual int Execute(int ctxId, void* param);

	int id;
};

#endif /*_DB_DELETELEAGUE_H_*/