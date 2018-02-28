#ifndef __LOGMONEY_H__
#define __LOGMONEY_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGMoney: public LOGBase
{
public:
	LOGMoney(DataBase* db):LOGBase(db) {}
	virtual ~LOGMoney() {}
	DBError Save(void* Info);
};

#endif//__LOGMONEY_H__