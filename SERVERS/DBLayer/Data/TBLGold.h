#ifndef __TBLGOLD_H__
#define __TBLGOLD_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

class TBLGold: public TBLBase
{
public:
	TBLGold(DataBase* db);
	virtual ~TBLGold();
	DBError QueryGold(int AccountID, int PlayerId, int& Gold, int& TotalDrawGolds);
	DBError UpdateGold(int AccountID, int Gold, int GoldOpType, int PlayerID,
			int& LeftGold, int& TotalDrawGold);
	DBError QueryTotalDrawGolds(int PlayerId, int& Gold);
};

#endif//__TBLGOLD_H__

