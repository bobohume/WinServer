#ifndef __BOSS_MGR_H
#define __BOSS_MGR_H
#include "BossBase.h"
#include "BaseMgr.h"
#include "Base/tVector.h"
#include <memory>


class CBossMgr
{
	typedef CBaseManager<BossRef>   Parent;
	typedef std::unordered_map<S32, BossBattleInfoRef>			BATTLE_MAP;
	typedef std::unordered_map<S32, BossBattleRankRef>			BATTLE_RANK_MAP;
	typedef std::unordered_map<S32, BATTLE_MAP>				BOSS_BATTLE_MAP;
	typedef std::unordered_map<S32, BATTLE_RANK_MAP>		BOSS_BATTLE_RANK_MAP;
	typedef std::unordered_map<S32, BossRef>				BOSS_MAP;
public:
	CBossMgr();
	virtual ~CBossMgr();

	static CBossMgr* Instance();

	enOpError CreateBoss(S32 nID, S32 nDataID);
	enOpError BattleBoss(S32 nPlayerID, S32 nID, std::string UID[3]);
	void AttackBoss(BossRef pData, BossBattleInfoRef pBattle);
	enOpError BuyBuff(S32 nPlayerID, S32 nID, S32 Type);
	//---------Notify Operator---------//
	void SendInitToClient(U32 nPlayerID);

	BossRef	  GetData(S32 nID);
	void	  AddData(BossRef pData);
	//---------DB Operator---------//
	void LoadDB();

	bool UpdateDB(BossRef pData);
	bool InsertDB(BossRef pData);
	bool DeleteDB(S32 ID);

	void TimeProcess(U32 bExit);
	bool HandleBossOpenOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleBossBuyBuffResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleBossInfoResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleBossBattleResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	static void Export(struct lua_State* L);
private:
	BOSS_BATTLE_MAP			m_BossBattleMap;
	BOSS_BATTLE_RANK_MAP	m_BossBattleRankMap;
	BOSS_MAP				m_BossMap;
};

#define BOSSMGR CBossMgr::Instance()
#endif//__BOSS_MGR_H