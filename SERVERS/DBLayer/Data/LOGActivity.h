#ifndef __LOGACTIVITY_H__
#define __LOGACTIVITY_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGActivity: public LOGBase
{
public:
	LOGActivity(DataBase* db):LOGBase(db) {}
	virtual ~LOGActivity() {}
	DBError Save(void* Info);
};

#endif//__LOGACTIVITY_H__