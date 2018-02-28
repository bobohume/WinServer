#ifndef __TBLSTAGINGREBATE_H__
#define __TBLSTAGINGREBATE_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#pragma once
#include "Common/GoldExchangeBase.h"

class TBLStagingRebate: public TBLBase
{
public:
	TBLStagingRebate(DataBase* db):TBLBase(db) {}
	virtual ~TBLStagingRebate() {}
	DBError QueryActivity(U32 PlayerId,
			std::vector<stSR_Period*>& ActivityList, 
			std::vector<stSR_Bonus*>& ConfigList,
			std::vector<stSR_History*>& HistoryList);
	DBError QueryConfig(U32 SRId, std::vector<stSR_Bonus*>& ConfigList);
	DBError QueryBonus(U32 PlayerId, stSR_History& BonusData);
	DBError DrawBonus(U32 AccountId, U32 PlayerId, U32 SRId, U8 PerMonth,
		stSR_Bonus_Data& BonusData, int& Err);
	DBError UpdateBonus(U32 PlayerId);
};

#endif//__TBLSTAGINGREBATE_H__