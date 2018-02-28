#ifndef _DBLOGINPLAYER_H_
#define _DBLOGINPLAYER_H_

#include "CommLib/ThreadPool.h"

class DB_LoginPlayer 
	:public ThreadBase
{
public:
	DB_LoginPlayer();

	virtual int Execute(int ctxId, void* param);

	char MachineUID[256];
	char IP[20];
	int account;
	int socketId;
	int areaId;		//ÇëÇóµÇÂ¼µÄ·þID

	bool loginAward;
	int playerId;
};

#endif /*_DBLOGINPLAYER_H_*/