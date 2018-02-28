#ifndef TBL_ACTOR_KEYMAP_H
#define TBL_ACTOR_KEYMAP_H


#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include "Common/KeyMapBase.h"

class CDBConn;

class TBLActorKeyMap : public TBLBase
{
public:
	TBLActorKeyMap(DataBase* db) : TBLBase( db ) {}
	virtual ~TBLActorKeyMap() {}

	DBError load( int playerId, stKeyMapBase& keyMaps );
	DBError save( int playerId, stKeyMapBase& keyMaps );
};

#endif