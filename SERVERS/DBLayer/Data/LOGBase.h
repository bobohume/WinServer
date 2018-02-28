#ifndef __LOGBASE_H__
#define __LOGBASE_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

class LOGBase: public TBLBase
{
public:
	LOGBase(DataBase* db) : TBLBase(db) {}
	virtual ~LOGBase() {}
	virtual DBError Save(void* Info) = 0;
};

#endif//__LOGBASE_H__