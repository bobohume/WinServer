#ifndef PLAYERINFO_MANAGER_H
#define PLAYERINFO_MANAGER_H

#include <hash_map>
#include "PlayerInfoBase.h"
#include "TimeSetBase.h"
#include "WINTCP/IPacket.h"
#include "BaseMgr.h"
#include "Common/Common.h"

enum enAddType
{
	Business,
	Military,
	Technology,
	Money,
	Ore,
	Troops,
	Attack,
	Hp,
	Land,
	BattleVal,
	PveMoney,
	CashGift,
	Conquest,
};

/************************************************************************/
/* 管理角色基础数据
/************************************************************************/
class CPlayerInfoManager : public CBaseManager<PlayerBaseInfoRef>
{
	typedef CBaseManager<PlayerBaseInfoRef> Parent;
public:
	CPlayerInfoManager();
	~CPlayerInfoManager();

	static CPlayerInfoManager* Instance();
	static void Initialize();
	static void AnsyDB(S32 Id, std::string sqlstr);
	//新注册玩家信息
	void AddPlayer(U32 nPlayerID);
	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);
	PlayerBaseInfoRef SyncLoadDB(U32 nPlayerID);
	bool InsertDB(U32 nPlayerID, PlayerBaseInfoRef pData);
	bool DeleteDB(U32 nPlayerID);
	bool UpdateDB(U32 nPlayerID, PlayerBaseInfoRef pData);
	//----------------------------------
	//---------一些属性操作-------//
	bool CanAddValue(U32 nPlayerID, enAddType type, S32 addVal);
	bool AddBusiness(U32 nPlayerID, S32 val);
	bool AddMilitary(U32 nPlayerID, S32 val);
	bool AddTechnology(U32 nPlayerID, S32 val);
	bool AddMoney(U32 nPlayerID, S32 val);
	bool AddOre(U32 nPlayerID, S32 val);
	bool AddTroops(U32 nPlayerID, S32 val);
	bool AddAttack(U32 nPlayerID, S32 val);
	bool AddHp(U32 nPlayerID, S32 val);
	bool AddLand(U32 nPlayerID, S32 val);
	bool AddBattleVal(U32 nPlayerID, S32 val);
	bool AddPLearnNum(U32 nPlayerID, S32 val);
	bool AddPPlantNum(U32 nPlayerID, S32 val);
	bool AddCardDevNum(U32 nPlayerID, S32 val);
	bool AddPveScore(U32 nPlayerID, S32 val);
	bool AddPveMoney(U32 nPlayerID, S32 val);
	bool AddQueenVal(U32 nPlayerID, S32 val, bool bSubmit = false);
	bool AddCashGift(U32 nPlayerID, S32 val);
	bool AddConquest(U32 nPlayerID, S32 val);
	bool CaculateBattleVal(U32 nPlayerID);
	void AddBuff(U32 nPlayerID, S32 pos, stBuff& buff, bool bNotity = false);
	PlayerBaseInfoRef GetPlayer(U32 nPlayerID);
	enOpError Impose(U32 nPlayerID, U32 nType, U32& nAddVal);
	void NotifyTimeDel(U32 nPlayerID, TimeSetRef pTime);
	bool AddImposeTimes(U32 nPlayerID, S32 val);
	S32  GetImposeTimes(U32 nPlayerID);
	bool SetFace(U32 nPlayerID, S32 val);
	bool SetPlayerFlag(U32 nPlayerID, S32 nSlot, S32& nFlag);
	void SetPlayerOrg(U32 nPlayerID, S32 OrgId);
	static void NewInCostActivity(S32 nPlayerID, enActivityType Type, S32 nVal);
	//-----------每小时增加资源------------//
	void TimeProcess(U32 bExit);
	//----------通知客户端-----------//
	void SendInitToClient(U32 nPlayerID);
	void SendInitDataToClient(U32 nPlayerID);
	void UpdateToClient(U32 nPlayerID, PlayerBaseInfoRef pPlayer);
	void UpdateToClient(U32 nPlayerID);

	//----------角色登录通知------//
	bool HandleImposeRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleSetFaceRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleSetPlayerFlagRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool onPlayerEnter(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool onPlayerLeave(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool onPlayerUpdateClient(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	static void Export(struct lua_State* L);
};

static auto GetRecoverTimes = [](auto CurTime, auto OldTime, auto InterVal) {
	if (CurTime >= OldTime) {
		return mClamp((S32)((CurTime - OldTime) / InterVal), 1, 100);
	}

	return 0;
};

#define PLAYERINFOMGR CPlayerInfoManager::Instance()
#define SUBMITACTIVITY(nPlayerID, type, val) PLAYERINFOMGR->NewInCostActivity(nPlayerID, type, val)
#define ANSYDB(id, str) CPlayerInfoManager::AnsyDB(id, str)
#endif//PLAYERINFO_MANAGER_H
