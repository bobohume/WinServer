#ifndef __TBLDISCOUNT_H__
#define __TBLDISCOUNT_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

struct stDiscount;
class TBLDiscount: public TBLBase
{
public:
	TBLDiscount(DataBase* db);
	virtual ~TBLDiscount();
	DBError LoadAll(std::vector<stDiscount*>& info);
	DBError Load(std::vector<stDiscount*>& info);
	DBError Save(std::vector<stDiscount*>& info);
};

#endif//__TBLDISCOUNT_H__