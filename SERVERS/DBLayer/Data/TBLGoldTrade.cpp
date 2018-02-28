#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "DBUtility.h"
#include "TBLGoldTrade.h"

// ----------------------------------------------------------------------------
// 取消满18小时的挂单
DBError TBLGoldTrade::BreakOrder(std::vector<int>& buylist, std::vector<int>& selllist)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(), "EXECUTE Sp_GoldTrade_Break");
		while(GetConn()->More())
		{
			if(GetConn()->GetInt() == 0)
				buylist.push_back(GetConn()->GetInt());
			else
				selllist.push_back(GetConn()->GetInt());
		}
		err = DBERR_NONE;
	}
	DBECATCH();
	return err;
}

// ----------------------------------------------------------------------------
// 新增元宝交易挂单数据
DBError TBLGoldTrade::AddOrder( stGoldTrade* pOrder)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		char szUpTime[32] = {0};
		GetDBTimeString(pOrder->UpTime, szUpTime);
		M_SQL(GetConn(), "EXECUTE Sp_AddGoldTradeOrder %d,%d,'%s',%d,%d,%d,%d,'%s'",
			pOrder->AccountID, pOrder->PlayerID, pOrder->PlayerName, pOrder->OrderType,
			pOrder->Price, pOrder->TradeNum, pOrder->LeftNum, szUpTime);
		if(GetConn()->More() && GetConn()->GetInt() == 0)
		{
			pOrder->OrderID = GetConn()->GetInt();
			err = DBERR_NONE;
		}
		else
			throw ExceptionResult(DBERR_GOLDTRADE_REDUCEGOLD);
	}
	DBCATCH( pOrder->PlayerID );
	return err;
}

// ----------------------------------------------------------------------------
// 更新元宝交易挂单数据
DBError TBLGoldTrade::CommissionOrder(stGoldTrade* pTrade, std::vector<stGoldTradeEx*>& source)
{
	char szRecord[8000] = {0};
	char szRow[128] = {0};
	bool isFirst = true;

	for(size_t i = 0; i < source.size(); i++)
	{
		stGoldTradeEx* pSource = source[i];
		if(!pSource)
			continue;
		if(isFirst)
		{
			sprintf_s(szRow, 128, "EXECUTE Sp_DoGoldTradeOrder %d,%d,'%d,%d",
				pTrade->OrderID, pTrade->LeftNum, pSource->OrderID, pSource->BargainNum);
		}
		else
		{
			sprintf_s(szRow, 128, ";%d,%d",
				pSource->OrderID, pSource->BargainNum);
		}
		strcat_s(szRecord, 8000, szRow);
		isFirst = false;
	}

	if(!isFirst)
		strcat_s(szRecord, 8000, "'");

	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(szRecord[0])
		{
			M_SQL(GetConn(), szRecord);
			if(!GetConn()->More() || GetConn()->GetInt() != 0)
				throw ExceptionResult(DBERR_UNKNOWERR);
			err = DBERR_NONE;
		}		
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 载入所有挂单数据
DBError TBLGoldTrade::LoadOrder(MAPASC& sell, MAPDESC& buy)
{
	DBError err = DBERR_UNKNOWERR;
	stGoldTrade* pOrder = NULL;
	try
	{		
		M_SQL(GetConn(), "SELECT OrderID,AccountID,PlayerID,PlayerName,OrderType,Price,TradeNum,LeftNum "\
			"FROM Tbl_GoldTrade_Order WHERE LeftNum<>0 AND OrderStatus=0");
		while(GetConn()->More())
		{
			pOrder = new stGoldTrade;
			pOrder->OrderID		= GetConn()->GetInt();
			pOrder->AccountID	= GetConn()->GetInt();
			pOrder->PlayerID	= GetConn()->GetInt();
			strcpy_s(pOrder->PlayerName, 32, GetConn()->GetString());
			pOrder->OrderType	= GetConn()->GetInt();
			pOrder->Price		= GetConn()->GetInt();
			pOrder->TradeNum	= GetConn()->GetInt();
			pOrder->LeftNum		= GetConn()->GetInt();

			if(pOrder->OrderType == OT_BUY)
				buy.insert(MAPDESC::value_type(pOrder->Price, pOrder));
			else
				sell.insert(MAPASC::value_type(pOrder->Price, pOrder));
		}
		err = DBERR_NONE;
	}
	DBECATCH();
	return err;
}

// ----------------------------------------------------------------------------
// 删除挂单数据
DBError TBLGoldTrade::DeleteOrder(stGoldTrade* pOrder)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(), "EXECUTE Sp_DeleteGoldTradeOrder %d", pOrder->OrderID);
		if(GetConn()->More())
		{
			int ret = GetConn()->GetInt();
			if(ret == -1)
				throw ExceptionResult(DBERR_GOLDTRADE_RETURNGOLD);
			else if(ret == -2)
				throw ExceptionResult(DBERR_GOLDTRADE_NOFOUND);
			else
				err = DBERR_NONE;
		}
		else
			throw ExceptionResult(DBERR_UNKNOWERR);
	}
	DBECATCH();
	return err;
}

// ----------------------------------------------------------------------------
// 挂单数据清盘
DBError TBLGoldTrade::CloseOrder(int CloseTime,int& ClosePrice, U32& NextCloseTime,
			std::vector<stCloseGoldTrade*>& taxList,
			std::vector<stCloseGoldTrade*>& goldList)
{
	ClosePrice = 0;
	NextCloseTime = 0;
	stCloseGoldTrade* pOrder = NULL;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		char szCloseTime[32];
		GetDBTimeString(CloseTime, szCloseTime);
		M_SQL(GetConn(), "EXECUTE Sp_CloseGoldTradeOrder '%s'", szCloseTime);
		while(GetConn()->Fetch())
		{
			int index = GetConn()->GetRecordSetIndex();
			if(index == 1)
			{
				ClosePrice = GetConn()->GetInt();
				NextCloseTime = GetConn()->GetTime();
				err = DBERR_NONE;
			}
			else if(index == 2)
			{
				pOrder = new stCloseGoldTrade;
				pOrder->OrderID = GetConn()->GetInt();
				pOrder->PlayerID = GetConn()->GetInt();
				pOrder->LeftValue = GetConn()->GetInt();
				taxList.push_back(pOrder);
				err = DBERR_NONE;
			}
			else if (index ==3)
			{
				pOrder = new stCloseGoldTrade;
				pOrder->OrderID = GetConn()->GetInt();
				pOrder->PlayerID = GetConn()->GetInt();
				pOrder->LeftValue = GetConn()->GetInt();
				goldList.push_back(pOrder);
				err = DBERR_NONE;
			}
		}
	}
	DBECATCH();
	return err;
}

// ----------------------------------------------------------------------------
// 查询当前玩家所有挂单数据及历史记录
DBError TBLGoldTrade::QueryOrder(int PlayerID, std::vector<stGoldTrade*>& order,
					std::vector<stGoldTradeDetail*>& detail)
{
	DBError err = DBERR_UNKNOWERR;
	stGoldTrade* pOrder = NULL;
	stGoldTradeDetail* pOrderDetail = NULL;
	try
	{
		int index = 0, rows = 0;
		M_SQL(GetConn(), "EXECUTE Sp_QueryOrder %d", PlayerID);
		while(GetConn()->Fetch())
		{
			index = GetConn()->GetRecordSetIndex();
			if(index == 1)
			{
				pOrder = new stGoldTrade;
				pOrder->OrderID		= GetConn()->GetInt();
				pOrder->AccountID	= GetConn()->GetInt();
				pOrder->PlayerID	= GetConn()->GetInt();
				strcpy_s(pOrder->PlayerName, 32, GetConn()->GetString());
				pOrder->OrderType	= GetConn()->GetInt();
				pOrder->Price		= GetConn()->GetInt();
				pOrder->TradeNum	= GetConn()->GetInt();
				pOrder->LeftNum		= GetConn()->GetInt();
				pOrder->OrderStatus	= GetConn()->GetInt();
				pOrder->UpTime		= GetConn()->GetTime();
				order.push_back(pOrder);
			}
			else
			{
				if(rows < 2000)	//为网络消息包嵌制一下行集数据最大上限
				{
					pOrderDetail = new stGoldTradeDetail;
					pOrderDetail->OrderID	= GetConn()->GetInt();
					pOrderDetail->TradeNum	= GetConn()->GetInt();
					pOrderDetail->TradeDate	= GetConn()->GetTime();
					detail.push_back(pOrderDetail);
				}				
				rows++;
			}
		}
		err = DBERR_NONE;
	}
	DBECATCH();
	return err;
}