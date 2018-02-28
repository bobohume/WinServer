#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "Common/PacketType.h"
#include "DBUtility.h"
#include "TBLPromotion.h"
#include "Common/Common.h"

// ----------------------------------------------------------------------------
// 检查更新推广员奖励
DBError TBLPromotion::UpdateBonus(int PROMAccountId, std::vector<int>& AccountList)
{	
	if(AccountList.empty())
		return DBERR_NONE;

	DBError err = DBERR_UNKNOWERR;
	char szStr[20];
	char szAccount[8000] = {0,};
	for(size_t i = 0; i < AccountList.size(); i++)
	{
		if( i > 0)
		{
			strcat_s(szAccount, 8000, ",");
		}

		_itoa_s( AccountList[i], szStr, 20, 10 );
		strcat_s(szAccount, 8000, szStr);

		if(i > MAX_QUERY_PRESENTEE)
		{
			g_Log.WriteError("此推广员(AccountId=%d)下线人数超过600人,", PROMAccountId);
			break;
		}
	}

	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_UpdatePromotionBonus %d,'%s'",
			PROMAccountId, szAccount);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PROMAccountId)
	return err;
}

// ----------------------------------------------------------------------------
// 新增推广员奖励
DBError TBLPromotion::AddBonus(int AccountId, int PlayerId, int PROMAccountId,
							   int& money, int& exp, int& bindGold, int& mayday)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_AddPromotionBonus %d,%d,%d,%d,%d,%d,%d",
					AccountId, PlayerId, PROMAccountId, money, exp, bindGold, mayday);
		money = exp = bindGold  = mayday = 0;
		if(GetConn()->More())
		{
			money = GetConn()->GetInt();
			exp = GetConn()->GetInt();
			bindGold = GetConn()->GetInt();
			mayday = GetConn()->GetInt();
			err = DBERR_NONE;
		}
	}
	DBCATCH(PlayerId)
	return err;
}

// ----------------------------------------------------------------------------
// 查询推广员奖励
DBError TBLPromotion::QueryBonus(int PROMAccountId, int& money, int& exp, 
								 int& bindGold, int& mayday)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_QueryPromotionBonus %d", PROMAccountId);
		if(GetConn()->More())
		{
			money = GetConn()->GetInt();
			exp = GetConn()->GetInt();
			bindGold = GetConn()->GetInt();
			mayday = GetConn()->GetInt();
			err = DBERR_NONE;
		}
	}
	DBCATCH(PROMAccountId)
	return err;
}

// ----------------------------------------------------------------------------
// 推广员领取奖励
DBError TBLPromotion::DrawBonus(int PROMAccountId, int PROMPlayerId, 
			int& Money, int& Exp, int& bindGold, int& HistoryId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_AddPromotionHistory %d,%d", PROMAccountId, PROMPlayerId);
		if(GetConn()->More())
		{
			Money = GetConn()->GetInt();
			Exp = GetConn()->GetInt();
			bindGold = GetConn()->GetInt();
			HistoryId = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBCATCH(PROMAccountId)
	return err;
}

// ----------------------------------------------------------------------------
// 推广员领取劳动节奖章
DBError TBLPromotion::DrawBonusForMayDay(int PROMAccountId, int PROMPlayerId, 
								int& MayDay, int& HistoryId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_AddPromotionHistoryForMayDay %d,%d", PROMAccountId, PROMPlayerId);
		if(GetConn()->More())
		{
			MayDay = GetConn()->GetInt();
			HistoryId = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBCATCH(PROMAccountId)
	return err;
}

// ----------------------------------------------------------------------------
// 取消推广员领取的奖励
DBError TBLPromotion::CancelDrawBonus(int PROMAccountId, int HistoryId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_DelPromotionHistory %d,%d", 
			PROMAccountId, HistoryId);
		if(GetConn()->More() && GetConn()->GetInt() == 0)
			err = DBERR_NONE;
	}
	DBCATCH(PROMAccountId)
	return err;
}

// ----------------------------------------------------------------------------
// 新增推广员物品奖励
// 参数2　为达到各等级的好友人数（80级，75级,70级，60级，50级，40级)
DBError TBLPromotion::AddBonusForItem(int PROMAccountId, stPROMItemBonus& bonus)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		int itemId, isDraw;
		M_SQL(GetConn(),"EXECUTE Sp_AddPromotionBonusForItemId %d", PROMAccountId);
		while(GetConn()->Fetch())
		{
			int index = GetConn()->GetRecordSetIndex();
			if(index == 1)		//行数，总页数，当前页数,总记录数
			{
				bonus.Levels[0] = GetConn()->GetInt();
				bonus.Levels[1] = GetConn()->GetInt();
				bonus.Levels[2] = GetConn()->GetInt();
				bonus.Levels[3] = GetConn()->GetInt();
				bonus.Levels[4] = GetConn()->GetInt();
				bonus.Levels[5] = GetConn()->GetInt();
			}
			else if(index == 2)
			{
				itemId = GetConn()->GetInt();
				isDraw = GetConn()->GetInt() + 1;//0-不可领取 1-可领取 2-已领取
				switch(itemId)
				{
				case PROM80_5 : bonus.DrawFlag[0] = isDraw;break;
				case PROM80_4 : bonus.DrawFlag[1] = isDraw;break;
				case PROM75_5 : bonus.DrawFlag[2] = isDraw;break;
				case PROM75_4 : bonus.DrawFlag[3] = isDraw;break;
				case PROM70_5 : bonus.DrawFlag[4] = isDraw;break;
				case PROM70_4 : bonus.DrawFlag[5] = isDraw;break;
				case PROM60_4 : bonus.DrawFlag[6] = isDraw;break;
				case PROM50_4 : bonus.DrawFlag[7] = isDraw;break;
				case PROM60_3 : bonus.DrawFlag[8] = isDraw;break;
				case PROM50_3 : bonus.DrawFlag[9] = isDraw;break;
				case PROM40_3 : bonus.DrawFlag[10] = isDraw;break;
				}
			}
		}
		err = DBERR_NONE;
	}
	DBCATCH(PROMAccountId)
	return err;
}

// ----------------------------------------------------------------------------
// 推广员领取劳动节奖章
DBError TBLPromotion::DrawBonusForItem(int PROMAccountId, int PROMPlayerId, 
									   int ItemId, int& ret)
{
	DBError err = DBERR_UNKNOWERR;
	ret = 0;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_AddPromotionHistoryForItemId %d,%d,%d", 
			PROMAccountId, PROMPlayerId, ItemId);
		if(GetConn()->More())
		{
			ret = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBCATCH(PROMAccountId)
	return err;
}

// ----------------------------------------------------------------------------
// 新增推广员关系
DBError TBLPromotion::AddPROMAccount(int AccountId, int PROMAccountId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_AddPROMAccount %d,%d", AccountId, PROMAccountId);
		GetConn()->Exec();
	}
	DBCATCH(AccountId)
	return err;
}

// ----------------------------------------------------------------------------
// 批量新增推广员关系
DBError TBLPromotion::BatchAddPROMAccount(int PROMAccountId, std::vector<int>& AccountList)
{	
	if(AccountList.empty())
		return DBERR_NONE;

	DBError err = DBERR_UNKNOWERR;
	char szStr[20];
	char szAccount[8000] = {0,};
	for(size_t i = 0; i < AccountList.size(); i++)
	{
		if( i > 0)
		{
			strcat_s(szAccount, 8000, ",");
		}

		_itoa_s( AccountList[i], szStr, 20, 10 );
		strcat_s(szAccount, 8000, szStr);

		if(i > MAX_QUERY_PRESENTEE)
		{
			g_Log.WriteError("此推广员(AccountId=%d)下线人数超过600人,", PROMAccountId);
			break;
		}
	}

	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_BatchAddPROMAccount %d,'%s'",
			PROMAccountId, szAccount);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PROMAccountId)
	return err;
}