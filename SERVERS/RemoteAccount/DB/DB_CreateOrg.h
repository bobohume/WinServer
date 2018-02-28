#ifndef _DBCREATEORG_H_
#define _DBCREATEORG_H_

#include "CommLib/ThreadPool.h"

class DB_CreateOrg 
	:public ThreadBase
{
public:
	DB_CreateOrg();

	virtual int Execute(int ctxId, void* param);

	std::string name;
	std::string qq;
	std::string memo;
	bool autoAcceptJoin;
	char buffer[512];
	int account;
	int socketId;
};

#endif /*_DBCREATEORG_H_*/