#ifndef __LOGACCOUNT_H__
#define __LOGACCOUNT_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGAccount: public LOGBase
{
public:
	LOGAccount(DataBase* db):LOGBase(db) {}
	virtual ~LOGAccount() {}
	DBError Save(void* Info);
};

#endif//__LOGACCOUNT_H__