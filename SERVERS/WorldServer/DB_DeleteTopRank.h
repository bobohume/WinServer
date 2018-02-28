#ifndef _DBDELETETOPRANK_H_
#define _DBDELETETOPRANK_H_

#include <vector>
#include "CommLib/ThreadPool.h"
#include "DBLayer/Data/TBLTopRank.h"

class DB_DeleteTopRank : public ThreadBase
{
public:
	DB_DeleteTopRank(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);

	int m_playerId;
    int m_type;
	bool m_isAllClear;
	U64 m_uid;
};

#endif /*_DBSAVETOPRANK_H_*/