#ifndef __TBLAUCTION_H__
#define __TBLAUCTION_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#pragma once
#include "Common/AuctionBase.h"
#include "Common/OrgBase.h"

class TBLAuction: public TBLBase
{
public:
	TBLAuction(DataBase* db):TBLBase(db) {}
	virtual ~TBLAuction() {}
	DBError CheckUpdate();
	DBError Query(unsigned int& lastQueryTime, std::vector<stAuctionInfo*>& auctionList);
	DBError AddNew(stAuctionInfo* info);
	DBError BuyOne(U32 id, U32 buyerId, S32 buyPrice, U32 buyTime, S32& error);
	DBError Cancel(U32 id, U32 owner, U32 cancelTime, S32& error);
};

#endif//__TBLAUCTION_H__