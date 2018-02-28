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

class TBLGoldHistory: public TBLBase
{
public:
	TBLGoldHistory(DataBase* db) : TBLBase( db ) {};
	virtual ~TBLGoldHistory() {};
	DBError QueryGoldCostMonth(stdext::hash_map<int,int>& TopGold);
	DBError QueryGoldCostTotal(stdext::hash_map<int,int>& TopGold);

};

#endif