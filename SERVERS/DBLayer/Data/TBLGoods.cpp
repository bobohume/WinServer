#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include <time.h>
#include "TBLGoods.h"
#include "Common/BillingBase.h"

TBLGoods::TBLGoods(DataBase* db):TBLBase(db)
{
}

TBLGoods::~TBLGoods()
{
}

// ----------------------------------------------------------------------------
// 查询商城商品数据
DBError TBLGoods::LoadAll(std::vector<stGoods*>& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{	
		typedef stdext::hash_map<int, stGoods*> GOODSMAP;
		GOODSMAP goodslist;
		stGoods* goods;

		M_SQL(GetConn(),"SELECT * FROM Tbl_Goods");
		while(GetConn()->More())
		{
			goods = new stGoods;
			goods->GoodsID = GetConn()->GetInt();
			goods->StoreType = GetConn()->GetInt();
			goods->GoodsMainType = GetConn()->GetInt();
			goods->GoodsSubType = GetConn()->GetInt();			
			goods->Price = GetConn()->GetInt();
			goods->OnSale = GetConn()->GetTime();
			goods->OffSale = GetConn()->GetTime();
			goodslist.insert(GOODSMAP::value_type(goods->GoodsID, goods));
		}

		M_SQL(GetConn(),"SELECT * FROM Tbl_Goods_Item");
		while(GetConn()->More())
		{
			int GoodsID = GetConn()->GetInt();
			int ItemID = GetConn()->GetInt();
			int ItemNum = GetConn()->GetInt();
			GOODSMAP::iterator it = goodslist.find(GoodsID);
			if(it != goodslist.end())
			{
				goods = it->second;
				if(goods->ItemCount >= stGoods::MAX_ITEMS)
					continue;
				goods->ItemID[goods->ItemCount] = ItemID;
				goods->ItemNum[goods->ItemCount] = ItemNum;
				goods->ItemCount++;
			}
		}

		for each(const std::pair<int, stGoods*>& it in goodslist)
		{
			Info.push_back(it.second);
		}		
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 查询商城商品数据（因每周必维护，查询数据量可优化，仅一周数据即可)
DBError TBLGoods::Load(std::vector<stGoods*>& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		struct tm today, after7day;
		time_t curtime, endtime;
		time(&curtime);
		localtime_s(&today, &curtime);
		endtime = curtime + (24 * 3600 * 7);
		localtime_s(&after7day, &endtime);

		typedef stdext::hash_map<int, stGoods*> GOODSMAP;
		GOODSMAP goodslist;
		stGoods* goods;
		
		//超过当前时间的下架和当前时间一周后的上架都被忽略
		M_SQL(GetConn(),"SELECT * FROM Tbl_Goods WHERE OffSale>='%d-%02d-%02d %02d:%02d' AND OnSale<='%d-%02d-%02d %02d:%02d'",
						today.tm_year + 1900, today.tm_mon + 1, today.tm_mday, today.tm_hour, today.tm_min,
						after7day.tm_year + 1900, after7day.tm_mon + 1, after7day.tm_mday, after7day.tm_hour, after7day.tm_min);
		while(GetConn()->More())
		{
			goods = new stGoods;
			goods->GoodsID = GetConn()->GetInt();
			goods->StoreType = GetConn()->GetInt();
			goods->GoodsMainType = GetConn()->GetInt();
			goods->GoodsSubType = GetConn()->GetInt();			
			goods->Price = GetConn()->GetInt();
			goods->OnSale = GetConn()->GetTime();
			goods->OffSale = GetConn()->GetTime();
			goodslist.insert(GOODSMAP::value_type(goods->GoodsID, goods));
		}

		M_SQL(GetConn(),"SELECT GoodsID,ItemID,ItemNum FROM Tbl_Goods_Item");
		while(GetConn()->More())
		{
			int GoodsID = GetConn()->GetInt();
			int ItemID = GetConn()->GetInt();
			int ItemNum = GetConn()->GetInt();
			GOODSMAP::iterator it = goodslist.find(GoodsID);
			if(it != goodslist.end())
			{
				goods = it->second;
				if(goods->ItemCount >= stGoods::MAX_ITEMS)
					continue;
				goods->ItemID[goods->ItemCount] = ItemID;
				goods->ItemNum[goods->ItemCount] = ItemNum;
				goods->ItemCount++;
			}
		}

		for each(const std::pair<int, stGoods*>& it in goodslist)
		{
			if(it.second && it.second->ItemCount > 0)
				Info.push_back(it.second);
		}	

		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 存储商城商品数据
//DBError TBLGoods::Save(std::vector<stGoods*>& info)
//{
	//DBError err = DBERR_UNKNOWERR;
	//try
	//{		
	//	M_SQL(GetConn(),"TRUNCATE TABLE Tbl_Goods");
	//	GetConn()->Exec();

	//	char szRecord[8000] = {0};
	//	char szRow[256] = {0};
	//	bool isFirst = true;
	//	for(size_t i = 0; i < info.size(); i++)
	//	{
	//		if(i % 50 == 0)
	//		{
	//			if(!isFirst)
	//			{
	//				M_SQL(GetConn(),szRecord);
	//				GetConn()->Exec();
	//			}
	//			strcpy_s(szRecord, 8000, "INSERT Tbl_Goods (GoodsID,Store,Main,Sub,"\
	//				"Item1,Item2,Item3,Item4,Item5,Price,OnSale,OffSale) SELECT ");
	//			isFirst = true;
	//		}

	//		if(!isFirst)
	//			strcat_s(szRecord, 8000, " UNION ALL SELECT ");

	//		sprintf_s(szRow, 256, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
	//				info[i]->GoodsID,
	//				info[i]->StoreType,
	//				info[i]->GoodsMainType,
	//				info[i]->GoodsSubType,
	//				info[i]->ItemID[0],
	//				info[i]->ItemID[1],
	//				info[i]->ItemID[2],
	//				info[i]->ItemID[3],
	//				info[i]->ItemID[4],
	//				info[i]->Price,
	//				info[i]->OnSale,
	//				info[i]->OffSale);
	//		strcat_s(szRecord, 8000, szRow);
	//		isFirst = false;
	//	}

	//	if(!isFirst)
	//	{
	//		M_SQL(GetConn(),szRecord);
	//		GetConn()->Exec();
	//	}
	//}
	//DBECATCH()
	//return err;
//}