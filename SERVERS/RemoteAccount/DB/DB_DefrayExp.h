#ifndef _DB_DEFRAY_EXPERIENCE_H_
#define _DB_DEFRAY_EXPERIENCE_H_

#include "CommLib/ThreadPool.h"

class DB_DefrayExp
	:public ThreadBase
{
public:
	DB_DefrayExp();

	virtual int Execute(int ctxId, void* param);

	int socketID;
	int accountID;
	int playerID;
	int experience;
};

#endif /*_DB_DEFRAY_EXPERIENCE_H_*/