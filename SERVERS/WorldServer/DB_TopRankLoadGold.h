#ifndef _DBTOPRANKLOAD_H_
#define _DBTOPRANKLOAD_H_

#include <vector>
#include "CommLib/ThreadPool.h"
#include "DBLayer/Data/TBLGoldHistory.h"
#include "DBLayer/Data/TBLMailList.h"

class DB_TopRankLoadGold : public ThreadBase
{
public:
	DB_TopRankLoadGold(void);
	~DB_TopRankLoadGold();
	
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
	typedef stdext::hash_map<int,int> TOPGOLDCOST;             //玩家元宝消费量

	TOPGOLDCOST m_topGoldCostMonth;
	TOPGOLDCOST m_topGoldCostTotal;

};


#endif /*_DBSAVETOPRANK_H_*/