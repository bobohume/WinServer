#ifndef _DBTOPRANKLOADPLAYER_H_
#define _DBTOPRANKLOADPLAYER_H_

#include <vector>
#include "CommLib/ThreadPool.h"
#include "DBLayer/Data/TBLTopRank.h"

class DB_TopRankLoadPlayer : public ThreadBase
{
public:
	DB_TopRankLoadPlayer(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);

	int m_tarPlayerId;
	int m_srcPlayerId;
	//stPlayerStruct m_pPlayer;
	int m_socketHandle;
};

#endif /*_DBSAVETOPRANK_H_*/