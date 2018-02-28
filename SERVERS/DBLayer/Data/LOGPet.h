#ifndef __LOGPET_H__
#define __LOGPET_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGPet: public LOGBase
{
public:
	LOGPet(DataBase* db):LOGBase(db) {}
	virtual ~LOGPet() {}
	DBError Save(void* Info);
};

class LOGPetDetail: public LOGBase
{
public:
	LOGPetDetail(DataBase* db):LOGBase(db) {}
	virtual ~LOGPetDetail() {}
	DBError Save(void* Info);
};

#endif//__LOGPET_H__