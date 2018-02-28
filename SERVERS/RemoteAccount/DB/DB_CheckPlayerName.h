#ifndef _DBCHECKPLAYERNAME_H_
#define _DBCHECKPLAYERNAME_H_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"

class DB_CheckPlayerName 
	:public ThreadBase
{
public:
	DB_CheckPlayerName();

	virtual int Execute(int ctxId, void* param);

	int areaId;
	int account;
	int socketId;
	char m_CreateName[COMMON_STRING_LENGTH];
};

#endif /*_DBCREATEPLAYER_H_*/