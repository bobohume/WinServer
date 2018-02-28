#ifndef __LOGORG_H__
#define __LOGORG_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGOrg: public LOGBase
{
public:
	LOGOrg(DataBase* db):LOGBase(db) {}
	virtual ~LOGOrg() {}
	DBError Save(void* Info);
};

#endif//__LOGORG_H__