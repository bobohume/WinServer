#include "stdafx.h"
#include "DB_LoadTopRank.h"
#include "../Common/TopType.h"
#include "DBContext.h"

DB_LoadTopRank::DB_LoadTopRank(void)
{
    m_iType = 0;
	m_iLoadBlobType = -1;
}

int DB_LoadTopRank::Execute(int ctxId,void* param)
{
	if(DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		assert(0 != pDBConn);
	    
		TBLTopRank tb_toprank(0);
		tb_toprank.AttachConn(pDBConn);
		DBError err = tb_toprank.Load(m_iType,m_toprankList,m_iLoadBlobType);	

		if(err != DBERR_NONE)
			g_Log.WriteWarn("载入TopRank 类型%s 数据失败,原因:%s",strTopRankType[m_iType],getDBErr(err));
		else
		{
			//g_Log.WriteLog("载入TopRank 类型%s 数据成功",strTopRankType[m_iType]);

			TBLPlayer tb_player(0);
			tb_player.AttachConn(pDBConn);
			SimplePlayerData player;
			int playerId;
			DBError err;

			for (std::vector<TBLTopRank::stTopRank>::iterator it = m_toprankList.begin(); it != m_toprankList.end();)
			{
				playerId = it->PlayerID;
				err = tb_player.Load_Simple(playerId,&player);
				if (err != DBERR_NONE)
				{
					it = m_toprankList.erase(it);
					tb_toprank.Delete(playerId);
					continue;
				}

				strcpy_s(it->PlayerName,MAX_NAME_LENGTH,player.PlayerName);
				
				/*if(__isNeedFilt(m_iType) && playerId != 0)
					m_palyerStruct[playerId] = player;*/

				++it;				
			}
		}

		PostLogicThread(this);
		return PACKET_NOTREMOVE;
	}
	else
	{
		SERVER->GetTopManager()->Insert(m_iType,m_toprankList);
		// 现在不在排行版里面缓存一份玩家结构
		// SERVER->GetTopManager()->AddPlayerIniteMap(m_palyerStruct);
	}
    return PACKET_OK;
}

bool DB_LoadTopRank::__isNeedFilt(int type)
{
	/*if ((type >= eTopType_PetPower && type <= eTopType_Item3) || type == eTopType_PetStrengthen)   //奇珍异兽
		return false;
	if ( (type >= eTopType_HonorOrg && type <= eTopType_ActivityOrg) || (type >= eTopType_MoneyOrg && type <= eTopType_assaultVictoryOrg) )   //家族相关
		return false;
	if (type == eTopType_AllFamily_total)   //门宗综合
		return false;
	if(type >= eTopType_Score && type <= eTopType_Honuor)  //用不到的
		return false;*/
	return true;
}