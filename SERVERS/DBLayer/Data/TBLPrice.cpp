#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include <time.h>
#include "DBUtility.h"
#include "TBLPrice.h"
#include "Common/BillingBase.h"

TBLPrice::TBLPrice(DataBase* db):TBLBase(db)
{
}

TBLPrice::~TBLPrice()
{
}

// ----------------------------------------------------------------------------
// 检查上架商品更新
bool TBLPrice::CheckUpdate(int oldver, int& newver)
{
	DBError err = DBERR_UNKNOWERR;
	newver = -1;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_CheckPriceUpdate");
		if(GetConn()->More())
		{
			newver = GetConn()->GetInt();
			err = DBERR_NONE;
		}
	}
	DBECATCH();
	return oldver != newver;
}

// ----------------------------------------------------------------------------
// 查询所有上架商品数据
DBError TBLPrice::Load(std::vector<stGoodsPrice*>& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		typedef stdext::hash_map<int, stGoods*> GOODSMAP;
		GOODSMAP goodslist;
		stGoods* goods;
		stGoodsPrice* goodsPrice;
		int index = 0;

		M_SQL(GetConn(),"EXECUTE Sp_QueryGoodsPrice");
		while(GetConn()->Fetch())
		{
			index = GetConn()->GetRecordSetIndex();
			if(index == 1)
			{
				int GoodsID = GetConn()->GetInt();
				int ItemID = GetConn()->GetInt();
				int ItemNum = GetConn()->GetInt();
				GOODSMAP::iterator it = goodslist.find(GoodsID);
				if(it == goodslist.end())
				{
					goods = new stGoods;
					goodslist.insert(GOODSMAP::value_type(GoodsID, goods));
				}
				else
					goods = it->second;

				if(goods->ItemCount >= stGoods::MAX_ITEMS)
					continue;
				goods->GoodsID = GoodsID;
				goods->ItemID[goods->ItemCount] = ItemID;
				goods->ItemNum[goods->ItemCount] = ItemNum;
				goods->ItemCount++;
			}
			else
			{
				goodsPrice = new stGoodsPrice;
				goodsPrice->PriceID = GetConn()->GetBigInt();
				goodsPrice->GoodsID = GetConn()->GetInt();
				strcpy_s(goodsPrice->GoodsName, 32, GetConn()->GetString());
				goodsPrice->ShowItemID = GetConn()->GetInt();
				goodsPrice->DiscountID = GetConn()->GetInt();
				goodsPrice->StoreType = GetConn()->GetInt();
				goodsPrice->GoodsMainType = GetConn()->GetInt();
				goodsPrice->GoodsSubType = GetConn()->GetInt();
				goodsPrice->PriceType = GetConn()->GetInt();
				goodsPrice->Price = GetConn()->GetInt();
				goodsPrice->DiscountRate = GetConn()->GetInt();
				goodsPrice->OnSale	= GetConn()->GetTime();
				goodsPrice->OffSale = GetConn()->GetTime();
				goodsPrice->StartTime = GetConn()->GetTime();
				goodsPrice->EndTime = GetConn()->GetTime();
				Info.push_back(goodsPrice);
			}
		}

		for each(stGoodsPrice* it in Info)
		{
			GOODSMAP::iterator itFind = goodslist.find(it->GoodsID);
			if(itFind != goodslist.end())
			{
				goods = itFind->second;
				it->ItemCount = goods->ItemCount;
				for(int i = 0; i < goods->ItemCount; i++)
				{
					it->ItemID[i] = goods->ItemID[i];
					it->ItemNum[i] = goods->ItemNum[i];
				}
			}
		}		
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 查询商城广播消息内容
DBError TBLBoardMsg::Load(BOARDVEC& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_Mall_QueryBoard");
		stMallBoard* pMsg = NULL;
		while(GetConn()->More())
		{
			pMsg = new stMallBoard;
			pMsg->MsgId = GetConn()->GetInt();
			strncpy_s(pMsg->Content, 250, GetConn()->GetString(), _TRUNCATE);
			pMsg->UpdValue = GetConn()->GetInt();
			Info.push_back(pMsg);
		}		
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}