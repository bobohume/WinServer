#ifndef __LOGMISSION_H__
#define __LOGMISSION_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGMission: public LOGBase
{
public:
	LOGMission(DataBase* db):LOGBase(db) {}
	virtual ~LOGMission() {}
	DBError Save(void* Info);
};

class LOGMissionDetail: public LOGBase
{
public:
	LOGMissionDetail(DataBase* db):LOGBase(db) {};
	virtual ~LOGMissionDetail() {};
	DBError Save(void* Info);
};

#endif//__LOGMISSION_H__