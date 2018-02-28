#include "TopApprentice.h"
#include "DBLayer/Data/TBLMPList.h"
#include "DBLib\dbPool.h"

typedef stdext::hash_map<int,int> CACHE;

CtopApprentice::CtopApprentice()
{
	readDB();
}

CtopApprentice::~CtopApprentice()
{

}

void CtopApprentice::readDB()
{
	MasterCount mCount;
	TBLMPList tb_mplist(SERVER_CLASS_NAME::GetActorDB());
	DBError err = tb_mplist.Count(mCount);

	if(err != DBERR_NONE)
	{
		g_Log.WriteError("获取出师人数数据失败,原因:%s",getDBErr(err));
		return;
	}
	
	for(size_t i = 0; i < mCount.size(); i++)
	{
		int playerId = mCount[i].first;
		int num = mCount[i].second;

		m_topApprentice[playerId] = num;
	}

}


CACHE CtopApprentice::getApprtcMap()
{
	return m_topApprentice;
}

void CtopApprentice::updateMap(int playerId, int num)
{
	m_topApprentice[playerId] = num;
}