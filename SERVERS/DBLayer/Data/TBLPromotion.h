#ifndef __TBLPROMOTION_H__
#define __TBLPROMOTION_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include "Common/BillingBase.h"

class TBLPromotion: public TBLBase
{
public:
	TBLPromotion(DataBase* db):TBLBase(db) {}
	virtual ~TBLPromotion() {}
	DBError AddPROMAccount(int AccountId, int PROMAccountId);
	DBError BatchAddPROMAccount(int PROMAccountId, std::vector<int>& AccountList);

	DBError UpdateBonus(int PROMAccountId, std::vector<int>& AccountList);
	DBError AddBonus(int AccountId, int PlayerId, int PROMAccountId, int& money, int& exp,
		int& bindGold, int& mayday);
	DBError QueryBonus(int PROMAccountId, int& money, int& exp, int& bindGold, int& mayday);
	DBError DrawBonus(int PROMAccountId, int PROMPlayerId, 
		int& Money, int& Exp, int& bindGold, int& HistoryId);
	DBError DrawBonusForMayDay(int PROMAccountId, int PROMPlayerId, 
		int& MayDay, int& HistoryId);
	DBError CancelDrawBonus(int PROMAccountId, int HistoryId);

	DBError AddBonusForItem(int PROMAccountId, stPROMItemBonus& bonus);
	DBError DrawBonusForItem(int PROMAccountId, int PROMPlayerId, 
		int ItemId, int& ret);
};

#endif//__TBLPROMOTION_H__