#ifndef __LOGMOUNT_H__
#define __LOGMOUNT_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGMount: public LOGBase
{
public:
	LOGMount(DataBase* db):LOGBase(db) {}
	virtual ~LOGMount() {}
	DBError Save(void* Info);
};

class LOGMountDetail: public LOGBase
{
public:
	LOGMountDetail(DataBase* db):LOGBase(db) {}
	virtual ~LOGMountDetail() {}
	DBError Save(void* Info);
};

#endif//__LOGMOUNT_H__