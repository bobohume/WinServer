#ifndef _DB_PURCHASE_RECORD_
#define _DB_PURCHASE_RECORD_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"

class DB_PurchaseRecord
	: public ThreadBase
{
public:
	DB_PurchaseRecord();

	virtual int Execute(int ctxId, void* param);

	U32 playerId;
	U32 quantity;
	std::string transaction_id;
	std::string product_id;
	std::string item_id;
	std::string purchase_date;
	U32 mError;
	U32 nGold;
	U32 nMoney;
	U32 nCopperCoil;;
	U32 nSilverCoil;
	U32 nGoldCoil;
};

#endif /*_DB_SPONSOR_ACTIVITY_H_*/