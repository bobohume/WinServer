#ifndef __LOGSKILL_H__
#define __LOGSKILL_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGSkill: public LOGBase
{
public:
	LOGSkill(DataBase* db):LOGBase(db) {}
	virtual ~LOGSkill() {}
	DBError Save(void* Info);
};

#endif//__LOGSKILL_H__