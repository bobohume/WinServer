#ifndef __TBLREWARD_H__
#define __TBLREWARD_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#ifndef __REWARDBASE_H__
#include "Common/RewardBase.h"
#endif//__REWARDBASE_H__

class TBLReward: public TBLBase
{
public:
	TBLReward(DataBase* db);
	virtual ~TBLReward();
	DBError Query(int AccountID, stAccountReward* reward);
	DBError Reset(int AccountID, int RewardID);
	DBError Update(stAccountReward* reward);
};

#endif//__TBLREWARD_H__