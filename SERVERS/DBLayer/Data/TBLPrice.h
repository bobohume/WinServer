#ifndef __TBLPRICE_H__
#define __TBLPRICE_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>
#include "common/BillingBase.h"

struct stGoodsPrice;
class TBLPrice: public TBLBase
{
public:
	TBLPrice(DataBase* db);
	virtual ~TBLPrice();
	bool	CheckUpdate(int oldver, int& newver);
	DBError Load(std::vector<stGoodsPrice*>& info);
};

class TBLBoardMsg: public TBLBase
{
public:
	TBLBoardMsg(DataBase* db):TBLBase(db) {};
	virtual ~TBLBoardMsg() {};
	DBError Load(BOARDVEC& info);
};

#endif//__TBLPRICE_H__