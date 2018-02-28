#include "stdafx.h"
#include "DB_TopRankLoadPlayer.h"
#include "../Common/TopType.h"
#include "DBContext.h"
#include "DBLayer/Data/TBLPlayer.h"
#include "TopManager.h"

DB_TopRankLoadPlayer::DB_TopRankLoadPlayer(void)
{
	
}

int DB_TopRankLoadPlayer::Execute(int ctxId,void* param)
{
	if(DB_CONTEXT_NOMRAL == ctxId)
	{

		CDBConn* pDBConn = (CDBConn*)param;
		assert(0 != pDBConn);

		TBLPlayer tb_player(0);
		tb_player.AttachConn(pDBConn);
		DBError err = DBERR_NONE;//tb_player.Load_BLOB(m_tarPlayerId,&m_pPlayer,false);	

		if(err != DBERR_NONE)
		{
			g_Log.WriteWarn("查询角色[%d]排行榜人物数据失败，原因:%s",m_tarPlayerId,getDBErr(err));
		}
		else
			g_Log.WriteLog("查询角色[%d]排行榜人物数据成功",m_tarPlayerId);

		PostLogicThread(this);
		return PACKET_NOTREMOVE;
	}
	else
	{
		//SERVER->GetTopManager()->SendPlayerEquip(m_socketHandle,m_srcPlayerId,m_pPlayer);
		return PACKET_OK;
	}
}