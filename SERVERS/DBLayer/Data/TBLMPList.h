#ifndef __TBLMPLIST_H__
#define __TBLMPLIST_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>
#include "Common/SocialBase.h"

typedef std::vector<stLearnPrentice> VecLearnPrentice;
typedef std::vector< std::pair<int, int> > MasterCount;

class TBLMPList: public TBLBase
{
public:
	TBLMPList(DataBase* db):TBLBase(db) {}
	virtual ~TBLMPList() {}

	DBError Load(int playerID, VecLearnPrentice& info);
	DBError Save(int playerId,int masterPlayerId);
	DBError Count(MasterCount& masterCount);
};

#endif//__TBLMPLIST_H__