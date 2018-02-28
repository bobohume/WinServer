#ifndef _DBSAVETOPRANK_H_
#define _DBSAVETOPRANK_H_

#include <vector>
#include "CommLib/ThreadPool.h"
#include "DBLayer/Data/TBLTopRank.h"

class DB_SaveTopRank : public ThreadBase
{
public:
	DB_SaveTopRank(void);
	
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    int m_iType;
    bool m_isEnded;
	 int m_iSaveBlobType;
    std::vector<TBLTopRank::stTopRank> m_toprankList;
};

#endif /*_DBSAVETOPRANK_H_*/