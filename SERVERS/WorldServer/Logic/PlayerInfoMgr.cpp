#include "PlayerInfoMgr.h"
#include "../WorldServer.h"
#include "../PlayerMgr.h"
#include "../Database/DB_Execution.h"
#include "Common/MemGuard.h"
#include "WINTCP/dtServerSocket.h"
#include "DBLayer/Data/TBLExecution.h"
#include "ItemMgr.h"
#include "TimeSetMgr.h"
#include "CardMgr.h"
#include "BattleMgr.h"
#include "OfficerMgr.h"
#include "BuffMgr.h"
#include "PrincessMgr.h"
#include "PrinceMgr.h"
#include "SkillMgr.h"
#include "platform.h"
#include "TopMgr.h"
#include "PvpMgr.h"
#include "VisitlMgr.h"
#include "../MailManager.h"
#include "../Script/lua_tinker.h"
#include "TaskMgr.h"
#include "Common/BanInfoData.h"
#include "DinnerMgr.h"
#include "CommLib/ToSql.h"
#include "ActivityManager.h"
#include "Org.h"
#include "Common/dbStruct.h"
#include "Common/LogHelper.h"
#include "VIPMgr.h"
#include "BossMgr.h"

DECLARE_SQL_UNIT(stPlayerBaseInfo);
CPlayerInfoManager::CPlayerInfoManager()
{
	REGISTER_EVENT_METHOD("WW_Player_Enter_To_PlayerInfoMgr", this, &CPlayerInfoManager::onPlayerEnter);
	REGISTER_EVENT_METHOD("WW_Player_Leave_To_PlayerInfoMgr", this, &CPlayerInfoManager::onPlayerLeave);
	REGISTER_EVENT_METHOD("WW_Player_UpdateClient_PlayerInfoMgr",   this, &CPlayerInfoManager::onPlayerUpdateClient);
	REGISTER_EVENT_METHOD("CW_IMPOSE_REQUEST", this, &CPlayerInfoManager::HandleImposeRequest);
	REGISTER_EVENT_METHOD("CW_SETFACE_REQUEST", this, &CPlayerInfoManager::HandleSetFaceRequest);
	REGISTER_EVENT_METHOD("CW_SETPLAYERFLAG_REQUEST", this, &CPlayerInfoManager::HandleSetPlayerFlagRequest);
	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CPlayerInfoManager::TimeProcess,  60 * 1000);
	g_BanInfoRepository.read();

	GET_SQL_UNIT(stPlayerBaseInfo).SetName("Tbl_PlayerBaseInfo");
	REGISTER_SQL_UNITKEY(stPlayerBaseInfo, PlayerID);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Business);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Military);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Technology);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Money);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Ore);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Troops);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Attack);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Hp);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Land);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, BattleVal);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, PLearnNum);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, PPlantNum);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, CardDevNum);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, PveScore);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, PveMoney);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, CashGift);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, Conquest);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, PlayerFlag);
	REGISTER_SQL_UNIT(stPlayerBaseInfo, OrgId);
}

CPlayerInfoManager::~CPlayerInfoManager()
{
	g_BanInfoRepository.clear();
}

CPlayerInfoManager* CPlayerInfoManager::Instance()
{
	static CPlayerInfoManager s_Mgr;
	return &s_Mgr;
}

void CPlayerInfoManager::Initialize()
{
	ACTIVITYMGR->LoadDB();
	TOPMGR->LoadDB();
	DINNERMGR->LoadDB();
	ORGMGR->LoadDB();
	BOSSMGR->LoadDB();
}

void CPlayerInfoManager::AnsyDB(S32 Id, std::string sqlstr)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(Id);
	pHandle->SetSql(sqlstr);
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
}

void CPlayerInfoManager::AddPlayer(U32 nPlayerID)
{
	PlayerBaseInfoRef pPlayer = PlayerBaseInfoRef(new stPlayerBaseInfo);
	if (pPlayer)
	{
		pPlayer->PlayerID = nPlayerID;
		AddData(nPlayerID, pPlayer);
		InsertDB(nPlayerID, pPlayer);
	}
}

auto GetPlayerBaseInfoDB = [](auto pHandle){
	PlayerBaseInfoRef pPlayer = PlayerBaseInfoRef(new stPlayerBaseInfo);
	pPlayer->PlayerID = pHandle->GetInt();
	pPlayer->Business = pHandle->GetInt();
	pPlayer->Military = pHandle->GetInt();
	pPlayer->Technology = pHandle->GetInt();
	pPlayer->Money = pHandle->GetBigInt();
	pPlayer->Ore = pHandle->GetBigInt();
	pPlayer->Troops = pHandle->GetBigInt();
	pPlayer->Attack = pHandle->GetInt();
	pPlayer->Hp = pHandle->GetInt();
	pPlayer->Land = pHandle->GetInt();
	pPlayer->BattleVal = pHandle->GetInt();
	pPlayer->PLearnNum = pHandle->GetInt();
	pPlayer->PPlantNum = pHandle->GetInt();
	pPlayer->CardDevNum = pHandle->GetInt();
	pPlayer->PveScore = pHandle->GetInt();
	pPlayer->PveMoney = pHandle->GetInt();
	pPlayer->CashGift = pHandle->GetInt();
	pPlayer->Conquest = pHandle->GetInt();
	pPlayer->PlayerFlag = pHandle->GetInt();
	pPlayer->OrgId = pHandle->GetInt();
	return pPlayer;
};

auto RecordLoginTime = [](U32 nPlayerID) {
	auto nCurTime = (S32)time(NULL);
	auto nTotalTime = 0;
	auto pTime = TIMESETMGR->GetData(nPlayerID, LOG_LOGIN_TIME); 
	if (pTime) {
		nTotalTime = nCurTime - pTime->Flag1;
	}

	TIMESETMGR->AddTimeSet(nPlayerID, LOG_LOGIN_TIME, 0, "", nCurTime);
	return nTotalTime;
};

DB_Execution* CPlayerInfoManager::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	{
		pHandle->SetId(nPlayerID);
		pHandle->SetSql(GET_SQL_UNIT(stPlayerBaseInfo).LoadSql("PlayerID", nPlayerID));
		pHandle->RegisterFunction([](int id, int error, void* pH) {
			DB_Execution* pHandle = (DB_Execution*)(pH);
			if (pHandle && error == NONE_ERROR)
			{
				if (pHandle->More())
				{
					PlayerBaseInfoRef pPlayer = GetPlayerBaseInfoDB(pHandle);
					PLAYERINFOMGR->AddData(id, pPlayer);
				}
				else//新注册
				{
					PLAYERINFOMGR->AddPlayer(id);
					TIMESETMGR->AddPlayerTimeSet(id);
					TIMESETMGR->AddNewPlayerTimeSet(id);
					TASKMGR->AddPlayerTask(id);
					CARDMGR->AddCard(id, 110007, false);
					CARDMGR->AddCard(id, 110008, false);
					CARDMGR->AddCard(id, 110009, false);
					PRINCESSMGR->AddData(id, 200001, 0, false);
					CARDMGR->UpdateApply(id, 3);
					SERVER->GetLog()->writeLog(id, 0, "玩家注册");
				}
				BUFFMGR->LoadDB(id);
			}
		});
	}

	pHandle->RegisterDoneFunction([](int id, int error, void *pH)
	{
		U32 nPlayerID = id;
		//更新用户首次有效，增加征收令
		VIPMGR->VIPInitAddImpose(nPlayerID);
		//加载完毕
		BATTLEMGR->LoadDB(nPlayerID);
		//发送初始化物品数据
		PLAYERINFOMGR->SendInitToClient(nPlayerID);
		//发送初始化物品数据
		ITEMMGR->SendInitToClient(nPlayerID);
		//发送卡牌信息
		CARDMGR->SendInitToClient(nPlayerID);
		OFFICERMGR->SendInitToClient(nPlayerID);
		PRINCESSMGR->SendInitToClient(nPlayerID);
		SKILLMGR->SendInitToClient(nPlayerID);
		TIMESETMGR->SendInitToClient(nPlayerID);
		PVEMGR->SendInitToClient(nPlayerID);
		PRINCEMGR->SendInitToClient(nPlayerID);
		VISITMGR->SendInitToClient(nPlayerID);
		TASKMGR->SendInitToClient(nPlayerID);
		PLAYERINFOMGR->SendInitDataToClient(nPlayerID);
		DINNERMGR->SendInitToClient(nPlayerID);
		ACTIVITYMGR->SendInitToClient(nPlayerID);
		ORGMGR->SendInitToClient(nPlayerID);
		AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
		if (pAccount)
		{
			char buf[128];
			Base::BitStream sendPacket(buf, sizeof(buf));
			stPacketHead *pSendHead = IPacket::BuildPacketHead(sendPacket, "CLIENT_GAME_LoginResponse", pAccount->GetAccountId(), SERVICE_CLIENT);
			sendPacket.writeInt(0, Base::Bit16);
			sendPacket.writeInt(S32(_time32(0)), Base::Bit32);
			pSendHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
		}
		RecordLoginTime(id);
		SERVER->GetLog()->writeLog(id, 0, "玩家登陆", "", TIMESETMGR->NewPlayer(id));
	});
	pHandle->AddChild(TIMESETMGR->LoadDB(nPlayerID));
	pHandle->AddChild(ITEMMGR->LoadDB(nPlayerID));
	pHandle->AddChild(CARDMGR->LoadDB(nPlayerID));
	pHandle->AddChild(OFFICERMGR->LoadDB(nPlayerID));
	pHandle->AddChild(PRINCESSMGR->LoadDB(nPlayerID));
	pHandle->AddChild(PRINCEMGR->LoadDB(nPlayerID));
	pHandle->AddChild(SKILLMGR->LoadDB(nPlayerID));
	pHandle->AddChild(PVEMGR->LoadDB(nPlayerID));
	pHandle->AddChild(VISITMGR->LoadDB(nPlayerID));
	pHandle->AddChild(TASKMGR->LoadDB(nPlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return NULL;
}

PlayerBaseInfoRef CPlayerInfoManager::SyncLoadDB(U32 nPlayerID)
{
	PlayerBaseInfoRef pPlayer;
	{
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetId(nPlayerID);
		tHandle.SetSql(GET_SQL_UNIT(stPlayerBaseInfo).LoadSql("PlayerID", nPlayerID));
		tHandle.RegisterFunction([&](int id, int error, void* pH) {
			CDBConn* pHandle = (CDBConn*)(pH);
			if (pHandle && error == NONE_ERROR)
			{
				if (pHandle->More())
				{
					pPlayer = GetPlayerBaseInfoDB(pHandle);
				}
			}
		});
		tHandle.Commint();
	}
	return pPlayer;
}

bool CPlayerInfoManager::InsertDB(U32 nPlayerID, PlayerBaseInfoRef pData)
{
	pData->Money = 100000;
	pData->Ore = 1000;
	pData->Troops = 10000;
	pData->Attack = 2;
	pData->Hp = 10;
	pData->Land = 100000;
	pData->PLearnNum = 1;
	pData->PPlantNum = 1;
	pData->CardDevNum = 1;
	pData->PveScore = 1000;
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(GET_SQL_UNIT(stPlayerBaseInfo).InsertSql(pData));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPlayerInfoManager::UpdateDB(U32 nPlayerID, PlayerBaseInfoRef pData){
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(GET_SQL_UNIT(stPlayerBaseInfo).UpdateSql(pData));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPlayerInfoManager::DeleteDB(U32 nPlayerID)
{
	PlayerBaseInfoRef pData = PlayerBaseInfoRef(new stPlayerBaseInfo);
	pData->PlayerID = nPlayerID;
	//GET_REDIS_UNIT(stPlayerBaseInfo).DelHashRedis(pData, nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE Tbl_PlayerBaseInfo  WHERE PlayerID=%d", \
		nPlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}


bool CPlayerInfoManager::CanAddValue(U32 nPlayerID, enAddType type, S32 addVal)
{
	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer)
	{
		S32 val = 0;
		S64 val64 = 0;
		if (Business == type)
			val = (S32)(pPlayer->Business);
		else if (Military == type)
			val = (S32)(pPlayer->Military);
		else if (Technology == type)
			val = (S32)(pPlayer->Technology);
		else if (Money == type)
			val64 = (pPlayer->Money);
		else if (Ore == type)
			val64 = (pPlayer->Ore);
		else if (Troops == type)
			val64 = (pPlayer->Troops);
		else if (Attack == type)
			val = (S32)(pPlayer->Attack);
		else if (Hp == type)
			val = (S32)(pPlayer->Hp);
		else if (Land == type)
			val = (S32)(pPlayer->Land);
		else if (PveMoney == type)
			val = (S32)(pPlayer->Land);
		else if (CashGift == type)
			val = (S32)(pPlayer->CashGift);
		else if (Conquest == type)
			val = (S32)(pPlayer->Conquest);
		else
			val = (S32)(pPlayer->BattleVal);

		if (Ore == type || Money == type || Troops == type) {
			if ((val64 + (S64)addVal < 0) || (val64 + (S64)addVal > S64_MAX)) {
				return false;
			}
		}
		else if ((val + addVal < 0) || (val + addVal > S32_MAX))
		{
			return false;
		}

		return true;
	}

	return false;
}

bool CPlayerInfoManager::AddMoney(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if(pPlayer)
	{
		pPlayer->Money = mClamp((S64)pPlayer->Money + val, (S64)0, (S64)S64_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Money=%lld WHERE PlayerID=%d", (S64)pPlayer->Money, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);

		if (val < 0)
		{
			S32 nPlayerID = nPlayerId;
			SUBMIT(nPlayerID, 21, val);
			SUBMITACTIVITY(nPlayerID, ACTIVITY_MONEY, val);
		}
		return true;
	}
	else {
		pPlayer = SyncLoadDB(nPlayerId);
		if (pPlayer)
		{
			pPlayer->Money = mClamp((S64)pPlayer->Money + val, (S64)0, (S64)S64_MAX);
			TBLExecution tblHandle(SERVER->GetActorDB());
			tblHandle.SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Money=%lld WHERE PlayerID=%d", (S64)pPlayer->Money, nPlayerId));
			tblHandle.SetId(nPlayerId);
			tblHandle.Commint();
			return true;
		}
	}
	return false;
}

bool CPlayerInfoManager::AddBusiness(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if(pPlayer)
	{
		pPlayer->Business = mClamp((S32)pPlayer->Business + val, (S32)0, (S32)((pPlayer->Land > S32_MAX / 10) ? S32_MAX : pPlayer->Land * 10));
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Business=%d WHERE PlayerID=%d", (S32)pPlayer->Business, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddMilitary(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if(pPlayer)
	{
		pPlayer->Military = mClamp((S32)pPlayer->Military + val, (S32)0, (S32)((pPlayer->Land > S32_MAX / 10) ? S32_MAX : pPlayer->Land * 10));
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Military=%d WHERE PlayerID=%d", (S32)pPlayer->Military, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddTechnology(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if(pPlayer)
	{
		pPlayer->Technology = mClamp((S32)pPlayer->Technology + val, (S32)0, (S32)((pPlayer->Land > S32_MAX / 10) ? S32_MAX : pPlayer->Land * 10));
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Technology=%d WHERE PlayerID=%d", (S32)pPlayer->Technology, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddOre(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if(pPlayer)
	{
		pPlayer->Ore = mClamp((S64)pPlayer->Ore + val, (S64)0, (S64)S64_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Ore=%lld WHERE PlayerID=%d", (S64)pPlayer->Ore, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		if (val < 0)
		{
			S32 nPlayerID = nPlayerId;
			SUBMIT(nPlayerID, 22, val);
			SUBMITACTIVITY(nPlayerID, ACTIVITY_ORE, val);
		}
		return true;
	}
	else {
		pPlayer = SyncLoadDB(nPlayerId);
		if (pPlayer)
		{
			pPlayer->Ore = mClamp((S64)pPlayer->Ore + val, (S64)0, (S64)S64_MAX);
			TBLExecution tblHandle(SERVER->GetActorDB());
			tblHandle.SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Ore=%lld WHERE PlayerID=%d", (S64)pPlayer->Ore, nPlayerId));
			tblHandle.SetId(nPlayerId);
			tblHandle.Commint();
			return true;
		}
	}
	return false;
}

bool CPlayerInfoManager::AddTroops(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if(pPlayer)
	{
		pPlayer->Troops = mClamp((S64)pPlayer->Troops + val, (S64)0, (S64)S64_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Troops=%lld WHERE PlayerID=%d", (S64)pPlayer->Troops, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);

		if (val < 0)
		{
			S32 nPlayerID = nPlayerId;
			SUBMIT(nPlayerID, 23, val);
			SUBMITACTIVITY(nPlayerID, ACTIVITY_TROOP, val);
		}
		return true;
	}
	else {
		pPlayer = SyncLoadDB(nPlayerId);
		if (pPlayer)
		{
			pPlayer->Troops = mClamp((S64)pPlayer->Troops + val, (S64)0, (S64)S64_MAX);
			TBLExecution tblHandle(SERVER->GetActorDB());
			tblHandle.SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Troops=%lld WHERE PlayerID=%d", (S64)pPlayer->Troops, nPlayerId));
			tblHandle.SetId(nPlayerId);
			tblHandle.Commint();
			return true;
		}
	}

	return false;
}

bool CPlayerInfoManager::AddAttack(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if(pPlayer)
	{
		pPlayer->Attack = mClamp((S32)pPlayer->Attack + val, (S32)0, (S32)S32_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Attack=%d WHERE PlayerID=%d", (S32)pPlayer->Attack, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddHp(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if (pPlayer)
	{
		pPlayer->Hp = mClamp((S32)pPlayer->Hp + val, (S32)0, (S32)S32_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Hp=%d WHERE PlayerID=%d", (S32)pPlayer->Hp, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddLand(U32 nPlayerID, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer)
	{
		TOPMGR->NewInDataEx(eTopType_LandUp, nPlayerID, pPlayer->Land, mClamp((S32)pPlayer->Land + val, (S32)0, (S32)S32_MAX / 10));
		pPlayer->Land = mClamp((S32)pPlayer->Land + val, (S32)0, (S32)S32_MAX / 10);
		TOPMGR->NewInData(eTopType_Land, nPlayerID, pPlayer->Land);
		CaculateBattleVal(nPlayerID);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Land=%d WHERE PlayerID=%d", (S32)pPlayer->Land, nPlayerID));
		pHandle->SetId(nPlayerID);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerID, pPlayer);
		U32 nLand = pPlayer->Land;
		SUBMIT(nPlayerID, 18, nLand);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddBattleVal(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if(pPlayer)
	{
		pPlayer->BattleVal = mClamp((S32)pPlayer->BattleVal + val, (S32)0, (S32)S32_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET BattleVal=%d WHERE PlayerID=%d", (S32)pPlayer->BattleVal, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddPLearnNum(U32 nPlayerID, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer)
	{
		pPlayer->PLearnNum = mClamp((S32)pPlayer->PLearnNum + val, (S32)0, (S32)S8_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET PLearnNum=%d WHERE PlayerID=%d", (S8)pPlayer->PLearnNum, nPlayerID));
		pHandle->SetId(nPlayerID);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerID, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddPPlantNum(U32 nPlayerID, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer)
	{
		pPlayer->PPlantNum = mClamp((S32)pPlayer->PPlantNum + val, (S32)0, (S32)S8_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET PPlantNum=%d WHERE PlayerID=%d", (S8)pPlayer->PPlantNum, nPlayerID));
		pHandle->SetId(nPlayerID);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerID, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddCardDevNum(U32 nPlayerID, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer)
	{
		pPlayer->CardDevNum = mClamp((S32)pPlayer->CardDevNum + val, (S32)0, (S32)S8_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET CardDevNum=%d WHERE PlayerID=%d", (S8)pPlayer->CardDevNum, nPlayerID));
		pHandle->SetId(nPlayerID);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerID, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddPveScore(U32 nPlayerID, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer)
	{
		pPlayer->PveScore = mClamp((S32)pPlayer->PveScore + val, (S32)0, (S32)S32_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET PveScore=%d WHERE PlayerID=%d", (S32)pPlayer->PveScore, nPlayerID));
		pHandle->SetId(nPlayerID);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerID, pPlayer);
		SUBMIT(nPlayerID, 47, pPlayer->PveScore);
		return true;
	}
	else {
		pPlayer = SyncLoadDB(nPlayerID);
		if (pPlayer)
		{
			pPlayer->PveScore = mClamp((S32)pPlayer->PveScore + val, (S32)0, (S32)S32_MAX);
			TBLExecution tblHandle(SERVER->GetActorDB());
			tblHandle.SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET PveScore=%d WHERE PlayerID=%d", (S32)pPlayer->PveScore, nPlayerID));
			tblHandle.SetId(nPlayerID);
			tblHandle.Commint();
			return true;
		}
	}

	return false;
}

bool CPlayerInfoManager::AddPveMoney(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if (pPlayer)
	{
		pPlayer->PveMoney = mClamp((S32)pPlayer->PveMoney + val, (S32)0, (S32)S32_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET PveMoney=%d WHERE PlayerID=%d", (S32)pPlayer->PveMoney, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddCashGift(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if (pPlayer)
	{
		pPlayer->CashGift = mClamp((S32)pPlayer->CashGift + val, (S32)0, (S32)S32_MAX);
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET CashGift=%d WHERE PlayerID=%d", (S32)pPlayer->CashGift, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		return true;
	}
	else {
		pPlayer = SyncLoadDB(nPlayerId);
		if (pPlayer)
		{
			pPlayer->CashGift = mClamp((S32)pPlayer->CashGift + val, (S32)0, (S32)S32_MAX);
			TBLExecution tblHandle(SERVER->GetActorDB());
			tblHandle.SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET CashGift=%d WHERE PlayerID=%d", (S32)pPlayer->CashGift, nPlayerId));
			tblHandle.SetId(nPlayerId);
			tblHandle.Commint();
			return true;
		}
	}

	return false;
}

bool CPlayerInfoManager::AddConquest(U32 nPlayerId, S32 val)
{
	if (val == 0)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if (pPlayer)
	{
		auto pStats = BUFFMGR->GetData(nPlayerId);
		pPlayer->Conquest = mClamp(((S32)pPlayer->Conquest + val) * (1.0f + (pStats ? (pStats->OrgConquest_gPc / 100.0f) : (0.0f))), (S32)0, (S32)(pPlayer->Land * 0.05));
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET Conquest=%d WHERE PlayerID=%d", (S32)pPlayer->Conquest, nPlayerId));
		pHandle->SetId(nPlayerId);
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
		UpdateToClient(nPlayerId, pPlayer);
		return true;
	}
	return false;
}

bool CPlayerInfoManager::AddImposeTimes(U32 nPlayerID, S32 val)
{
	if (val == 0)
		return false;

	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, IMPOSE_TIMER1);
	if (pTime)
	{
		S32 nTimes = mClamp((S32)(pTime->Flag1 + val), (S32)0, (S32)S32_MAX);
		TIMESETMGR->AddTimeSet(nPlayerID, IMPOSE_TIMER1, 0, "", nTimes);
	}
	return true;
}
S32  CPlayerInfoManager::GetImposeTimes(U32 nPlayerID)
{
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, IMPOSE_TIMER1);
	if (pTime)
	{
		return pTime->Flag1;
	}
	return 10;
}

bool CPlayerInfoManager::SetFace(U32 nPlayerID, S32 val)
{	
	TimeSetRef pTimer = TIMESETMGR->GetData(nPlayerID, FACE_SET_TIMER);
	if (!pTimer) 
	{
		if (SERVER->GetPlayerManager()->SetFace(nPlayerID, val))
		{
			TIMESETMGR->AddTimeSet(nPlayerID, FACE_SET_TIMER, 24 * 60 * 60, "", 0);
			return true;
		}
	}

	return false;
}

bool CPlayerInfoManager::SetPlayerFlag(U32 nPlayerID, S32 nSlot, S32& nFlag)
{
	if (nSlot < 0 || nSlot > 31)
		return false;

	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer) 
	{
		//任务未完成
		if (!(pPlayer->PlayerFlag & BIT(nSlot))) {
			pPlayer->PlayerFlag |= BIT(nSlot);
			DB_Execution* pHandle = new DB_Execution();
			pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET PlayerFlag=%d WHERE PlayerID=%d", (S32)pPlayer->PlayerFlag, nPlayerID));
			pHandle->SetId(nPlayerID);
			SERVER->GetPlayerDBManager()->SendPacket(pHandle);
			return true;
		}
		nFlag = pPlayer->PlayerFlag;
	}
	return false;
}

void CPlayerInfoManager::SetPlayerOrg(U32 nPlayerID, S32 OrgId) {
	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer)
	{
		pPlayer->OrgId = OrgId;
	}
	
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET OrgId=%d WHERE PlayerID=%d", OrgId, nPlayerID));
	pHandle->SetId(nPlayerID);
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
}

void CPlayerInfoManager::NewInCostActivity(S32 nPlayerID, enActivityType Type, S32 nVal) {
	MAKE_WORKQUEUE_PACKET(sendPacket, 256);
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WW_CostACTIVITY_Notify", nPlayerID, 0, abs(nVal), Type);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SEND_WORKQUEUE_PACKET(sendPacket, WQ_NORMAL, OWQ_EnterGame);
}

void CPlayerInfoManager::NotifyTimeDel(U32 nPlayerID, TimeSetRef pTime)
{
	if (!pTime)
		return;

	auto nNextDay = Platform::getNextTime(Platform::INTERVAL_DAY);
	nNextDay -= _time32(NULL);
	auto CurTime = (S32)_time32(NULL);

	if (pTime->ID == TECHNOLOGY_TIMER || pTime->ID == TECHNOLOGY_TIMER1 || pTime->ID == TECHNOLOGY_TIMER2)
		SKILLMGR->DoLevelUp(nPlayerID, pTime->Flag1);
	else if (pTime->ID == NEXTTENMINUTES_TIMER)
	{
		S32 Times = 1;
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, REFLASH_TIME);
		if (pTime && (0 != pTime->Flag1)) {
			Times = (CurTime - pTime->Flag1) / (10 * 60);
		}

		for (int i = 0; i < Times; ++i) {
			BATTLEMGR->ReflashTime(nPlayerID);
		}
		TIMESETMGR->AddTimeSet(nPlayerID, REFLASH_TIME, 0, "", CurTime);
		TIMESETMGR->AddTimeSet(nPlayerID, NEXTTENMINUTES_TIMER, 10 * 60, "", CurTime);
		
	}
	else if (pTime->ID == NEXTHOUR_TIMER)
	{
		S32 nRecover = GetRecoverTimes(CurTime, pTime->ExpireTime, 30 * 60);
		nRecover = mClamp(nRecover, 0, 3);
		TIMESETMGR->AddTimeSet(nPlayerID, NEXTHOUR_TIMER, 60 * 60, "", 0);
	}
	else if (pTime->ID == NEXTHHOUR_TIMER)
	{
		S32 nMaxTimes = GetRecoverTimes(CurTime, pTime->ExpireTime, 30 * 60);
		CARDMGR->UpdateApply(nPlayerID, nMaxTimes);
		TIMESETMGR->AddTimeSet(nPlayerID, NEXTHHOUR_TIMER, 30 * 60, "", 0);
	}
	else if (pTime->ID == CITY_TIMER2)
	{
		S32 nRecover = GetRecoverTimes(CurTime, pTime->ExpireTime, VISITCITYCD * (1.f + VIPMGR->GetVipVSAReducePercent(nPlayerID)/100.f));
		if (pTime->Flag1 < 5 + VIPMGR->GetVipStrengthTopLimit(nPlayerID)) {
			if (pTime->Flag1 + nRecover < 5 + VIPMGR->GetVipStrengthTopLimit(nPlayerID))
				pTime->Flag1 += nRecover;
			else
				pTime->Flag1 = 5 + VIPMGR->GetVipStrengthTopLimit(nPlayerID);
		}

		TIMESETMGR->AddTimeSet(nPlayerID, CITY_TIMER2, VISITCITYCD * (1.f + VIPMGR->GetVipVSAReducePercent(nPlayerID)/100.f), "", pTime->Flag1);
	}
	else if (pTime->ID == IMPOSE_TIMER)
	{
		PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
		auto nVal = (pPlayer ? (pPlayer->BattleVal - pPlayer->Land) : 1) * 1.0f;
		S32 nTime = (nVal / (nVal + 1000000)) * 1800 + 60;
		nTime = mClamp(nTime, 60, 1800 + 60);

		S32 nMaxTimes = GetImposeTimes(nPlayerID);
		S32 nRecover = GetRecoverTimes(CurTime, pTime->ExpireTime, nTime);
		if (pTime->Flag1 < nMaxTimes) {
			if (pTime->Flag1 + nRecover < nMaxTimes)
				pTime->Flag1 += nRecover;
			else
				pTime->Flag1 = nMaxTimes;
		}

		TIMESETMGR->AddTimeSet(nPlayerID, IMPOSE_TIMER, nTime, "", pTime->Flag1);
	}
	/*else if (pTime->ID >= CARD_DEVELOP_TIMER1 &&  pTime->ID <= CARD_DEVELOP_TIMER6)
	{
		CARDMGR->DispatchCardAward(nPlayerID, pTime->Flag, pTime->Flag1);
	}*/
	else if (pTime->ID == NEXTDAY_TIMER)
	{
		PRINCESSMGR->UpdatePrincess(nPlayerID);
		TIMESETMGR->AddTimeSet(nPlayerID, NEXTDAY_TIMER, nNextDay, "", 0);
		TASKMGR->RemoveDailyTask(nPlayerID);

		S32 nRecover = GetRecoverTimes(CurTime, pTime->ExpireTime, 24 * 60 * 60);
		PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
		if (pPlayer) {
			auto nAddVal = pPlayer->Land * 0.01;
			AddConquest(nPlayerID, nRecover * nAddVal);
		}
	}
	else if (pTime->ID == TURN_CARD_TIMER)
	{
		S32 nRecover = GetRecoverTimes(CurTime, pTime->ExpireTime, TRUNCARDCD * ((100.f + VIPMGR->GetVipVSAReducePercent(nPlayerID)) / 100.f));
		if (pTime->Flag1 < 5 + VIPMGR->GetVipVIMTopLimit(nPlayerID)) {
			if (pTime->Flag1 + nRecover < 5 + VIPMGR->GetVipVIMTopLimit(nPlayerID))
				pTime->Flag1 += nRecover;
			else
				pTime->Flag1 = 5 + VIPMGR->GetVipVIMTopLimit(nPlayerID);
		}

		TIMESETMGR->AddTimeSet(nPlayerID, TURN_CARD_TIMER, TRUNCARDCD * ((100.f + VIPMGR->GetVipVSAReducePercent(nPlayerID)) / 100.f), "", pTime->Flag1);
	}
}

void CPlayerInfoManager::TimeProcess(U32 bExit)
{
	auto nCurTime = _time32(NULL);
	//for (auto itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
	{
		////每小时产量
		//{
		//	TimeSetRef pTime = TIMESETMGR->GetData(itr->first, IMPOSE_TIMER);
		//	if (pTime)
		//	{
		//		S32 nTime = atoi(pTime->Flag.c_str());
		//		if (nCurTime >= nTime)
		//		{
		//			if(pTime->Flag1 < 10)
		//				++pTime->Flag1;

		//			pTime->Flag = fmt::sprintf("%d", (_time32(NULL) + 30 * 60));
		//			TIMESETMGR->UpdateDB(pTime);
		//		}
		//	}
		//}

		//奏章
	/*	{
			TimeSetRef pTime = TIMESETMGR->GetData(itr->first, APPLY_TIMER);
			if (pTime && nCurTime >= pTime->Flag1)
			{
				CARDMGR->UpdateApply(itr->first);
			}
		}*/

		//隔天
		//{
		//	TimeSetRef pTime = TIMESETMGR->GetData(itr->first, NEXTDAY_TIMER);
		//	if (pTime && nCurTime >= pTime->Flag1)
		//	{
		//		auto nNextDay = Platform::getNextTime(Platform::INTERVAL_DAY);
		//		PRINCESSMGR->UpdatePrincess(itr->first);
		//		pTime->Flag1 = nNextDay;
		//		TIMESETMGR->UpdateDB(pTime);
		//	}
		//}

		//每隔小时
		//{
		//	TimeSetRef pTime = TIMESETMGR->GetData(itr->first, NEXTHOUR_TIMER);
		//	if (pTime && nCurTime >= pTime->Flag1)
		//	{
		//		PRINCEMGR->UpdatePrince(itr->first);
		//		TIMESETMGR->AddTimeSet(itr->first, NEXTHOUR_TIMER, 0, "", (_time32(NULL) + 60 * 60));
		//	}
		//}

		//翻拍
	/*	{
			TimeSetRef pTime = TIMESETMGR->GetData(itr->first, TURN_CARD_TIMER);
			if (pTime)
			{
				S32 nTime = atoi(pTime->Flag.c_str());
				if (nCurTime >= nTime)
				{
					if (pTime->Flag1 < 5)
						pTime->Flag1 = 5;

					pTime->Flag = fmt::sprintf("%d", Platform::getNextTime(Platform::INTERVAL_DAY));
					TIMESETMGR->UpdateDB(pTime);
				}
			}
		}*/

		////寻访
		//{
		//	TimeSetRef pTime = TIMESETMGR->GetData(itr->first, CITY_TIMER2);
		//	if (pTime)
		//	{
		//		S32 nTime = atoi(pTime->Flag.c_str());
		//		if (nCurTime >= nTime)
		//		{
		//			if (pTime->Flag1 < 5)
		//				++pTime->Flag1;

		//			pTime->Flag = fmt::sprintf("%d", (_time32(NULL) + 60 * 60));
		//			TIMESETMGR->UpdateDB(pTime);
		//		}
		//	}
		//}
	}
}

void CPlayerInfoManager::AddBuff(U32 nPlayerID, S32 pos, stBuff& buff, bool bNotity/*=false*/)
{
	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer)
	{
		pPlayer->getBuff(pos) += buff;
		CaculateBattleVal(nPlayerID);
		if(!bNotity)
			UpdateToClient(nPlayerID);
		S32 nBusniess = pPlayer->Business + pPlayer->getBuff(0).Business + pPlayer->getBuff(1).Business + pPlayer->getBuff(2).Business + \
			pPlayer->getBuff(3).Business;
		S32 nMilitary = pPlayer->Military + pPlayer->getBuff(0).Military + pPlayer->getBuff(1).Military + pPlayer->getBuff(2).Military + \
			pPlayer->getBuff(3).Military;
		S32 nTechnology = pPlayer->Technology + pPlayer->getBuff(0).Technology + pPlayer->getBuff(1).Technology + pPlayer->getBuff(2).Technology + \
			pPlayer->getBuff(3).Technology;
		SUBMIT(nPlayerID, 15, nBusniess);
		SUBMIT(nPlayerID, 16, nTechnology);
		SUBMIT(nPlayerID, 17, nMilitary);
	}
}

bool CPlayerInfoManager::AddQueenVal(U32 nPlayerId, S32 val, bool bSubmit/*=false*/)
{
	PlayerBaseInfoRef pPlayer = GetData(nPlayerId);
	if (pPlayer)
	{
		TimeSetRef pTimer = TIMESETMGR->GetData(nPlayerId, TOP_UP_PRINCESS);
		if (!pTimer) {
			pTimer = TIMESETMGR->AddTimeSet(nPlayerId, TOP_UP_PRINCESS, 0, "", pPlayer->QueenVal);
		}

		pPlayer->QueenVal = mClamp((S32)pPlayer->QueenVal + val, (S32)0, (S32)S32_MAX);
		if (pTimer) {
			if (pPlayer->QueenVal > pTimer->Flag1) {
				TOPMGR->NewInDataEx(eTopType_PrincessUp, nPlayerId, pTimer->Flag1, pPlayer->QueenVal);
				TIMESETMGR->AddTimeSet(nPlayerId, TOP_UP_PRINCESS, 0, "", pPlayer->QueenVal);
			}
		}
		CaculateBattleVal(nPlayerId);

		if (!bSubmit) {
			SUBMIT(nPlayerId, 19, pPlayer->QueenVal);
		}
		return true;
	}
	return false;
}

bool CPlayerInfoManager::CaculateBattleVal(U32 nPlayerID)
{
	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	if (pPlayer)
	{
		S32 nBattleVal = 0;
		for (int i = 0; i < 4; ++i)
		{
			nBattleVal = mClamp((S32)(nBattleVal + pPlayer->Buff[i].Business + pPlayer->Buff[i].Military + pPlayer->Buff[i].Technology), (S32)0, (S32)S32_MAX);
		}
		
		nBattleVal = mClamp((S32)(nBattleVal + pPlayer->Business),	(S32)0, (S32)S32_MAX);
		nBattleVal = mClamp((S32)(nBattleVal + pPlayer->Military),	(S32)0, (S32)S32_MAX);
		nBattleVal = mClamp((S32)(nBattleVal + pPlayer->Technology), (S32)0, (S32)S32_MAX);
		nBattleVal = mClamp((S32)(nBattleVal + pPlayer->QueenVal),	(S32)0, (S32)S32_MAX);
		nBattleVal = mClamp((S32)(nBattleVal + pPlayer->Land),		(S32)0, (S32)S32_MAX);

		if (nBattleVal > pPlayer->BattleVal)
		{
			TOPMGR->NewInDataEx(eTopType_BattleUp, nPlayerID, pPlayer->BattleVal, nBattleVal);
			pPlayer->BattleVal = nBattleVal;
			DB_Execution* pHandle = new DB_Execution();
			//GET_REDIS_UNIT(stPlayerBaseInfo).SetHashRedis(pPlayer, nPlayerID);
			pHandle->SetSql(fmt::sprintf("UPDATE Tbl_PlayerBaseInfo SET BattleVal=%d WHERE PlayerID=%d", (S32)pPlayer->BattleVal, nPlayerID));
			pHandle->SetId(nPlayerID);
			SERVER->GetPlayerDBManager()->SendPacket(pHandle);
			UpdateToClient(nPlayerID, pPlayer);
			SUBMIT(nPlayerID, 14, nBattleVal);
		}
		TOPMGR->NewInData(eTopType_Battle, nPlayerID, nBattleVal);

		return true;
	}
	return false;
}

void CPlayerInfoManager::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	PlayerBaseInfoRef pPlayer = GetData(nPlayerID);
	PlayerSimpleDataRef spd = SERVER->GetPlayerManager()->GetPlayerData(nPlayerID);
	if (pAccount && pPlayer && spd)
	{
		auto pStats = BUFFMGR->GetData(nPlayerID);
		pPlayer->Gold = spd->Gold;
		pPlayer->DrawGold = spd->DrawGold;
		pPlayer->AttackEx = pPlayer->Attack + (pStats ? pStats->SolAtk : 0);
		pPlayer->HpEx	  = pPlayer->Hp + (pStats ? pStats->SolHp : 0);
		CMemGuard Buffer(4096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 4096);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_PLAYERINFO_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pPlayer->WriteData(&sendPacket);
		sendPacket.writeInt(spd->Family, Base::Bit32);
		sendPacket.writeInt(spd->FirstClass, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

PlayerBaseInfoRef CPlayerInfoManager::GetPlayer(U32 nPlayerID)
{
	PlayerBaseInfoRef pData = GetData(nPlayerID);
	if (!pData)
	{
		pData = SyncLoadDB(nPlayerID);
	}

	return pData;
}

void CPlayerInfoManager::SendInitDataToClient(U32 nPlayerID)
{
#define MAX_DISPATCH_NUM 7
	U32 nError = 0;
	CMemGuard Buffer(4096 MEM_GUARD_PARAM);
	Base::BitStream sendPacket(Buffer.get(), 4096);
	PlayerBaseInfoRef pData = GetData(nPlayerID);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount && pData)
	{
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_DATA_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(PVEMGR->GetBattleType(nPlayerID), Base::Bit8);
		sendPacket.writeInt(pData->PlayerFlag, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

}

void CPlayerInfoManager::UpdateToClient(U32 nPlayerID, PlayerBaseInfoRef pPlayer)
{
	S32 nCurTime = time(NULL);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	PlayerSimpleDataRef spd = SERVER->GetPlayerManager()->GetPlayerData(nPlayerID);
	if (pAccount && pPlayer && spd)
	{
		auto pStats = BUFFMGR->GetData(nPlayerID);
		pPlayer->Gold = spd->Gold;
		pPlayer->VipLv = spd->VipLv;
		pPlayer->DrawGold = spd->DrawGold;
		pPlayer->AttackEx = pPlayer->Attack + (pStats ? pStats->SolAtk : 0);
		pPlayer->HpEx = pPlayer->Hp + (pStats ? pStats->SolHp : 0);
		CMemGuard Buffer(4096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 4096);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_PLAYERINFO_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pPlayer->WriteData(&sendPacket);
		sendPacket.writeInt(spd->Family, Base::Bit32);
		sendPacket.writeInt(spd->FirstClass, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CPlayerInfoManager::UpdateToClient(U32 nPlayerID)
{
	PlayerBaseInfoRef pPlayer = GetPlayer(nPlayerID);
	UpdateToClient(nPlayerID, pPlayer);
}

bool CPlayerInfoManager::onPlayerUpdateClient(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nPlayerID = pHead->Id;
	int Gold  = Packet->readInt(Base::Bit32);
	enOpType Type = (enOpType)Packet->readInt(Base::Bit32);
	PlayerBaseInfoRef pData = GetData(nPlayerID);
	if (pData)
		UpdateToClient(nPlayerID, pData);

	if (Gold < 0)
	{
		SERVER->GetLog()->writeLog(nPlayerID, 0, "玩家消耗元宝", fmt::sprintf("%d", Gold), GETLOGSTR(Type));
		SUBMIT(nPlayerID, 20, abs(Gold));
		SUBMITACTIVITY(nPlayerID, ACTIVITY_GOLD, Gold);
	}
	return true;
}

bool CPlayerInfoManager::onPlayerEnter(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 nPlayerID = pHead->Id;
	LoadDB(nPlayerID);
	return true;
}

bool CPlayerInfoManager::onPlayerLeave(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nPlayerID = pHead->Id;
	SERVER->GetLog()->writeLog(nPlayerID, 0, "玩家退出",fmt::sprintf("%d", RecordLoginTime(nPlayerID)), TIMESETMGR->NewPlayer(nPlayerID));
	RemovePlayer(nPlayerID);
	ITEMMGR->RemovePlayer(nPlayerID);
	CARDMGR->RemovePlayer(nPlayerID);
	TIMESETMGR->RemovePlayer(nPlayerID);
	BATTLEMGR->RemovePlayer(nPlayerID);
	OFFICERMGR->RemovePlayer(nPlayerID);
	BUFFMGR->RemovePlayer(nPlayerID);
	VISITMGR->RemovePlayer(nPlayerID);
	PVEMGR->RemovePlayer(nPlayerID);
	PRINCEMGR->RemovePlayer(nPlayerID);
	PRINCESSMGR->RemovePlayer(nPlayerID);
	SKILLMGR->RemovePlayer(nPlayerID);
	TASKMGR->RemovePlayer(nPlayerID);
	DINNERMGR->LeaveDinner(nPlayerID);
	return true;
}

enOpError CPlayerInfoManager::Impose(U32 nPlayerID, U32 nType, U32& nAddVal)
{
	S32 iAddVal = 0;
	if (nType < 0 || nType > 3)
		return OPERATOR_PARAMETER_ERROR;

	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pPlayer)
		return OPERATOR_HASNO_PLAYER;
	
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, IMPOSE_TIMER);
	if (!pTime || pTime->Flag1 <= 0)
		return OPERATOR_HASNO_IMPOSE_ITEM;

	{
		StatsRef pStats = BUFFMGR->GetData(nPlayerID);
		F32 iBaseVal = 0;
		if(nType == 0)
			iBaseVal = mFloor((pPlayer->Business + pPlayer->getBuff(0).Business + pPlayer->getBuff(1).Business + pPlayer->getBuff(2).Business + \
			pPlayer->getBuff(3).Business + pPlayer->Land) * (1.0f + VIPMGR->GetResIncreasePercent(nPlayerID) / 100.f + (pStats ? (pStats->AddMoney_gPc + pStats->AddRes_gPc + pStats->OrgImposeMoney_gPc) : 0) / 100.0f));
		else if(nType == 1)
			iBaseVal = mFloor((pPlayer->Technology + pPlayer->getBuff(0).Technology + pPlayer->getBuff(1).Technology + pPlayer->getBuff(2).Technology + \
			pPlayer->getBuff(3).Technology + pPlayer->Land)*(1.0f + VIPMGR->GetResIncreasePercent(nPlayerID) / 100.f + (pStats ? (pStats->AddOre_gPc + pStats->AddRes_gPc + pStats->OrgOre_gPc) : 0) / 100.0f));
		else
			iBaseVal = mFloor((pPlayer->Military + pPlayer->getBuff(0).Military + pPlayer->getBuff(1).Military + pPlayer->getBuff(2).Military + \
			pPlayer->getBuff(3).Military + pPlayer->Land) * (1.0f + VIPMGR->GetResIncreasePercent(nPlayerID) / 100.f + (pStats ? (pStats->CallTroop_gPc + pStats->AddRes_gPc + pStats->OrgTroop_gPc) : 0) / 100.0f)) * 10;

		nAddVal = iAddVal = mFloor(iBaseVal * ((nType == 0) ? (1.0f) : 0.01f));
		if (iAddVal == 0)
			OPERATOR_IMPOSE_ERROR;

		if (nType == 0)
		{
			PLAYERINFOMGR->AddMoney(nPlayerID, iAddVal);
			SetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 0, 1);
			SUBMIT(nPlayerID, 48, 1);
		}
		else if (nType == 1)
		{
			PLAYERINFOMGR->AddOre(nPlayerID, iAddVal);
			SetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 1, 1);
			SUBMIT(nPlayerID, 49, 1);
		}
		else
		{
			PLAYERINFOMGR->AddTroops(nPlayerID, iAddVal);
			SetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 2, 1);
			SUBMIT(nPlayerID, 50, 1);
		}

		SUBMIT(nPlayerID, 3, 1);
		SUBMITACTIVITY(nPlayerID, ACTIVITY_IMPOSE, 1);

		S32 iTime = Platform::getNextTime(Platform::INTERVAL_DAY);
		TIMESETMGR->AddTimeSet(nPlayerID, IMPOSE_TIMER, -1, "", --pTime->Flag1);
		return OPERATOR_NONE_ERROR;
	}

	return OPERATOR_IMPOSE_ERROR;
}

bool CPlayerInfoManager::HandleImposeRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nAddVal = 0;
		U32 nError = Impose(pAccount->GetPlayerId(), nType, nAddVal);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_IMPOSE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nType, Base::Bit8);
		sendPacket.writeInt(nAddVal, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPlayerInfoManager::HandleSetFaceRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nVal = Packet->readInt(Base::Bit32);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = 0;
		if (!SetFace(pAccount->GetPlayerId(), nVal))
		{
			nError = OPERATOR_SET_FACE_COOLDOWN;
		}
		else 
		{
			UpdateToClient(pAccount->GetPlayerId());
		}

		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_SETFACE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPlayerInfoManager::HandleSetPlayerFlagRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	U32 nSlot = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		S32 nError = 0, nPlayerFlag = 0;
		if (!SetPlayerFlag(pAccount->GetPlayerId(), nSlot, nPlayerFlag))
		{
			nError = OPERATOR_PARAMETER_ERROR;
		}
		else
		{
			//SendInitDataToClient(pAccount->GetPlayerId());
		}

		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_SETPLAYERFLAG_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nPlayerFlag, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

void ___AddMoney(int id, int gold)
{
	PLAYERINFOMGR->AddMoney(id, gold);
}

void ___AddGold(int id, int gold)
{
	SERVER->GetPlayerManager()->AddGold(id, gold);
}

void ___GetPlayer(int id)
{
	PLAYERINFOMGR->GetPlayer(id);
}

void ___KickAccount(int id)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(id);
	if (pAccount) {
		pAccount->Kick();
	}
}

void CPlayerInfoManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "AddGold", &___AddGold);
	lua_tinker::def(L, "AddMoney", &___AddMoney);
	lua_tinker::def(L, "GetPlayer", &___GetPlayer);
	lua_tinker::def(L, "KickAccount", &___KickAccount);
	CARDMGR->Export(L);
	ITEMMGR->Export(L);
	TIMESETMGR->Export(L);
	TASKMGR->Export(L);
	BATTLEMGR->Export(L);
	OFFICERMGR->Export(L);
	PRINCESSMGR->Export(L);
	PRINCEMGR->Export(L);
	PVEMGR->Export(L);
	MAILMGR->Export(L);
	DINNERMGR->Export(L);
	ACTIVITYMGR->Export(L);
	ORGMGR->Export(L);
	VIPMGR->Export(L);
	TOPMGR->Export(L);
}