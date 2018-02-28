#ifndef _DB_DELETEORG_H_
#define _DB_DELETEORG_H_

#include "CommLib/ThreadPool.h"

class DB_DeleteOrg 
	:public ThreadBase
{
public:
	DB_DeleteOrg();

	virtual int Execute(int ctxId, void* param);

	int id;
};

#endif /*_DB_DELETEORG_H_*/