#ifndef _DBLOADTOPRANK_H_
#define _DBLOADTOPRANK_H_

#include <vector>
#include <hash_map>
#include "CommLib/ThreadPool.h"
#include "DBLayer/Data/TBLTopRank.h"
#include "DBLayer/Data/TBLPlayer.h"

class DB_LoadTopRank : public ThreadBase
{
public:
	DB_LoadTopRank(void);
	
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
	bool __isNeedFilt(int type);
   
	int m_iType;
	//bool m_isPlayer;          //判断主键是否玩家，用来判断是否载入玩家blob数据
	int m_iLoadBlobType;       //如果有读取blob数据，区分是宠物还是道具

	std::vector<TBLTopRank::stTopRank> m_toprankList;
	//stdext::hash_map<int,stPlayerStruct> m_palyerStruct;
};

#endif /*_DBSAVETOPRANK_H_*/