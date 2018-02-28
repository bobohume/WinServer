#ifndef __LOGSPIRIT_H__
#define __LOGSPIRIT_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGSpirit: public LOGBase
{
public:
	LOGSpirit(DataBase* db):LOGBase(db) {}
	virtual ~LOGSpirit() {}
	DBError Save(void* Info);
};

class LOGSpiritDetail: public LOGBase
{
public:
	LOGSpiritDetail(DataBase* db):LOGBase(db) {}
	virtual ~LOGSpiritDetail() {}
	DBError Save(void* Info);
};
#endif//__LOGSPIRIT_H__