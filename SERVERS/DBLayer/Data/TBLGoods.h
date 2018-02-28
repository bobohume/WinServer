#ifndef __TBLGOODS_H__
#define __TBLGOODS_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

struct stGoods;
class TBLGoods: public TBLBase
{
public:
	TBLGoods(DataBase* db);
	virtual ~TBLGoods();
	DBError Load(std::vector<stGoods*>& info);
	DBError LoadAll(std::vector<stGoods*>& info);
	//DBError Save(std::vector<stGoods*>& info);
};

#endif//__TBLGOODS_H__