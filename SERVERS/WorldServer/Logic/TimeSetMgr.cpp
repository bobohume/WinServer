#include "TimeSetMgr.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "PlayerInfoMgr.h"
#include "../WorldServer.h"
#include "../PlayerMgr.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "Common/MemGuard.h"
#include "WINTCP/dtServerSocket.h"
#include "platform.h"
#include "VIPMgr.h"
#include "NoviceGuide.h"
#include "ActivityManager.h"
#include "PrincessMgr.h"
#include "VisitlMgr.h"
#include "TaskMgr.h"

const unsigned int MAX_TIMESET_SLOT = 500;
//DECLARE_REDIS_UNIT(stTimeSet);
CTimeSetManager::CTimeSetManager()
{
	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CTimeSetManager::TimeProcess, 1 * 1000);
	/*GET_REDIS_UNIT(stTimeSet).SetName("Tbl_TimeSet");
	REGISTER_REDIS_UNITKEY(stTimeSet, PlayerID);
	REGISTER_REDIS_UNITKEY(stTimeSet, ID);
	REGISTER_REDIS_UNIT(stTimeSet, Flag);
	REGISTER_REDIS_UNIT(stTimeSet, Flag1);
	REGISTER_REDIS_UNIT(stTimeSet, ExpireTime);*/
	REGISTER_EVENT_METHOD("CW_ADVANCETIME_REQUEST", this, &CTimeSetManager::HandleAdvanceTimeRequest);
}

CTimeSetManager::~CTimeSetManager()
{
	m_PlayerSortMap.clear();
}

CTimeSetManager* CTimeSetManager::Instance()
{
	static CTimeSetManager s_Mgr;
	return &s_Mgr;
}

TimeSetRef CTimeSetManager::CreateTimeSet(U32 ID, U32 nPayerID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		TimeSetRef pTime = TimeSetRef(new stTimeSet);
		pTime->ID = ID;
		pTime->PlayerID = nPayerID;
		return pTime;
	}
	DBECATCH()

	return NULL;
}

TimeSetRef CTimeSetManager::GetData(U32 nPlayerID, U32 ID)
{
	PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		DATA_MAP& DataMap = itr->second;
		DATA_ITR iter = DataMap.find(ID);
		if (iter != DataMap.end())
		{
			return iter->second;
		}
	}
	else {
		return SyncLoadDB(nPlayerID, ID);
	}

	return TimeSetRef();
}

TimeSetRef CTimeSetManager::AddTimeSet(U32 nPlayerID, U32 nID, S32 nExpireTime, std::string strFlag, S32 Flag1/*= 0*/)
{
	bool bOnline = false;
	if (PLAYERINFOMGR->GetData(nPlayerID)) {
		bOnline = true;
	}

	TimeSetRef pTime = GetData(nPlayerID, nID);
	if (pTime)
	{
		S32 curTime = (S32)_time32(0);
		if (bOnline)
			m_PlayerSortMap[nPlayerID].erase(pTime);
		if (nExpireTime == 0)
			pTime->ExpireTime = S32_MAX;//永久
		else if (nExpireTime != 0 && nExpireTime != -1)
			pTime->ExpireTime = curTime + nExpireTime;

		pTime->Flag = strFlag;
		pTime->Flag1 = Flag1;
		if(bOnline)
			m_PlayerSortMap[nPlayerID].insert(TIME_SORT_SET::value_type(pTime));
		UpdateDB(pTime);
		UpdateToClient(nPlayerID, pTime);
	}
	else
	{
		pTime = CreateTimeSet(nID, nPlayerID);
		if (!pTime)
			return TimeSetRef();

		S32 curTime = (S32)_time32(0);
		if (nExpireTime == 0)
			pTime->ExpireTime = S32_MAX;//永久
		else
			pTime->ExpireTime = curTime + nExpireTime;

		pTime->Flag = strFlag;
		pTime->Flag1 = Flag1;
		if (bOnline)
			_AddTimeSet(nPlayerID, pTime);
		InsertDB(nPlayerID, pTime);
		UpdateToClient(nPlayerID, pTime);
	}

	return pTime;
}

void CTimeSetManager::_AddTimeSet(U32 nPlayerID, TimeSetRef pTime)
{
	if (pTime)
	{
		m_PlayerMap[nPlayerID].insert(DATA_MAP::value_type(pTime->ID, pTime));
		m_PlayerSortMap[nPlayerID].insert(TIME_SORT_SET::value_type(pTime));
	}
}

bool CTimeSetManager::RemoveTimeSet(U32 nPlayerID, U32 nID)
{
	_RemoveTimeSet(nPlayerID, nID);
	DeleteDB(nPlayerID, nID);
	return true;
}

void CTimeSetManager::_RemoveTimeSet(U32 nPlayerID, U32 nID)
{
	PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		DATA_MAP& TimeMap = itr->second;
		DATA_ITR iter = TimeMap.find(nID);
		if (iter != TimeMap.end())
		{
			m_PlayerSortMap[nPlayerID].erase(iter->second);
			TimeMap.erase(iter);
		}
	}
}

void CTimeSetManager::RemovePlayer(U32 nPlayerID)
{
	PLAYER_ITR  itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		DATA_MAP& PlayerMap = itr->second;
		for (DATA_ITR iter = PlayerMap.begin(); iter != PlayerMap.end(); ++iter)
		{
			m_PlayerSortMap[nPlayerID].erase(iter->second);
		}
		PlayerMap.clear();
		m_PlayerMap.erase(itr);
		m_PlayerSortMap.erase(nPlayerID);
	}
}

void CTimeSetManager::NotifyTimeDel(U32 nPlayerID, TimeSetRef pTime)
{
	PLAYERINFOMGR->NotifyTimeDel(nPlayerID, pTime);
}

enOpError CTimeSetManager::AdvanceTime(U32 nPlayerID, S32 ID)
{
	if (ID != DISPATCH_PRINCESS_TIMER1 && ID != DISPATCH_PRINCESS_TIMER2 && ID != DISPATCH_PRINCESS_TIMER3 && ID != DISPATCH_PRINCESS_TIMER4 && \
		ID != LEARN_RITE_TIMER2 && ID != LEARN_RITE_TIMER3 && ID != LEARN_RITE_TIMER4 && ID != LEARN_RITE_TIMER5 && \
		ID != PLANT_FLOWER_TIMER1 && ID != PLANT_FLOWER_TIMER2 && ID != PLANT_FLOWER_TIMER3 && ID != PLANT_FLOWER_TIMER4 && \
		ID != TECHNOLOGY_TIMER && ID != TECHNOLOGY_TIMER1 && ID != TECHNOLOGY_TIMER2)
		return OPERATOR_PARAMETER_ERROR;

	TimeSetRef pData = TIMESETMGR->GetData(nPlayerID, ID);
	if (!pData)
		return OPERATOR_PARAMETER_ERROR;

	S32 nTime = 0;
	bool bFlag1 = (ID == DISPATCH_PRINCESS_TIMER1 || ID == DISPATCH_PRINCESS_TIMER2 || ID == DISPATCH_PRINCESS_TIMER3 || ID == DISPATCH_PRINCESS_TIMER4 || \
		ID == LEARN_RITE_TIMER2 || ID== LEARN_RITE_TIMER3 || ID == LEARN_RITE_TIMER4 || ID == LEARN_RITE_TIMER5 || \
		ID == PLANT_FLOWER_TIMER1 || ID == PLANT_FLOWER_TIMER2 || ID == PLANT_FLOWER_TIMER3 || ID == PLANT_FLOWER_TIMER4);
	if (bFlag1)
	{
		nTime = pData->Flag1;
	}
	else
	{
		nTime = pData->ExpireTime;
	}

	if (nTime < _time32(NULL))
		return OPERATOR_PARAMETER_ERROR;

// 	//首次免费
// 	if(!((ID >= TECHNOLOGY_TIMER && ID <= TECHNOLOGY_TIMER2 && NGUIDE->IsFirstSkillFast(nPlayerID)) || /*科技加速*/
// 		(ID >= DISPATCH_PRINCESS_TIMER1 && ID <= DISPATCH_PRINCESS_TIMER4 && NGUIDE->IsFirstCourtAffairFast(nPlayerID)) ||  /*宫务加速*/
// 		(ID >= LEARN_RITE_TIMER1 && ID <= LEARN_RITE_TIMER5 && NGUIDE->IsFirstRiteFast(nPlayerID)) ||  /*礼仪加速*/
// 		(ID >= PLANT_FLOWER_TIMER1 && ID <= PLANT_FLOWER_TIMER5 && NGUIDE->IsFirstPlantFast(nPlayerID))))  /*养花加速*/
// 	{
		S32 iCostGold = 0;
		if (TECHNOLOGY_TIMER == ID || TECHNOLOGY_TIMER1 == ID || TECHNOLOGY_TIMER2 == ID)
			iCostGold = mCeil((nTime - _time32(NULL)) / (60.0f));
		else
			iCostGold = mCeil((nTime - _time32(NULL)) / (60.0f));

		if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -iCostGold))
			return OPERATOR_HASNO_GOLD;

		SERVER->GetPlayerManager()->AddGold(nPlayerID, -iCostGold, LOG_GOLD_ADVANCE);
/*	}*/

	if (bFlag1)
	{
		TIMESETMGR->AddTimeSet(nPlayerID, ID, 0, pData->Flag, _time32(NULL));
	}
	else
	{
		NotifyTimeDel(nPlayerID, pData);
		TIMESETMGR->RemoveTimeSet(nPlayerID, ID);
	}
	return OPERATOR_NONE_ERROR;
}

enOpError CTimeSetManager::AdvanceTimeUseOther(U32 nPlayerID, S32 ID, S32 AccTime)
{
	if ( !((ID >= DISPATCH_PRINCESS_TIMER1 && ID <= DISPATCH_PRINCESS_TIMER4) ||
		(ID >= LEARN_RITE_TIMER1 && ID <= LEARN_RITE_TIMER5) ||
		(ID >= PLANT_FLOWER_TIMER1 && ID <= PLANT_FLOWER_TIMER5) || 
		(ID >= TECHNOLOGY_TIMER && ID <= TECHNOLOGY_TIMER2)) )
		return OPERATOR_PARAMETER_ERROR;

	TimeSetRef pData = TIMESETMGR->GetData(nPlayerID, ID);
	if (!pData)
		return OPERATOR_PARAMETER_ERROR;

	S32 nTime = 0;
	bool bFlag1 = ((ID >= DISPATCH_PRINCESS_TIMER1 && ID <= DISPATCH_PRINCESS_TIMER4) ||
		(ID >= LEARN_RITE_TIMER1 && ID <= LEARN_RITE_TIMER5) ||
		(ID >= PLANT_FLOWER_TIMER1 && ID <= PLANT_FLOWER_TIMER5));
	if (bFlag1)
	{
		nTime = pData->Flag1;
		nTime = nTime > AccTime + _time32(NULL) ? nTime - AccTime : _time32(NULL);
	}
	else
	{
		nTime = pData->ExpireTime;
		nTime = nTime > AccTime + _time32(NULL) ? nTime - AccTime : _time32(NULL);
	}

	if (nTime < _time32(NULL))
		return OPERATOR_PARAMETER_ERROR;

	if (bFlag1)
	{
		TIMESETMGR->AddTimeSet(nPlayerID, ID, 0, pData->Flag, nTime);
	}
	else
	{
		if (nTime > _time32(NULL)){
			TIMESETMGR->AddTimeSet(nPlayerID, ID, nTime - _time32(NULL), pData->Flag, pData->Flag1);
		}
		else {
			NotifyTimeDel(nPlayerID, pData);
			TIMESETMGR->RemoveTimeSet(nPlayerID, ID);
		}
	}

	if (ID >= LEARN_RITE_TIMER1 && ID <= LEARN_RITE_TIMER5) {
		SUBMIT(nPlayerID, 69, 1);
	}
	else if(ID >= DISPATCH_PRINCESS_TIMER1 && ID <= DISPATCH_PRINCESS_TIMER4) {
		SUBMIT(nPlayerID, 70, 1);
	}
	else if (ID >= PLANT_FLOWER_TIMER1 && ID <= PLANT_FLOWER_TIMER5) {
		SUBMIT(nPlayerID, 71, 1);
	}
	return OPERATOR_NONE_ERROR;
}

auto CanNotifyClient = [](auto pData) {
	auto ID = pData->ID;
	if ((ID >= DISPATCH_PRINCESS_TIMER1 && ID <= DISPATCH_PRINCESS_TIMER4) || (ID >= CARD_DEVELOP_TIMER1 && ID <= CARD_DEVELOP_TIMER6) || \
		ID == TURN_CARD_TIMER || (ID == IMPOSE_TIMER) || (ID >= APPLY_TIMER1 && ID <= APPLY_TIMER6) || \
		(ID >= LEARN_RITE_TIMER1 && ID <= LEARN_RITE_TIMER5) || (ID >= PLANT_FLOWER_TIMER1 && ID <= PLANT_FLOWER_TIMER5) || \
		(ID >= TECHNOLOGY_TIMER && ID <= TECHNOLOGY_TIMER2) || (ID == PRINCE_SLOT_TIMER) || (ID >= SHOP_BUY_TIMER1 && ID <= SHOP_BUY_TIMER20) ||  \
		(ID >= CITY_TIMER1 && ID <= CITY_TIMER2)  ||  ID == IMPOSE_TIMER1 || ID == COUNT_TIMER2 || ID == COUNT_TIMER5 || (ID >= CARD_HIRE_TIMER1 && ID <= CARD_HIRE_TIMER2) ||\
		(ID >= BATTLE_TIMER1 && ID <= BATTLE_TIMER7) || (ID >= DINNER_SHOP_TIME1 && ID <= DINNER_SHOP_TIME2)  || ID == DINNER_FREE_TIME|| \
		ID == TOP_RANK_UP_TIME || (ID >= ACTIVITY_TIME1 && ID <= ACTIVITY_TIME6) || (ID >= ACTIVITY_GOLD_TIME && ID <= ACTIVITY_PRINCE_TIME) || (VIP_TIME == ID) ||\
		(NOVICEGUIDE == ID) || (PVP_CD_TIME == ID) || (PVP_BATTLE_TIMES == ID) || (ID >= DINNER_PRINCESS_TIME1 && ID <= DINNER_PRINCESS_TIME80) || (GATE_IMTER == ID)||\
		ID == ORG_DONATE_TIME)
		return true;

	return false;
};

void CTimeSetManager::UpdateToClient(U32 nPlayerID, TimeSetRef pData)
{
	if (!CanNotifyClient(pData))
		return;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(256 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 256);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_TIME_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CTimeSetManager::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		DATA_MAP& TimeMap = m_PlayerMap[nPlayerID];
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_TIME_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(TimeMap.size(), Base::Bit16);
		for (DATA_ITR itr = TimeMap.begin(); itr != TimeMap.end(); ++itr)
		{
			if (!CanNotifyClient(itr->second))
			{
				sendPacket.writeFlag(false);
			}
			else if (sendPacket.writeFlag(itr->second != NULL))
			{
				itr->second->WriteData(&sendPacket);
			}
		}

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

bool CTimeSetManager::AddPlayerTimeSet(U32 nPayerID)
{
	U32 nCurTime = (U32)_time32(0);
	S32 nNextDay = Platform::getNextTime(Platform::INTERVAL_DAY);
	nNextDay -= nCurTime;

	F32 VSAReducePercent = (100.f + VIPMGR->GetVipVSAReducePercent(nPayerID)) / 100.f;
	S32 VimTopLimit = (S32)VIPMGR->GetVipVIMTopLimit(nPayerID);
	S32 StrengthTopLimit = (S32)VIPMGR->GetVipStrengthTopLimit(nPayerID);
	S32 ActivityTopLimit = (S32)VIPMGR->GetVipActivityTopLimit(nPayerID);
	S32 ImposeTopLimit = (S32)VIPMGR->GetImposeTopLimit(nPayerID);


	if (!GetData(nPayerID, IMPOSE_TIMER))
		AddTimeSet(nPayerID, IMPOSE_TIMER, 60, "", 10);
	if (!GetData(nPayerID, NEXTHHOUR_TIMER))
		AddTimeSet(nPayerID, NEXTHHOUR_TIMER, 30 * 60, "", 0);
	if (!GetData(nPayerID, NEXTDAY_TIMER))
		AddTimeSet(nPayerID, NEXTDAY_TIMER, nNextDay, "", 0);
	if (!GetData(nPayerID, NEXTHOUR_TIMER))
		AddTimeSet(nPayerID, NEXTHOUR_TIMER, 60 * 60, "", 0);
	if (!GetData(nPayerID, NEXTTENMINUTES_TIMER))
		AddTimeSet(nPayerID, NEXTTENMINUTES_TIMER, 10 * 60, "", 0);
	if (!GetData(nPayerID, TURN_CARD_TIMER))
		AddTimeSet(nPayerID, TURN_CARD_TIMER, TRUNCARDCD * VSAReducePercent, "", 5 + VimTopLimit);
	if (!GetData(nPayerID, CITY_TIMER2))
		AddTimeSet(nPayerID, CITY_TIMER2, VISITCITYCD * VSAReducePercent, "", 5 + StrengthTopLimit);
	if (!GetData(nPayerID, IMPOSE_TIMER1))
		AddTimeSet(nPayerID, IMPOSE_TIMER1, 0, "", 10 + ImposeTopLimit);
	/*for (int i = COUNT_TIMER1; i <= COUNT_TIMER6; ++i) {
		if (!GetData(nPayerID, i))
		{
			stTimeSetInfo info("");
			AddTimeSet(nPayerID, i, 0, info.GetFlag(), 0);
		}
	}
	if (!GetData(nPayerID, CARD_HIRE_TIMER1))
	{
		stTimeSetInfo info("");
		AddTimeSet(nPayerID, CARD_HIRE_TIMER1, 0, info.GetFlag(), 0);
	}*/

	return true;
}

bool CTimeSetManager::AddNewPlayerTimeSet(U32 nPayerID)
{
	U32 nCurTime = (U32)_time32(0);
	S32 nNextDay = Platform::getNextTime(Platform::INTERVAL_DAY);
	nNextDay -= nCurTime;

	if (!GetData(nPayerID, NEW_PLAYER_TIMER))
		AddTimeSet(nPayerID, NEW_PLAYER_TIMER, nNextDay, "", 0);

	return true;
}

const char* CTimeSetManager::NewPlayer(U32 nPlayerID)
{
	TimeSetRef pData = GetData(nPlayerID, NEW_PLAYER_TIMER);
	if (pData)
		return "新玩家";

	return "老玩家";
}

auto GetTimeSetDB = [](auto pHandle) {
	TimeSetRef pTime = TimeSetRef(new stTimeSet);
	pTime->ID = pHandle->GetInt();
	pTime->PlayerID = pHandle->GetInt();
	pTime->Flag = pHandle->GetString();
	pTime->Flag1 = pHandle->GetInt();
	pTime->ExpireTime = pHandle->GetInt();
	return pTime;
};

DB_Execution* CTimeSetManager::LoadDB(U32 nPlayerID)
{ 
	RemovePlayer(nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(fmt::sprintf("SELECT ID, PlayerID, Flag, Flag1, ExpireTime FROM Tbl_TimeSet WHERE PlayerID=%d", nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void* pH)
	{
		DB_Execution* pHandle = (DB_Execution*)(pH);
		if (pHandle && error == NONE_ERROR)
		{
			while (pHandle->More())
			{
				TimeSetRef pTime = GetTimeSetDB(pHandle);
				_AddTimeSet(id, pTime);
			}

			AddPlayerTimeSet(id);
		}
	});
	pHandle->RegisterDoneFunction([](int id, int error, void *pH)
	{
		//登录天数
		U32 nPlayerID = id;
		S32 nCurTime = time(NULL);
		{
			S32 nSaveTime = GetTimesInfo(nPlayerID, CARD_HIRE_TIMER2, 1);
			if (nSaveTime < nCurTime) {
				SetTimesInfo(id, CARD_HIRE_TIMER2, 1, (S32)Platform::getNextTime(Platform::INTERVAL_DAY), false);
				SetTimesInfo(id, CARD_HIRE_TIMER2, 0, 1);
			}
		}

		{
			S32 nSaveTime = GetTimesInfo(nPlayerID, LASTTRIGGERLOGIN_TIME, 1);
			if (nSaveTime < nCurTime) {
				S32 EndTime = GetTimesInfo(nPlayerID, LASTTRIGGERLOGIN_TIME, 0);
				auto pActivity = ACTIVITYMGR->GetActivity(ACTIVITY_LOGIN);
				if (pActivity) {
					if (pActivity->EndTime != EndTime) {
						TIMESETMGR->RemoveTimeSet(nPlayerID, ACTIVITY_LOGIN_TIME);
					}

					SUBMITACTIVITY(id, ACTIVITY_LOGIN, 1);
					SetTimesInfo(id, LASTTRIGGERLOGIN_TIME, 1, (S32)Platform::getNextTime(Platform::INTERVAL_DAY), false);
					SetTimesInfo(id, LASTTRIGGERLOGIN_TIME, 0, pActivity->EndTime, false);
				}
			}
		}
	});
	return pHandle;
}

TimeSetRef CTimeSetManager::SyncLoadDB(U32 nPlayerID, U32 nTimeID) {
	TimeSetRef pTime;
	{
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetId(nPlayerID);
		tHandle.SetSql(fmt::sprintf("SELECT ID, PlayerID, Flag, Flag1, ExpireTime FROM Tbl_TimeSet WHERE PlayerID=%d AND ID=%d", nPlayerID, nTimeID));
		tHandle.RegisterFunction([&](int id, int error, void* pH)
		{
			CDBConn* pHandle = (CDBConn*)(pH);
			if (pHandle && error == NONE_ERROR)
			{
				if (pHandle->More())
				{
					pTime = GetTimeSetDB(pHandle);
				}
			}
		});
		tHandle.Commint();
	}
	return pTime;
}

bool CTimeSetManager::UpdateDB(TimeSetRef pTime)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_TimeSet SET Flag='%s', Flag1 =%d, ExpireTime=%d WHERE ID=%d AND PlayerID=%d", \
		pTime->Flag.c_str(), pTime->Flag1, pTime->ExpireTime, pTime->ID, pTime->PlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CTimeSetManager::InsertDB(U32 nPlayerID, TimeSetRef pTime)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("INSERT INTO  Tbl_TimeSet \
			 (ID,		PlayerID,		Flag,			Flag1,			ExpireTime) \
	   VALUES(%d,		%d,				'%s',			%d,				%d)", \
		pTime->ID,	pTime->PlayerID, pTime->Flag.c_str(), pTime->Flag1, pTime->ExpireTime));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CTimeSetManager::DeleteDB(U32 nPlayerID, U32 nTimeID)
{
	TimeSetRef pTime = TimeSetRef(new stTimeSet);
	pTime->PlayerID = nPlayerID;
	pTime->ID = nTimeID;
	pTime->ExpireTime = -1;
	UpdateToClient(nPlayerID, pTime);

	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE Tbl_TimeSet  WHERE ID=%d AND PlayerID=%d", \
		nTimeID, nPlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CTimeSetManager::onPlayerEnter(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	LoadDB(pHead->Id);
	return true;
}

bool CTimeSetManager::onPlayerLeave(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	RemovePlayer(pHead->Id);
	return true;
}

void CTimeSetManager::TimeProcess(U32 bExit)
{
	for (PLAYER_SORT_ITR itr = m_PlayerSortMap.begin(); itr != m_PlayerSortMap.end(); ++itr)
	{
		S32 nCurTime = (S32)_time32(NULL);
		U32 nPlayerID = itr->first;
		TIME_SORT_SET& TimeSet = itr->second;
		for (TIME_SORT_ITR iter = TimeSet.begin(); iter != TimeSet.end();)
		{
			TimeSetRef pTime = *iter;
			//活动过期
			if (pTime && (pTime->ExpireTime <= nCurTime))
			{
				U32 nID = pTime->ID;
				m_TimeStack.push_front(pTime);
				m_PlayerMap[nPlayerID].erase(nID);
				iter = TimeSet.erase(iter);
				DeleteDB(nPlayerID, nID);
			}
			else if (pTime  && (pTime->ExpireTime > nCurTime))
			{
				break;
			}
			else
			{
				++iter;
			}
		}
	}

	while (!m_TimeStack.empty())
	{
		TimeSetRef pTime = *(m_TimeStack.begin());
		if (pTime)
		{ 
			NotifyTimeDel(pTime->PlayerID, pTime);
		}

		m_TimeStack.pop_front();
	}
}

bool CTimeSetManager::HandleAdvanceTimeRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	S32 ID = Packet->readInt(Base::Bit16);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = AdvanceTime(pAccount->GetPlayerId(), ID);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ADVANCETIME_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

void addTimeset(int playerid, int id, int time, std::string val, S32 flag)
{
	TIMESETMGR->AddTimeSet(playerid, id, time, val, flag);
}

void LoginTime(int playerid)
{
	SERVER->GetPlayerManager()->AddGold(50000005, -10);
	TIMESETMGR->LoadDB(playerid);
}

void ___SetTimesInfo(int playerid, int id, int index, int val)
{
	SetTimesInfo(playerid, id, index, val);
}

void ___PrintTimesInfo(int playerid, int id)
{
	auto pTimer = TIMESETMGR->GetData(playerid, id);
	if (pTimer)
	{
		stTimeSetInfo info(pTimer->Flag);
		g_Log.WriteLog(fmt::sprintf("timeinfo :[%d, %d, %d, %d, %d, %d]", info.nVal[0], info.nVal[1], info.nVal[2], info.nVal[3], info.nVal[4], info.nVal[5]));
	}
}

void CTimeSetManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "addTimeset", &addTimeset);
	lua_tinker::def(L, "LoginTime", &LoginTime);
	lua_tinker::def(L, "SetTimesInfo", &___SetTimesInfo);
	lua_tinker::def(L, "PrintTimesInfo", &___PrintTimesInfo);
}
