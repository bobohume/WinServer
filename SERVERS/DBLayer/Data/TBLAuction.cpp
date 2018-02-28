#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "Common/PacketType.h"
#include "Common/AuctionBase.h"
#include "DBUtility.h"
#include "TBLAuction.h"

// ----------------------------------------------------------------------------
// 检查更新拍卖行商品
DBError TBLAuction::CheckUpdate()
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_Auction_CheckUpdate");
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 为物品数据生成到三个binary字段数据
bool GetItemBinary(stItemInfo* pItem, char* ItemData)
{
	ItemData[0] = '\0';
	if(pItem == NULL)
		return true;

	int srcLen = sizeof(stItemInfo);
	if(srcLen > 255)
		return false;

	ConvertHex(ItemData, 255*2+1, (unsigned char*)pItem, srcLen);
	return true;
}

// ----------------------------------------------------------------------------
//	载入拍卖行所有拍卖物数据
//　lastQueryTime 上一次查询时间
//  serverList 待查询过滤服务器列表
//  auctionList	返回的拍卖行列表
DBError TBLAuction::Query(unsigned int& lastQueryTime, std::vector<stAuctionInfo*>& auctionList)
{
	DBError err = DBERR_UNKNOWERR;
	S32 iLen;
	stAuctionInfo* pAuction;
	char *pBlock;
	char szLastQueryTime[32] = {0};
	int index;

	GetDBTimeString(lastQueryTime, szLastQueryTime);

	try
	{
		/*Id,[Owner],[Status],[FixedPrice],[ExpiryTime],[ItemId],[ItemNum],[ItemData]*/
		M_SQL(GetConn(),"EXECUTE Sp_Auction_Query '%s'", szLastQueryTime);
		while(GetConn()->Fetch())
		{
			index = GetConn()->GetRecordSetIndex();
			if(index == 1)
			{
				lastQueryTime = GetConn()->GetTime();
			}
			else
			{
				pAuction = new stAuctionInfo;
				pAuction->Id			= GetConn()->GetInt();
				pAuction->Owner			= GetConn()->GetInt();
				pAuction->Status		= GetConn()->GetInt();
				pAuction->FixedPrice	= GetConn()->GetInt();
				pAuction->ExpiryTime	= GetConn()->GetTime();
				pAuction->ItemId		= GetConn()->GetInt();
				pAuction->ItemNum		= GetConn()->GetInt();

				auctionList.push_back(pAuction);

				pBlock = (char*)GetConn()->GetBinary("ItemData", iLen);
				if(NULL == pBlock || iLen < 100)//小于100肯定非Item数据
					continue;

				pAuction->pItem = new stItemInfo;
				memcpy(pAuction->pItem, pBlock, iLen);
			}			
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 新增拍卖行商品数据
DBError TBLAuction::AddNew(stAuctionInfo* info)
{
	DBError err = DBERR_UNKNOWERR;
	if(NULL == info)
		return err;

	char szExpiryTime[32] = {0};
	GetDBTimeString(info->ExpiryTime, szExpiryTime);
	char szOpTime[32] = {0};
	GetDBTimeString(info->Id, szOpTime);

	char ItemData[255*2+1];
	if(!GetItemBinary(info->pItem, ItemData))
	{
		g_Log.WriteFocus("新增拍卖行商品数据时创建物品数据失败!(playerid=%d,itemid=%d",
				info->Owner, info->ItemId);
		return err;
	}

	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_Auction_Add %d,%d,'%s','%s',%d,%d,0x%s",
					info->Owner, info->FixedPrice, szExpiryTime, szOpTime, 
					info->ItemId, info->ItemNum, ItemData);
		GetConn()->Exec();
		info->Id = 0;
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 购买拍卖行商品
DBError TBLAuction::BuyOne(U32 id, U32 buyerId, S32 buyPrice, U32 buyTime, S32& error)
{
	DBError err = DBERR_UNKNOWERR;
	error = -1;
	char szBuyTime[32] = {0};
	GetDBTimeString(buyTime, szBuyTime);
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_Auction_Buy %d,%d,%d,'%s'", 
			id, buyerId, buyPrice,szBuyTime);
		if(GetConn()->More())
		{
			error = GetConn()->GetInt();
			switch(error)
			{
			case -1: throw ExceptionResult(DBERR_AUCTION_NOFOUND); break;
			case -3: throw ExceptionResult(DBERR_AUCTION_DONTBUYSELF); break;
			case -4: throw ExceptionResult(DBERR_AUCTION_FIXEDPRICEERR); break;
			default: err = DBERR_NONE; break;
			}
		}
	}
	DBCATCH(id)
	return err;
}

// ----------------------------------------------------------------------------
// 取消拍卖行商品
DBError TBLAuction::Cancel(U32 id, U32 owner, U32 cancelTime, S32& error)
{
	DBError err = DBERR_UNKNOWERR;
	char szCancelTime[32] = {0};
	GetDBTimeString(cancelTime, szCancelTime);
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_Auction_Cancel %d,%d,'%s'", id, owner, szCancelTime);
		if(GetConn()->More())
		{
			error = GetConn()->GetInt();
			err = DBERR_NONE;
		}
	}
	DBCATCH(id)
	return err;
}