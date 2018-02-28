#ifndef _TBLARENA_H_
#define _TBLARENA_H_

#include "base/types.h"

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include "Common/PlayerStruct.h"
#include "Common/ChallengeBase.h"

struct ArenaDBInfo
{
public:
    ArenaData m_datas[4];
};

typedef stdext::hash_map<U32,ArenaDBInfo> ARENADATA_MAP;

class TBLArena : public TBLBase
{
public:
    TBLArena(DataBase* db):TBLBase(db) {}
    
    DBError	LoadAllArenas(ARENADATA_MAP& map,int season,int& lastSeason,bool& isRewarded);

	//载入玩家的竞技场信息
    DBError	LoadArenaInfo(U32 playerId,U32 type,ArenaData* pData);
    DBError	UpdateArenaInfo(U32 playerId,U32 type,ArenaData& data,int season);
    DBError	ClearArena(int season);
    DBError Reward(int season,int item,const char* title,int playerId,int rank);

    //玩家挑战相关数据
    DBError SaveChallengeMember(ChallengeMember* pMember);
    DBError LoadChallengeMembers(std::vector<ChallengeMember*>& members);
} ;

#endif
