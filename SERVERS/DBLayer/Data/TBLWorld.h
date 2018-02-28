#ifndef __TBLWORLD_H__
#define __TBLWORLD_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>
#include "Common/ZoneSetting.h"

class CDBConn;

class TBLWorld: public TBLBase
{
public:
	TBLWorld(DataBase* db):TBLBase(db) {}
	virtual ~TBLWorld() {}
	DBError QueryDisaster(int& TopLevel, int& TopDisaster);
	DBError SaveDisaster(int TopDisaster);
	DBError SaveTopLevel(int TopLevel);
	DBError SaveWorldSetting(stWorldSetting worldSetting);
	DBError QueryWorldSetting(stWorldSetting& worldSetting);
	DBError SaveStallShop(int MaxStalls, int TotalFlourish);
};

#endif//__TBLWORLD_H__