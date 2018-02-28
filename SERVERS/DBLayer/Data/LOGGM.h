#ifndef __LOGGM_H__
#define __LOGGM_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGGM: public LOGBase
{
public:
	LOGGM(DataBase* db):LOGBase(db) {}
	virtual ~LOGGM() {}
	DBError Save(void* Info);
};

#endif//__LOGGM_H__