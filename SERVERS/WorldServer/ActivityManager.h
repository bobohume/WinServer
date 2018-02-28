#ifndef BATTLE_MANAGER_H
#define BATTLE_MANAGER_H

#include <hash_map>
#include "WINTCP/IPacket.h"
#include "Common/PlayerStruct.h"
#include "BaseMgr.h"
#include "ActivityBase.h"
#include <set>
#include <deque>

/************************************************************************/
/* 战斗管理对象
/************************************************************************/
class CActivityMgr : public CBaseManager<>
{
#define MAX_BATTLE_NUM 4
	typedef std::deque<stBattleBase>					 BATTLE_DEQ;

public:
	CBattleManager();
	~CBattleManager();

	
	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);
	bool UpdateDB(U32 nPlayerId, BattleInfoRef pBattle);
	bool InsertDB(U32 nPlayerID, BattleInfoRef pBattle);

	static void Export(struct lua_State* L);
};

#define BATTLEMGR CBattleManager::Instance()
#endif//BATTLE_MANAGER_H
