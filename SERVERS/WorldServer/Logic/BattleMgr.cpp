#include "BattleMgr.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "../WorldServer.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "../Script/lua_tinker.h"
#include "Common/MemGuard.h"
#include "Common/mRandom.h"
#include "../PlayerMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "CardMgr.h"
#include "PlayerInfoMgr.h"
#include "SkillData.h"
#include "BuffData.h"
#include "CopyData.h"
#include "TimeSetMgr.h"
#include "ItemMgr.h"
#include "TaskMgr.h"
#include "VIPMgr.h"
#include "PrincessMgr.h"

static std::vector<S32> s_BattleTimer = { 
BATTLE_TIMER1, BATTLE_TIMER2, BATTLE_TIMER3, BATTLE_TIMER4, BATTLE_TIMER5, BATTLE_TIMER6, 
BATTLE_TIMER7
};

CBattleManager::CBattleManager()
{
	g_CopyDataMgr->read();
	REGISTER_EVENT_METHOD("CW_BATTLE_INFO_REQUEST", this, &CBattleManager::HandleClientBattleInfoRequest);
	REGISTER_EVENT_METHOD("CW_BATTLE_COMBAT_REQUEST", this, &CBattleManager::HandleClientBattleCombatRequset); 
	REGISTER_EVENT_METHOD("CW_BATTLE_COMBAT_EX_REQUEST", this, &CBattleManager::HandleClientBattleCombatExRequset);
	/*REGISTER_EVENT_METHOD("WW_Player_Enter_To_ItemMgr", this, &CItemManager::onPlayerEnter);
	REGISTER_EVENT_METHOD("WW_Player_Leave_To_ItemMgr", this, &CItemManager::onPlayerLeave);*/
}

CBattleManager::~CBattleManager()
{
	g_CopyDataMgr->clear();
}

CBattleManager* CBattleManager::Instance()
{
	static CBattleManager s_Mgr;
	return &s_Mgr;
}

void CBattleManager::CaculateBatlleInfo(U32 nPlayerID, BattleInfoRef pBattle)
{
	g_Stats.Clear();
	pBattle->Clear();
	for (int i = 0; i < MAX_BATTLE_NUM; ++i)
	{
		if (!pBattle->Card[i].empty())
		{
			CardRef pCard = CARDMGR->GetData(nPlayerID, pBattle->Card[i]);
			if (pCard)
			{
				S32 iTroops = (i == 0) ? mCeil(pCard->Str * 20) : pCard->Str * 10;
				pBattle->Troops += iTroops;
				CARDMGR->CaculateCardStats(nPlayerID, pCard->UID, g_Stats);
			}
		}
	}

 	PlayerBaseInfoRef pPlayerInfo = PLAYERINFOMGR->GetData(nPlayerID);
	if (pPlayerInfo)
	{
		pBattle->SoldierHp = pPlayerInfo->HpEx;
		pBattle->SoldierAtk = pPlayerInfo->AttackEx;
		if(pPlayerInfo->Troops < (S64)pBattle->Troops)
			pBattle->Troops = pPlayerInfo->Troops;
	}

	pBattle->Hp_Per = ((F32)g_Stats.HP_gPC) / 100.0f;
	pBattle->Atk_Per = ((F32)g_Stats.Atk_gPc) / 100.0f;
	pBattle->Critical = g_Stats.Cri_Pc;
	pBattle->Dodge = g_Stats.Dog_gPC;

	pBattle->CaculateHp();
	pBattle->CaculateAttack();
}

//生成叛乱
auto ReflashBattle = [=](auto nPlayerID) {
	BattleInfoRef pBattle = BATTLEMGR->GetData(nPlayerID);
	if (!pBattle)
		return false;

	if (pBattle->GateId <= 7007 * 10000)
		return false;

	bool bReflash = false;
	std::set<S32> tempSet;
	std::vector<S32> tempVec;
	S32 nMapId = pBattle->GateId / 10000;
	S32 nCurMapId = 0;
	S32 nTimeId = 0;
	S32 nMaxNum = mClamp(((nMapId - 7007) / 6 + 2), 2, 7);
	auto i = 0;
	for (auto itr = s_BattleTimer.begin(); itr != s_BattleTimer.end() && i < nMaxNum; ++itr, ++i) {
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, *itr);
		if (!pTime) {
			bReflash = true;
			if (nTimeId == 0) {
				nTimeId = *itr;
			}
		}
		else {
			tempSet.insert(pTime->Flag1);
		}
	}

	if (bReflash) {
		for (auto i = 7001; i < nMapId; ++i) {
			if (tempSet.find(i) == tempSet.end()) {
				tempVec.push_back(i);
			}
		}

		if (!tempVec.empty()) {
			S32 nRand = gRandGen.randI(0, tempVec.size() - 1);
			nCurMapId = tempVec[nRand];
			TIMESETMGR->AddTimeSet(nPlayerID, nTimeId, 0, "", nCurMapId);
		}
	}
	return true;
};

auto HasBattle = [=](auto nPlayerID, auto nMapID) {
	for (auto itr = s_BattleTimer.begin(); itr != s_BattleTimer.end(); ++itr) {
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, *itr);
		if (pTime && pTime->Flag1 == nMapID) {
			return true;
		}
	}
	return false;
};

auto RemoveBattle = [=](auto nPlayerID, auto nMapID) {
	for (auto itr = s_BattleTimer.begin(); itr != s_BattleTimer.end(); ++itr) {
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, *itr);
		if (pTime && pTime->Flag1 == nMapID) {
			TIMESETMGR->RemoveTimeSet(nPlayerID, *itr);
		}
	}
	return false;
};

enOpError CBattleManager::Battle(U32 nPlayerID, U32 nGateID, BATTLE_DEQ& BattleList)
{
	BattleInfoRef pBattle = GetData(nPlayerID);
	if (!pBattle)
		return OPERATOR_HASNO_BATTLEINFO;

	CGateData* pGateData = g_CopyDataMgr->getGateData(nGateID);
	if (!pGateData)
		return OPERATOR_HASNO_GATEID;

	if (pBattle->GateId != nGateID)
		return OPERATOR_GATE_PASSED;

	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, GATE_IMTER);
	if (pTime)
	{
		S32 CurrentGateId = atoi(pTime->Flag.c_str());
		if (0 != CurrentGateId && CurrentGateId == nGateID)
		{
			return OPERATOR_GATE_PASSED;
		}
	}

	//兵力不够
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pPlayer || (pPlayer->Troops <= (S64)0))
		return OPERATOR_HASNO_TROOPS;

	stBattleBase Source = *((stBattleBase*)pBattle.get());
	stBattleBase Target(pGateData->m_SoldierHp, pGateData->m_SoldierAtk, pGateData->m_Troops);

	BattleList.push_back(Source);
	BattleList.push_back(Target);

	_Battle(Source, Target, 5, BattleList);

	SUBMIT(nPlayerID, 13, 1);

	//战斗胜利
	if (Target.Troops <= 0 || Target.Hp <= 0)
	{
		pBattle->GateId = g_CopyDataMgr->getNextGateData(nGateID);
		if (U32_MAX == pBattle->GateId) {
			pBattle->GateId = nGateID;
		}

		S32 nNextCopyID = pBattle->GateId / 10000;
		TIMESETMGR->AddTimeSet(nPlayerID, GATE_IMTER, 0, std::to_string(nGateID).c_str(), pBattle->GateId);
		UpdateDB(nPlayerID, pBattle);
		UpdateToClient(nPlayerID, pBattle);
		PLAYERINFOMGR->AddMoney(nPlayerID, pGateData->m_AddMoney);
		PLAYERINFOMGR->AddLand(nPlayerID, pGateData->m_AddLand);

		S32 nCardNum = 0;
		for (int i = 0; i < MAX_BATTLE_NUM; ++i)
		{
			if (!pBattle->Card[i].empty())
				++nCardNum;
		}
		nCardNum = mClamp(nCardNum, 1, 4);
		S32 nAddachievement = mCeil(mClampF((F32)((pGateData->m_AddAchievement * 1.0f) / nCardNum), 1.0f, S32_MAX));
		for (int i = 0; i < MAX_BATTLE_NUM; ++i)
		{
			if (!pBattle->Card[i].empty())
			{
				g_Stats.Clear();
				CARDMGR->CaculateCardStats(nPlayerID, pBattle->Card[i], g_Stats);
				CARDMGR->AddCardAchievement(nPlayerID, pBattle->Card[i], nAddachievement);
			}
		}

		SUBMIT(nPlayerID, 28, pBattle->GateId);

		//成功以后疆土奖励
		if (pGateData->m_CopyID != nNextCopyID)
		{
			//第一次打完生产平乱
			if (pBattle->GateId == 70070001) {
				ReflashBattle(nPlayerID);
			}
			CCopyData* pCopyData = g_CopyDataMgr->getData(pGateData->m_CopyID);
			if (pCopyData)
			{
				//添加妃子
				std::vector<U32> PricessIDs;
				if (PRINCESSMGR->FindPrincessIdsBySupport(pGateData->m_GateID, PricessIDs)) {
					for (auto item : PricessIDs)
					{
						PRINCESSMGR->AddData(nPlayerID, item, 0);
					}
				}

				for (int i = 0; i < 4; ++i)
				{
					if (pCopyData->m_AwardItem[i] && pCopyData->m_AwardNum[i])
						ITEMMGR->AddItem(nPlayerID, pCopyData->m_AwardItem[i], pCopyData->m_AwardNum[i]);
				}
			}
		}
	}
	//扣除兵力
	S32 iReduceTroops = pBattle->Troops - Source.Troops;
	PLAYERINFOMGR->AddTroops(nPlayerID, -iReduceTroops);
	return OPERATOR_NONE_ERROR;
}

//扫荡
enOpError CBattleManager::BattleEx(U32 nPlayerID, U32 nMapID, BATTLE_DEQ& BattleList)
{
	BattleInfoRef pBattle = GetData(nPlayerID);
	if (!pBattle)
		return OPERATOR_HASNO_BATTLEINFO;

	CCopyData* pMapData = g_CopyDataMgr->getData(nMapID);
	if (!pMapData)
		return OPERATOR_HASNO_GATEID;

	if (pBattle->GateId / 10000 <= nMapID)
		return OPERATOR_GATE_PASSED;

	if (!HasBattle(nPlayerID, nMapID))
		return OPERATOR_GATE_PASSED;

	//兵力不够
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pPlayer || (pPlayer->Troops <= (S64)0))
		return OPERATOR_HASNO_TROOPS;

	stBattleBase Source = *((stBattleBase*)pBattle.get());
	stBattleBase Target(pMapData->m_SoldierHp, pMapData->m_SoldierAtk, pMapData->m_Rebellion);

	BattleList.push_back(Source);
	BattleList.push_back(Target);

	_Battle(Source, Target, 5, BattleList);

	SUBMIT(nPlayerID, 13, 1);
	SUBMIT(nPlayerID, 64, 1);

	//战斗胜利
	if (Target.Troops <= 0 || Target.Hp <= 0)
	{
		for (auto i = 0; i < 4; ++i) {
			ITEMMGR->DropItem(nPlayerID, pMapData->m_RebellionItem[i]);
		}
	}

	RemoveBattle(nPlayerID, nMapID);
	//扣除兵力
	S32 iReduceTroops = pBattle->Troops - Source.Troops;
	PLAYERINFOMGR->AddTroops(nPlayerID, -iReduceTroops);
	return OPERATOR_NONE_ERROR;
}

bool CBattleManager::_Battle(stBattleBase& Source, stBattleBase& Target, S32 iBattleNum, BATTLE_DEQ& BattleList)
{
	S32 iCurBattleNum = 0;
	do 
	{
		bool bCritical = (gRandGen.randI(1, 100) < Source.Critical);//暴击
		bool bDodge = (gRandGen.randI(1, 100) < Source.Dodge);//闪避
		Target.Hp -= (bCritical ? Source.Attack * 2 : Source.Attack);
		Source.Hp -= (bDodge ? 0 : Target.Attack);

		Target.CaculateTroops();
		Target.CaculateHp();
		Target.CaculateAttack();

		Source.CaculateTroops();
		Source.CaculateHp();
		Source.CaculateAttack();

		BattleList.push_back(Source);
		BattleList.push_back(Target);
		iCurBattleNum++;

		if(Source.Hp <= 0 || Source.Troops <= 0 || Target.Troops <= 0 || Target.Hp <= 0 || iCurBattleNum >= iBattleNum)
			break;

	} while (true);
	return true;
}

bool CBattleManager::AddBattle(U32 nPlaydID)
{
	BattleInfoRef pBattle = BattleInfoRef(new stBattleInfo);
	if (pBattle)
	{
		pBattle->GateId = 70010001;
		AddData(nPlaydID, pBattle);
		InsertDB(nPlaydID, pBattle);
		UpdateToClient(nPlaydID, pBattle);
		return true;
	}

	return false;
}

enOpError CBattleManager::SetBattle(U32 nPlayerID, std::string UID[MAX_BATTLE_NUM])
{
	typedef std::vector<std::string> CARD_VEC;
	CARD_VEC CardVec;
	BattleInfoRef pBattle = GetData(nPlayerID);
	if (!pBattle)
		return OPERATOR_HASNO_BATTLEINFO;

	bool bUpdate = false;
	for (int i = 0; i < MAX_BATTLE_NUM; ++i)
	{
		if (!UID[i].empty())
		{
			CardRef pCard = CARDMGR->GetData(nPlayerID, UID[i]);
			if (pCard)
			{
				//主键重复
				for (auto cUid : CardVec) {
					if (0 == cUid.compare(UID[i]))
					{
						return OPERATOR_BATTLE_CARD_EXIST;
					}
				}

				CardVec.push_back(CARD_VEC::value_type(UID[i]));
				bUpdate = true;
			}
		}
	}

	if (bUpdate)
	{
		U32 nSlot = 0;
		for (CARD_VEC::iterator itr = CardVec.begin(); itr != CardVec.end(); ++itr)
		{
			pBattle->Card[nSlot++] = *itr;
		}

		UpdateDB(nPlayerID, pBattle);
		UpdateToClient(nPlayerID, pBattle);
		CaculateBatlleInfo(nPlayerID, pBattle);
	}

	return (bUpdate) ? (OPERATOR_NONE_ERROR) : (OPERATOR_PARAMETER_ERROR);
}

void CBattleManager::ReflashTime(U32 nPlayerID)
{
	ReflashBattle(nPlayerID);
}

DB_Execution* CBattleManager::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	BattleInfoRef pBattle = BattleInfoRef(new stBattleInfo);
	BATTLEMGR->AddData(nPlayerID, pBattle);
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, GATE_IMTER);
	if (pTime)
	{
		pBattle->GateId = pTime->Flag1;
	}
	return NULL;
}

bool CBattleManager::UpdateDB(U32 nPlayerId, BattleInfoRef pBattle)
{
	/*DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Battle SET Card0='%s', Card1='%s', Card2='%s', Card3='%s', CardNum=%d, GateId=%d WHERE PlayerID=%d", \
		pBattle->Card[0].c_str(), pBattle->Card[1].c_str(), pBattle->Card[2].c_str(), pBattle->Card[3].c_str(), pBattle->CardNum, pBattle->GateId, nPlayerId));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);*/
	return true;
}

bool CBattleManager::InsertDB(U32 nPlayerID, BattleInfoRef pBattle)
{
	/*DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("INSERT INTO  Tbl_Battle \
			 (PlayerID,		CardNum, GateId) \
			 VALUES(%d,		%d,		 %d)", \
			  nPlayerID,	0,		 pBattle->GateId));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);*/
	return true;
}


void CBattleManager::UpdateToClient(U32 nPlayerID, BattleInfoRef pBattle)
{
	/*AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(512 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 512);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_BATTLE_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pBattle->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}*/
}

bool CBattleManager::HandleClientBattleInfoRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		BattleInfoRef pBattle = GetData(pAccount->GetPlayerId());
		if (pBattle)
		{
			CMemGuard Buffer(512 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 512);
			stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_BATTLE_INFO_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
			sendPacket.writeInt(0, Base::Bit16);
			CaculateBatlleInfo(pAccount->GetPlayerId(), pBattle);
			pBattle->WriteData(&sendPacket);
			pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
		}
	}
	return true;
}

bool CBattleManager::HandleClientBattleCombatRequset(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nGateID = Packet->readInt(Base::Bit32);
	U32 nSize = Packet->readInt(Base::Bit16);
	nSize = (nSize > 4) ? 4 : nSize;
	char strUID[MAX_UUID_LENGTH] = "";
	std::string UID[4];
	for (int i = 0; i < nSize; ++i)
	{
		if (Packet->readFlag())
		{
			memset(strUID, 0, sizeof(strUID));
			Packet->readString(strUID, MAX_UUID_LENGTH);
			UID[i] = strUID;
		}
	}
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = SetBattle(pAccount->GetPlayerId(), UID);
		BATTLE_DEQ BattleList;
		if(nError == OPERATOR_NONE_ERROR)
		{ 
			nError = Battle(pAccount->GetPlayerId(), nGateID, BattleList);
		}
		CMemGuard Buffer(8096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 8096);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_BATTLE_COMBAT_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(BattleList.size(), Base::Bit16);
		for(auto itr = BattleList.begin(); itr != BattleList.end(); ++itr)
		{
			sendPacket.writeFlag(true);
			(*itr).WriteData(&sendPacket);
		}
		//sendPacket.writeFlag(VIPMGR->CanJumpBattleAnim(pAccount->GetPlayerId()));
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CBattleManager::HandleClientBattleCombatExRequset(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nMapId = Packet->readInt(Base::Bit32);
	U32 nSize = Packet->readInt(Base::Bit16);
	nSize = (nSize > 4) ? 4 : nSize;
	char strUID[MAX_UUID_LENGTH] = "";
	std::string UID[4];
	for (int i = 0; i < nSize; ++i)
	{
		if (Packet->readFlag())
		{
			memset(strUID, 0, sizeof(strUID));
			Packet->readString(strUID, MAX_UUID_LENGTH);
			UID[i] = strUID;
		}
	}
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = SetBattle(pAccount->GetPlayerId(), UID);
		BATTLE_DEQ BattleList;
		if (nError == OPERATOR_NONE_ERROR)
		{
			nError = BattleEx(pAccount->GetPlayerId(), nMapId, BattleList);
		}
		CMemGuard Buffer(8096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 8096);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_BATTLE_COMBAT_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(BattleList.size(), Base::Bit16);
		for (auto itr = BattleList.begin(); itr != BattleList.end(); ++itr)
		{
			sendPacket.writeFlag(true);
			(*itr).WriteData(&sendPacket);
		}
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

void ___LoginBattle(int playerid)
{
	BATTLEMGR->LoadDB(playerid);
}

void CBattleManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "LoginBattle", &___LoginBattle);
}
