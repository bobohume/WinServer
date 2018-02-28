#ifndef __TBLGOLDTRADE_H__
#define __TBLGOLDTRADE_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif
#pragma once
#include "common/StockBase.h"
#include <vector>

class TBLGoldTrade: public TBLBase
{
public:
	TBLGoldTrade(DataBase* db) : TBLBase( db ) {};
	virtual ~TBLGoldTrade() {};
	DBError AddOrder(stGoldTrade* info);
	DBError CommissionOrder(stGoldTrade* pTrade, std::vector<stGoldTradeEx*>& source);
	DBError DeleteOrder(stGoldTrade* pOrder);
	DBError BreakOrder(std::vector<int>& buylist, std::vector<int>& selllist);
	DBError CloseOrder(int CloseTime,int& ClosePrice, U32& NextCloseTime,
			std::vector<stCloseGoldTrade*>& taxList, std::vector<stCloseGoldTrade*>& goldList);
	DBError LoadOrder(MAPASC& sell, MAPDESC& buy);
	DBError QueryOrder(int PlayerID, std::vector<stGoldTrade*>& order,
		std::vector<stGoldTradeDetail*>& detail);
};

#endif//__TBLGOLDTRADE_H__