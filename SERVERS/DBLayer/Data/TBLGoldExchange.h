#ifndef __TBLGOLDEXCHANGE_H__
#define __TBLGOLDEXCHANGE_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif
#pragma once
#include <vector>
#include "common/GoldExchangeBase.h"

class TBLGoldExchange: public TBLBase
{
public:
	TBLGoldExchange(DataBase* db) : TBLBase( db ) {};
	virtual ~TBLGoldExchange() {};
	DBError Query(stGoldExchange* exchange);
	DBError QueryMonthGolds(U32 PlayerID, int& MonthGolds);
	DBError SavePays(U32 PlayerID, S32 MonthPays);
	DBError QueryConfig(stGoldConfig* config);
	DBError Draw(int AccountID, int PlayerID, const char* PlayerName, int type,
		int& Exchanges, int& Gold, int& TotalDrawGolds);
};

#endif//__TBLGOLDEXCHANGE_H__