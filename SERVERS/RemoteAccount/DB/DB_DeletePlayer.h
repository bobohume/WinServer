#ifndef _DBDELETEPLAYER_H_
#define _DBDELETEPLAYER_H_

#include "CommLib/ThreadPool.h"

class DB_DeletePlayer 
	:public ThreadBase
{
public:
	DB_DeletePlayer();

	virtual int Execute(int ctxId, void* param);

	int id;
};

#endif /*_DBDELETEPLAYER_H_*/