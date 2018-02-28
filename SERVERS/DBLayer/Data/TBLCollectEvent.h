#ifndef __TBLCOLLECTEVENT_H__
#define __TBLCOLLECTEVENT_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

struct stCollectEvent;
class TBLCollectEvent: public TBLBase
{
public:
	TBLCollectEvent(DataBase* db):TBLBase(db) {}
	virtual ~TBLCollectEvent() {}
	DBError Load(std::vector<stCollectEvent>& info);
	DBError Save(const stCollectEvent& info);
};

#endif//__TBLCOLLECTEVENT_H__