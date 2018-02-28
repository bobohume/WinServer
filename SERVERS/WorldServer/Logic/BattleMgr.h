#ifndef BATTLE_MANAGER_H
#define BATTLE_MANAGER_H

#include <hash_map>
#include "WINTCP/IPacket.h"
#include "Common/PlayerStruct.h"
#include "BaseMgr.h"
#include "BattleBase.h"
#include <set>
#include <deque>

/************************************************************************/
/* 战斗管理对象
/************************************************************************/
class CBattleManager : public CBaseManager<BattleInfoRef>
{
#define MAX_BATTLE_NUM 4
	typedef std::deque<stBattleBase>					 BATTLE_DEQ;

public:
	CBattleManager();
	~CBattleManager();

	//计算玩家战斗属性
	void CaculateBatlleInfo(U32 nPlayerID, BattleInfoRef pBattle);
	enOpError Battle(U32 nPlayerID, U32 nGateID, BATTLE_DEQ& BattleList);
	enOpError BattleEx(U32 nPlayerID, U32 nMapID, BATTLE_DEQ& BattleList);//扫荡
	bool _Battle(stBattleBase& Source, stBattleBase& Target, S32 iBattleNum, BATTLE_DEQ& BattleList);

	static CBattleManager* Instance();
	bool AddBattle(U32 nPlaydID);
	enOpError SetBattle(U32 nPlayerID, std::string UID[MAX_BATTLE_NUM]);
	void UpdateToClient(U32 nPlayerID, BattleInfoRef pBattle);
	void ReflashTime(U32 nPlayerID);
	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);
	bool UpdateDB(U32 nPlayerId, BattleInfoRef pBattle);
	bool InsertDB(U32 nPlayerID, BattleInfoRef pBattle);

	//----------消息处理----------//
	bool HandleClientBattleInfoRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientBattleCombatRequset(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientBattleCombatExRequset(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	static void Export(struct lua_State* L);
};

#define BATTLEMGR CBattleManager::Instance()
#endif//BATTLE_MANAGER_H
