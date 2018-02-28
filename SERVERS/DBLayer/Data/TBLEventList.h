#ifndef __TBLEVENTLIST_H__
#define __TBLEVENTLIST_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

struct stEventItem;

class TBLEventList: public TBLBase
{
public:
	TBLEventList(DataBase* db):TBLBase(db) {}
	virtual ~TBLEventList() {}
	DBError LoadAll(U32 nRecver, std::vector<stEventItem*>& Info);
	DBError Delete(stEventItem* item, int reason);
	DBError Save(stEventItem* item);
	DBError UpdateState(U64 id, int state);
	DBError BatchUpdateState(std::vector<U64>& idList, int state);
};

#endif//__TBLEVENTLIST_H__