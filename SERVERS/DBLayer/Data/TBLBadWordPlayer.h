#ifndef __TBLBADWORDPLAYER_H__
#define __TBLBADWORDPLAYER_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>
#include "Common/ZoneSetting.h"

class CDBConn;

class TBLBadWordPlayer: public TBLBase
{
public:
	TBLBadWordPlayer(DataBase* db):TBLBase(db) {}
	virtual ~TBLBadWordPlayer() {}
	DBError SaveBadWordPlayer(int playerId, const char* badWord);
};

#endif