#ifndef __LOGLEAGUE_H__
#define __LOGLEAGUE_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGLeague: public LOGBase
{
public:
	LOGLeague(DataBase* db):LOGBase(db) {}
	virtual ~LOGLeague() {}
	DBError Save(void* Info);
};

#endif//__LOGLEAGUE_H__