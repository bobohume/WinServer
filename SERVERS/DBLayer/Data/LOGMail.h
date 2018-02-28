#ifndef __LOGMAIL_H__
#define __LOGMAIL_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGMail: public LOGBase
{
public:
	LOGMail(DataBase* db):LOGBase(db) {}
	virtual ~LOGMail() {}
	DBError Save(void* Info);
};

#endif//__LOGMAIL_H__