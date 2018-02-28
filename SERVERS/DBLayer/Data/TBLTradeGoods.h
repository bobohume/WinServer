#ifndef __TBLTRADEGOODS_H__
#define __TBLTRADEGOODS_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

struct stTradeGoods;
class TBLTradeGoods: public TBLBase
{
public:
	TBLTradeGoods(DataBase* db) : TBLBase( db ) {};
	virtual ~TBLTradeGoods() {};
	DBError Save(stTradeGoods& info);
};

#endif//__TBLTRADEGOODS_H__