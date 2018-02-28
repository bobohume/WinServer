#include "stdafx.h"
#include "DB_TopRankLoadGold.h"
#include "../Common/TopType.h"
#include "DBContext.h"


DB_TopRankLoadGold::DB_TopRankLoadGold(void)
{
    
}

DB_TopRankLoadGold::~DB_TopRankLoadGold(void)
{

}



int DB_TopRankLoadGold::Execute(int ctxId,void* param)
{
	if(DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		assert(0 != pDBConn);
	    
		TBLGoldHistory tb_goldHistory(0);
		tb_goldHistory.AttachConn(pDBConn);
		DBError err = tb_goldHistory.QueryGoldCostMonth(m_topGoldCostMonth);	

		/*
		if(err != DBERR_NONE)
		{
			g_Log.WriteError("排行榜 %s 读取goldHistory元宝数据失败，原因:%s",strTopRankType[eTopType_GoldCostMonth],getDBErr(err));
			return PACKET_ERROR;
		}
		else
			g_Log.WriteLog("排行榜 %s 读取goldHistory元宝数据成功!",strTopRankType[eTopType_GoldCostMonth]);

		err = tb_goldHistory.QueryGoldCostTotal(m_topGoldCostTotal);
		if(err != DBERR_NONE)
		{
			g_Log.WriteError("排行榜 %s 读取goldHistory元宝数据失败，原因:%s",strTopRankType[eTopType_GoldCostTotal],getDBErr(err));
			return PACKET_ERROR;
		}
		else
			g_Log.WriteLog("排行榜 %s 读取goldHistory元宝数据成功!",strTopRankType[eTopType_GoldCostTotal]);

		*/
		PostLogicThread(this);
		return PACKET_NOTREMOVE;

	}
	else
	{
		//SERVER->GetTopManager()->SetGoldMonthCost(m_topGoldCostMonth);
		//SERVER->GetTopManager()->SetGoldTotalCost(m_topGoldCostTotal);
	}
    
    return PACKET_OK;
}


