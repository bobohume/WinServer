#ifndef __TBLRECOMMEND_H__
#define __TBLRECOMMEND_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

struct stRecommend;
class TBLRecommend: public TBLBase
{
public:
	TBLRecommend(DataBase* db);
	virtual ~TBLRecommend();
	DBError Load(std::vector<stRecommend*>& info);
};

#endif//__TBLRECOMMEND_H__