#ifndef __TBLWORLDSETTING_H__
#define __TBLWORLDSETTING_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

struct stZoneSetting;
class TBLZoneSetting: public TBLBase
{
public:
	TBLZoneSetting(DataBase* db):TBLBase(db) {}
	virtual ~TBLZoneSetting() {};
	DBError Load(stZoneSetting& info,int lineid = 0, int zoneid = 0);
	DBError Save(stZoneSetting* info, int lineid = 0, int zoneid = 0);
};

#endif//__TBLWORLDSETTING_H__