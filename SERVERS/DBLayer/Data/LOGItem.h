#ifndef __LOGITEM_H__
#define __LOGITEM_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGItem: public LOGBase
{
public:
	LOGItem(DataBase* db):LOGBase(db) {}
	virtual ~LOGItem() {}
	DBError Save(void* Info);
};

class LOGItemDetail: public LOGBase
{
public:
	LOGItemDetail(DataBase* db):LOGBase(db) {}
	virtual ~LOGItemDetail() {}
	DBError Save(void* Info);
};

#endif//__LOGITEM_H__