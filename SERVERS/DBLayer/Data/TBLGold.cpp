#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include <time.h>
#include "TBLGold.h"
#include "Common/BillingBase.h"

TBLGold::TBLGold(DataBase* db):TBLBase(db)
{
}

TBLGold::~TBLGold()
{
}

// ----------------------------------------------------------------------------
// 查询元宝操作
DBError TBLGold::QueryGold(int AccountID, int PlayerID, int& Gold, int& TotalDrawGolds)
{
	DBError err = DBERR_UNKNOWERR;
	Gold = 0;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_QueryGolds %d, %d", AccountID, PlayerID);
		if(GetConn()->More())
		{
			Gold = GetConn()->GetInt();
			TotalDrawGolds = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBCATCH(AccountID)
	return err;
}

// ----------------------------------------------------------------------------
// 更新元宝操作
DBError TBLGold::UpdateGold(int AccountID, int Gold, int GoldOpType, int PlayerID,
							int& LeftGold, int& TotalDrawGold)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_UpdateGold %d,%d,%d,%d", AccountID, Gold, GoldOpType, PlayerID);
		if(GetConn()->More() && GetConn()->GetInt() == 0)
		{
			LeftGold = GetConn()->GetInt();
			TotalDrawGold = GetConn()->GetInt();
			err = DBERR_NONE;
		}
		else
			throw ExceptionResult(DBERR_ACCOUNT_UPDATEGOLD);
	}
	DBCATCH(AccountID)

	//若提取元宝成功，则判定玩家是否有分期返利活动可参加并重新计算奖励
	if(err == DBERR_NONE && GoldOpType == 0)
	{
		try
		{
			M_SQL(GetConn(),"EXECUTE Sp_UpdateStagingRebateHistory %d", PlayerID);
			GetConn()->Exec();
		}
		DBCATCH(PlayerID)
	}
	return err;
}

// ----------------------------------------------------------------------------
// 查询累计提取元宝操作
DBError TBLGold::QueryTotalDrawGolds(int PlayerID, int& Golds)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_QueryTotalDrawGolds %d", PlayerID);
		if(GetConn()->More())
		{
			Golds = GetConn()->GetInt();
			err = DBERR_NONE;
		}
		else
			throw ExceptionResult(DBERR_GOLDEXCHAGE_NOFOUND);
	}
	DBECATCH()
	return err;
}