#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "DBUtility.h"
#include "common/BillingBase.h"
#include "TBLTradeGoods.h"

DBError TBLTradeGoods::Save( stTradeGoods& info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		char szTradeDate[32] = {0};
		GetDBTimeString(info.TradeDate, szTradeDate);
		M_SQL(GetConn(), "EXECUTE Sp_SaveTradeGoods %d,%d,%I64d,%d,%d,%d,%d,'%s'",
			info.AccountID, info.PlayerID, info.PriceID, info.PriceNum,
			info.PriceType, info.Price, info.AreaID, szTradeDate);
		if(GetConn()->More())
		{
			info.TradeID = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBCATCH( info.PlayerID);
	return err;
}