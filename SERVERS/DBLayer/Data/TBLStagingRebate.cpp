#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "Common/PacketType.h"
#include "DBUtility.h"
#include "TBLStagingRebate.h"

// ----------------------------------------------------------------------------
// 查询分期返利活动数据
DBError TBLStagingRebate::QueryActivity(U32 PlayerId,
										std::vector<stSR_Period*>& ActivityList, 
										std::vector<stSR_Bonus*>& ConfigList,
										std::vector<stSR_History*>& HistoryList)
{
	DBError err = DBERR_UNKNOWERR;
	stSR_Period* pActivity;
	stSR_Bonus* pBonus;
	stSR_History* pHistory;
	U32 SRId;
	int index,PerMonth;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_QueryStagingRebatePeriod %d", PlayerId);
		while(GetConn()->Fetch())
		{
			index = GetConn()->GetRecordSetIndex();
			if(index == 1)
			{
				pActivity = new stSR_Period;
				pActivity->SRId = GetConn()->GetInt();
				pActivity->StartDate = GetConn()->GetTime();
				pActivity->EndDate = GetConn()->GetTime();
				pActivity->DrawDate1st = GetConn()->GetTime();
				pActivity->DrawDate2nd = GetConn()->GetTime();
				pActivity->DrawDate3rd = GetConn()->GetTime();
				pActivity->IsDefault = GetConn()->GetInt();
				pActivity->IsRewardGold = GetConn()->GetInt();
				pActivity->Status = GetConn()->GetInt();
				ActivityList.push_back(pActivity);
			}
			else if(index == 2)
			{
				pBonus = new stSR_Bonus;
				pBonus->SRId = GetConn()->GetInt();;
				pBonus->PerStag = GetConn()->GetInt();
				pBonus->PerMonth = GetConn()->GetInt();
				pBonus->Gold = GetConn()->GetInt();
				pBonus->BindGold = GetConn()->GetInt();
				pBonus->Money = GetConn()->GetInt();
				pBonus->BindMoney = GetConn()->GetInt();
				pBonus->Exp = GetConn()->GetInt();
				ConfigList.push_back(pBonus);
			}
			else if(index == 3)
			{
				pHistory = new stSR_History;
				pHistory->SRId = GetConn()->GetInt();
				pHistory->TotalDrawGolds = GetConn()->GetInt();
				HistoryList.push_back(pHistory);
			}
			else if(index == 4)
			{
				pHistory = NULL;
				SRId = GetConn()->GetInt();			

				for(size_t i = 0; i < HistoryList.size(); i++)
				{
					if(SRId == HistoryList[i]->SRId)
					{
						pHistory = HistoryList[i];
						break;
					}
				}

				if(pHistory)
				{
					PerMonth = GetConn()->GetInt();
					if(PerMonth <=0 || PerMonth > 3)
					{
						g_Log.WriteError("分期返利配置数据有错误!(SRId=%d, PlayerId=%d)", SRId, PlayerId);
						continue;
					}

					pHistory->Data[PerMonth-1].IsDraw = GetConn()->GetInt();
					pHistory->Data[PerMonth-1].Gold = GetConn()->GetInt();
					pHistory->Data[PerMonth-1].BindGold = GetConn()->GetInt();
					pHistory->Data[PerMonth-1].Money = GetConn()->GetInt();
					pHistory->Data[PerMonth-1].BindMoney = GetConn()->GetInt();
					pHistory->Data[PerMonth-1].Exp = GetConn()->GetInt();
				}
			}
		}
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
	return err;
}

// ----------------------------------------------------------------------------
// 查询分期返利的奖励配置数据
DBError TBLStagingRebate::QueryConfig(U32 SRId, std::vector<stSR_Bonus*>& ConfigList)
{
	DBError err = DBERR_UNKNOWERR;
	stSR_Bonus* pBonus;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_QueryStagingRebateBonus %d", SRId);
		while(GetConn()->More())
		{
			pBonus = new stSR_Bonus;
			pBonus->SRId = SRId;
			pBonus->PerStag = GetConn()->GetInt();
			pBonus->PerMonth = GetConn()->GetInt();
			pBonus->Gold = GetConn()->GetInt();
			pBonus->BindGold = GetConn()->GetInt();
			pBonus->Money = GetConn()->GetInt();
			pBonus->BindMoney = GetConn()->GetInt();
			pBonus->Exp = GetConn()->GetInt();

			ConfigList.push_back(pBonus);
		}
		err = DBERR_NONE;
	}
	DBCATCH(SRId)
	return err;
}

// ----------------------------------------------------------------------------
// 查询角色的分期返利奖励
DBError TBLStagingRebate::QueryBonus(U32 PlayerId, stSR_History& BonusData)
{
	DBError err = DBERR_UNKNOWERR;
	int index, PerMonth;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_QueryStagingRebateHistory %d,%d", BonusData.SRId, PlayerId);
		while(GetConn()->Fetch())
		{
			index = GetConn()->GetRecordSetIndex();
			if(index == 1)		//活动期间累计提取元宝数
			{
				BonusData.TotalDrawGolds = GetConn()->GetInt();
			}
			else if(index == 2) //分期返利的奖励数据
			{
				PerMonth = GetConn()->GetInt();
				if(PerMonth <=0 || PerMonth > 3)
				{
					g_Log.WriteError("分期返利配置数据有错误!(SRId=%d, PlayerId=%d)", BonusData.SRId, PlayerId);
					continue;
				}

				BonusData.Data[PerMonth-1].IsDraw = GetConn()->GetInt();
				BonusData.Data[PerMonth-1].Gold = GetConn()->GetInt();
				BonusData.Data[PerMonth-1].BindGold = GetConn()->GetInt();
				BonusData.Data[PerMonth-1].Money = GetConn()->GetInt();
				BonusData.Data[PerMonth-1].BindMoney = GetConn()->GetInt();
				BonusData.Data[PerMonth-1].Exp = GetConn()->GetInt();		
			}
		}
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
	return err;
}

// ----------------------------------------------------------------------------
// 领取分期返利的奖励
DBError TBLStagingRebate::DrawBonus(U32 AccountId, U32 PlayerId, U32 SRId, U8 PerMonth,
									stSR_Bonus_Data& BonusData, int& Err)
{
	DBError err = DBERR_UNKNOWERR;
	Err = -2;//异常失败（非正常数据处理失败）
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_DrawStagingRebateHistory %d,%d,%d,%d",
				AccountId, PlayerId, SRId, PerMonth);
		if(GetConn()->More())
		{
			Err = GetConn()->GetInt();
			BonusData.Gold = GetConn()->GetInt();
			BonusData.BindGold = GetConn()->GetInt();
			BonusData.Money = GetConn()->GetInt();
			BonusData.BindMoney = GetConn()->GetInt();
			BonusData.Exp = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
	return err;
}

// ----------------------------------------------------------------------------
// 提取元宝后更新玩家的分期返利奖励数据
DBError TBLStagingRebate::UpdateBonus(U32 PlayerId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_UpdateStagingRebateHistory %d", PlayerId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
	return err;
}