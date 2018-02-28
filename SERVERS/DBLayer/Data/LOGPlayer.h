#ifndef __LOGPLAYER_H__
#define __LOGPLAYER_H__

#ifndef __LOGBASE_H__
#include "LOGBase.h"
#endif

class LOGPlayer: public LOGBase
{
public:
	LOGPlayer(DataBase* db):LOGBase(db) {}
	virtual ~LOGPlayer() {}
	DBError Save(void* Info);
};

#endif//__LOGPLAYER_H__