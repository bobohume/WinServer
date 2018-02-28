#ifndef _DBQUERYPLAYERCOUNT_H_
#define _DBQUERYPLAYERCOUNT_H_

#include <vector>
#include "CommLib/ThreadPool.h"

class DB_QueryPlayerCount : public ThreadBase
{
public:
	DB_QueryPlayerCount(void);
	
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    int m_count;
};

#endif /*_DBQUERYPLAYERCOUNT_H_*/