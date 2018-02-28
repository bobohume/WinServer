#include "PrinceMgr.h"
#include "Common/UtilString.h"
#include "PrincessMgr.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "Common/MemGuard.h"
#include "../WorldServer.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "../PlayerMgr.h"
#include "Common/mRandom.h"
#include "PlayerInfoMgr.h"
#include "ItemMgr.h"
#include "CardMgr.h"
#include "TimeSetMgr.h"
#include "platform.h"
#include "WINTCP/dtServerSocket.h"
#include "PrincessData.h"
#include "BASE/tVector.h"
#include "BASE/mMathFn.h"
#include "ItemData.h"
#include "BuffMgr.h"
#include "DBLayer/Common/DBUtility.h"
#include "../Script/lua_tinker.h"
#include "TaskMgr.h"
#include "Common/FilterOpt.h"
#include "CommLib/ToSql.h"
#include "VIPMgr.h"
#include "NoviceGuide.h"

#define MAX_ACTIVITY_NUM 3
#define ACTIVITY_INTERVAL 30 * 60
const S32 AWARD_TIME_INTERVAL = 12 * 60 * 60;
const S32 static s_PrincePos[8][2] =
{
	{0,			0},
	{0,			4999},
	{5000,		20000},
	{20001,		50000},
	{50001,		100000},
	{100001,	150000},
	{150001,	300000},
	{300001,	S32_MAX},
};

const U32 static s_PrinceOpenHourse[8][2] =
{
	{ 0,			0 },
	{ 100,			1 },
	{ 200,			2 },
	{ 300,			3 },
	{ 400,			4 },
	{ 500,			5 },
	{ 600,			6 },
	{ 700,			7 },
};

const U32 static s_PrinceItem[8] =
{
	0,
	300011,
	300012,
	300013,
	300014,
	300015,
	300016,
	300017,
};

DECLARE_SQL_UNIT(stPrince);
CPrinceMgr::CPrinceMgr()
{
	GET_SQL_UNIT(stPrince).SetName("Tbl_Prince");
	REGISTER_SQL_UNITKEY(stPrince, UID);
	REGISTER_SQL_UNITKEY(stPrince, PlayerID);
	REGISTER_SQL_UNIT(stPrince, PrincessUID);
	REGISTER_SQL_UNIT(stPrince, CardUID);
	REGISTER_SQL_UNIT(stPrince, Name);
	REGISTER_SQL_UNIT(stPrince, Quatily);
	REGISTER_SQL_UNIT(stPrince, Level);
	REGISTER_SQL_UNIT(stPrince, Exp);
	REGISTER_SQL_UNIT(stPrince, Agi);
	REGISTER_SQL_UNIT(stPrince, Int);
	REGISTER_SQL_UNIT(stPrince, Str);
	REGISTER_SQL_UNIT(stPrince, OpenTime);
	REGISTER_SQL_UNIT(stPrince, OpenFlag);
	REGISTER_SQL_UNIT(stPrince, Peerages);
	REGISTER_SQL_UNIT(stPrince, Activity);
	REGISTER_SQL_UNIT(stPrince, ActivityTime);

	REGISTER_EVENT_METHOD("CW_PRICESS_ADDACTIVITY_REQUEST",   this, &CPrinceMgr::HandleClientAddActivity);
	REGISTER_EVENT_METHOD("CW_PRICESS_TRAIN_REQUEST",	      this, &CPrinceMgr::HandleClientTrain);
	REGISTER_EVENT_METHOD("CW_PRICESS_RENAME_REQUEST",		  this, &CPrinceMgr::HandleClientRename);
	REGISTER_EVENT_METHOD("CW_PRICESS_OPENHOURSE_REQUEST",	  this, &CPrinceMgr::HandleClientOpenHourse);
	REGISTER_EVENT_METHOD("CW_PRICESS_AWARDITEM_REQUEST",	  this, &CPrinceMgr::HandleClientAwardItem);
	REGISTER_EVENT_METHOD("CW_PRICESS_DELETE_REQUEST",		  this, &CPrinceMgr::HandleClientDeletePrince);
	REGISTER_EVENT_METHOD("CW_PRICESS_ADDMAXSLOT_REQUEST",	  this, &CPrinceMgr::HandleClientAddMaxSlotPrince);

	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CPrinceMgr::TimeProcess, 5 * 1000);
}

CPrinceMgr::~CPrinceMgr()
{
}

CPrinceMgr* CPrinceMgr::Instance()
{
	static CPrinceMgr s_Mgr;
	return &s_Mgr;
}

void CPrinceMgr::TimeProcess(U32 bExit)
{
	auto nCurTime = (S32)time(NULL);
	for (auto itr = m_UpdateMap.begin(); itr != m_UpdateMap.end();){
		DATA_MAP& DataMap = itr->second;
		for (auto itr1 = DataMap.begin(); itr1 != DataMap.end();) {
			PrinceRef& pData = itr1->second;
			if (pData && nCurTime >= pData->ActivityTime) {
				auto nRecoverActivity = GetRecoverTimes(nCurTime, pData->ActivityTime, ACTIVITY_INTERVAL * ((100.f + VIPMGR->GetVipVSAReducePercent(pData->PlayerID)) / 100.f));
				if (pData->Activity + nRecoverActivity < MAX_ACTIVITY_NUM + VIPMGR->GetVipActivityTopLimit(pData->PlayerID))
					pData->Activity += nRecoverActivity;
				else
					pData->Activity = MAX_ACTIVITY_NUM + VIPMGR->GetVipActivityTopLimit(pData->PlayerID);
				
				if (pData->Activity < MAX_ACTIVITY_NUM + VIPMGR->GetVipActivityTopLimit(pData->PlayerID)) {
					S32 ActivityInterval = ACTIVITY_INTERVAL * ((100 + VIPMGR->GetVipVSAReducePercent(pData->PlayerID)) / 100);
					pData->ActivityTime = time(NULL) + ActivityInterval;
				}

				UpdatePrinceActivity(pData->PlayerID, pData);
				UpdateToClient(pData->PlayerID, pData);
				if (pData->Activity >= MAX_ACTIVITY_NUM + VIPMGR->GetVipActivityTopLimit(pData->PlayerID)){
					itr1 = DataMap.erase(itr1);
				}
				else {
					++itr1;
				}
			}
			else {
				++itr1;
			}
		}

		if (DataMap.empty()) {
			itr = m_UpdateMap.erase(itr);
		}
		else {
			++itr;
		}
	}
}

void CPrinceMgr::ReduceTimePercent(U32 nPlayerID, F32 OldReducePercent, F32 NewReducePercent)
{
	auto PlayerMap = GetDataMap(nPlayerID);
	for (auto itr = PlayerMap.begin(); itr != PlayerMap.end(); ++itr)
	{
		itr->second->ActivityTime = time(NULL) + (itr->second->ActivityTime - time(NULL)) * ((100 + NewReducePercent - OldReducePercent) / 100);

		PRINCEMGR->UpdateDB(nPlayerID, itr->second);
		PRINCEMGR->UpdateToClient(nPlayerID, itr->second);
		
	}
}

PrinceRef CPrinceMgr::CreateData(U32 nPlayerID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		PrinceRef pData = PrinceRef(new stPrince);
		pData->PlayerID = nPlayerID;
		pData->UID = uuid();
		return pData;
	}
	DBECATCH()

	return NULL;
}

CPrinceMgr::DATA_MAP& CPrinceMgr::GetDataMap(U32 nPlayerID)
{
	return m_PlayerMap[nPlayerID];
}

//---get orange prince num
auto GetOrangeRate = [](S32 nCharm, S32 nPrincessNum) {
	S32 nRateVal = mClamp(mFloor((nCharm - (nPrincessNum + 1) * 500) / 10.0f), 0, 100);
	return nRateVal;
};

auto GetAddQuatily = [](S32 Quatily) {
	auto fAddQuatily = mPow(F32(Quatily), 2) / 10.0f + 0.9;
	return fAddQuatily;
};

auto CanTrainPrince = [](auto& DataMap, std::string UID)
{
	for (auto itr : DataMap)
	{
		PrinceRef pData = itr.second;
		if (pData->CardUID == UID)
			return false;
	}
	return true;
};

auto GetLimitLevel = [](S32 Quatily) {
	switch (Quatily)
	{
	case 1:
		return 25;
	case 2:
		return 50;
	case 3:
		return 75;
	default:
		return 100;
	}
};

auto CaculateBuf = [](auto& buff, auto& pData) {
	if (pData->OpenFlag == 1)
	{
		buff.Business += pData->Agi;
		buff.Military += pData->Str;
		buff.Technology += pData->Int;
	}
};

auto CaculateStats = [](auto& pData) {
	if (pData->PrincessUID != "")
	{
		PrincessRef pPrincess = PRINCESSMGR->GetData(pData->PlayerID, pData->PrincessUID);
		if (pPrincess)
		{
			pData->AgiEx = pData->Agi * (1.0f + pPrincess->PrinceAbility / 100.0f);
			pData->IntEx = pData->Int * (1.0f + pPrincess->PrinceAbility / 100.0f);
			pData->StrEx = pData->Str * (1.0f + pPrincess->PrinceAbility / 100.0f);
		}
	}
};

bool CPrinceMgr::AddData(U32 nPlayerID, std::string PrincessUID)
{
	StatsRef pStats = BUFFMGR->GetData(nPlayerID);
	U32 nMaxSlot = 2 + (pStats ? pStats->PrinceNum : 0) + VIPMGR->GetPriceNumTopLimit(nPlayerID);
 	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PRINCE_SLOT_TIMER);
 	if (pTime)
 		nMaxSlot += pTime->Flag1;

	S32 nNum = 0;
	{
		DATA_MAP& DataMap = GetDataMap(nPlayerID);
		for (auto itr = DataMap.begin(); itr != DataMap.end(); ++itr)
		{
			auto pData = itr->second;
			if (pData && pData->OpenFlag == 0)
				++nNum;
		}
	}

	if (nNum >= nMaxSlot)
		return false;

	PrincessRef pPrincess = PRINCESSMGR->GetData(nPlayerID, PrincessUID);
	if (!pPrincess)
		return false;

	PrinceRef pData = CreateData(nPlayerID);
	if (!pData)
		return false;

	S32 nOrangeRate = 0, nPurple = 0, nBlue = 0, nGreen = 0;
	pData->PrincessUID = PrincessUID;
	nOrangeRate = GetOrangeRate(pPrincess->Charm, pPrincess->PrinceNum);
	S32 nRateVal = 1;
	if (!NGUIDE->IsFirstAddPrince(nPlayerID)) {
		nRateVal = gRandGen.randI(1, 100);
	}
	nGreen  = nOrangeRate + mFloor(((100 - nOrangeRate) / 6.0f) * 3);
	nBlue   = nGreen + mFloor(((100 - nOrangeRate) / 6.0f) * 2);
	nPurple = nBlue + mFloor(((100 - nOrangeRate) / 6.0f) * 1);

	if (nRateVal <= nOrangeRate)
		pData->Quatily = 4;
	else if (nRateVal <= nGreen)
		pData->Quatily = 1;
	else if (nRateVal <= nBlue)
		pData->Quatily = 2;
	else
		pData->Quatily = 3;

	S32 nInitVal = mFloor(GetAddQuatily(pData->Quatily) * pPrincess->Support);
	pData->Agi = pData->Int = pData->Str = nInitVal;
	pData->Activity = 3;
	CaculateStats(pData);
	stBuff buff;
	CaculateBuf(buff, pData);
	PLAYERINFOMGR->AddBuff(nPlayerID, 1, buff);
	Parent::AddData(nPlayerID, pData->UID, pData);
	InsertDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	SetTimesInfo(nPlayerID, COUNT_TIMER2, 0, 1);
	SUBMIT(nPlayerID, 26, 1);
	return true;
}

enOpError CPrinceMgr::TrainPrince(U32 nPlayerID, std::string CUID, std::string UID)
{
	CardRef pCard = CARDMGR->GetData(nPlayerID, CUID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	PrinceRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCE;

	if (pData->Activity <= 0)
		return OPERATOR_HASHNO_ACTIVITY;

	//if (!pData->CardUID.empty())
	//	return OPERATOR_PRINCE_EXIST_CARD;

	if (pData->CardUID == "" && (!CanTrainPrince(m_PlayerMap[nPlayerID], CUID)))
		return OPERATOR_PRINCE_EXIST_CARD;

	S32 nLimitLV = GetLimitLevel(pData->Quatily);
	if (pData->Level >= nLimitLV)
		return OPERATOR_PRINCE_LIMIT_LEVEL;

	pData->Agi = mClamp((S32)pData->Agi + (pCard->Agi* GetAddQuatily(pData->Quatily) * 0.0002 + 1) * gRandGen.randF(0.90, 1.10), (S32)0, (S32)S32_MAX);
	pData->Int = mClamp((S32)pData->Int + (pCard->Int* GetAddQuatily(pData->Quatily) * 0.0002 + 1) * gRandGen.randF(0.90, 1.10), (S32)0, (S32)S32_MAX);
	pData->Str = mClamp((S32)pData->Str + (pCard->Str* GetAddQuatily(pData->Quatily) * 0.0002 + 1) * gRandGen.randF(0.90, 1.10), (S32)0, (S32)S32_MAX);

	S32 nLevelExp = mCeil(pData->Level / 10.0f);
	if (pData->Exp + 1 >= nLevelExp)
	{
		pData->Exp = 0;
		pData->Level = mClamp(pData->Level + 1, 0, 100);

		S32 nSupport = 1;
		PrincessRef pPrincess = PRINCESSMGR->GetData(nPlayerID, pData->PrincessUID);
		if (pPrincess)
			nSupport = pPrincess->Support;

		pData->Agi = mClamp((S32)pData->Agi + (nSupport * GetAddQuatily(pData->Quatily) * 0.2) * gRandGen.randF(0.90, 1.10), (S32)0, (S32)S32_MAX) + pData->Level;
		pData->Int = mClamp((S32)pData->Int + (nSupport * GetAddQuatily(pData->Quatily) * 0.2) * gRandGen.randF(0.90, 1.10), (S32)0, (S32)S32_MAX) + pData->Level;
		pData->Str = mClamp((S32)pData->Str + (nSupport * GetAddQuatily(pData->Quatily) * 0.2) * gRandGen.randF(0.90, 1.10), (S32)0, (S32)S32_MAX) + pData->Level;

		if (pData->Level >= nLimitLV)
		{
			AwardPeerages(nPlayerID, UID);
		}
	}
	else
	{
		pData->Exp = mClamp((S32)pData->Exp + 1, (S32)0, (S32)S32_MAX);
	}

	pData->CardUID = CUID;
	if (pData->Activity == MAX_ACTIVITY_NUM + VIPMGR->GetVipActivityTopLimit(pData->PlayerID)) {
		S32 ActivityInterval = ACTIVITY_INTERVAL * ((100 + VIPMGR->GetVipVSAReducePercent(pData->PlayerID)) / 100);
		pData->ActivityTime = (S32)time(NULL) + ActivityInterval;
	}
	pData->Activity = mClamp((S32)pData->Activity - 1, (S32)0, (S32)S8_MAX);

	AddUpdateMap(nPlayerID, pData);
	CaculateStats(pData);
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	SUBMIT(nPlayerID, 63, 1);
	SUBMIT(nPlayerID, 12, 1);
	SUBMITACTIVITY(nPlayerID, ACTIVITY_PRINCE, 1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrinceMgr::AwardPeerages(U32 nPlayerID, std::string UID)
{
	PrinceRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCE;

	CaculateStats(pData);
	S32 nTatalAbility = pData->AgiEx + pData->IntEx + pData->StrEx;
	for (auto i = 1; i < 8; ++i)
	{
		if (nTatalAbility >= s_PrincePos[i][0] && nTatalAbility <= s_PrincePos[i][1])
		{ 
			pData->Peerages = i;
			break;
		}
	}

	return OPERATOR_NONE_ERROR;
}

enOpError CPrinceMgr::RemoveData(U32 nPlayerID, std::string UID)
{
	PrinceRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCE;

	S32 nLimitLv = GetLimitLevel(pData->Quatily);
	if (pData->Level < nLimitLv)
		return OPERATOR_EXILE_PRINCE_FAILED;

	CaculateStats(pData);
	S32 nAddMoney	 = pData->AgiEx * 1000;
	S32 nAddOre		 = pData->IntEx * 10;
	S32 nAddTroops	 = pData->StrEx * 10;

	PLAYERINFOMGR->AddMoney(nPlayerID, nAddMoney);
	PLAYERINFOMGR->AddOre(nPlayerID, nAddOre);
	PLAYERINFOMGR->AddTroops(nPlayerID, nAddTroops);

	stBuff buff;
	CaculateBuf(buff, pData);
	-buff;
	PLAYERINFOMGR->AddBuff(nPlayerID, 1, buff);

	DeleteData(nPlayerID, UID);
	DeleteDB(nPlayerID, UID);
	SetTimesInfo(nPlayerID, COUNT_TIMER2, 0, -1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrinceMgr::OpenHourse(U32 nPlayerID, std::string UID, S32 nType)
{
	static S32 s_CostItemId = 600089;
	if (nType < 0 || nType > 1)
		return OPERATOR_PARAMETER_ERROR;

	PrinceRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCE;

	S32 nLimitLv = GetLimitLevel(pData->Quatily);
	if (pData->Level < nLimitLv)
		return OPERATOR_PRINCE_OPEN_HOURSE_FAILED;

	U32 nCurTime = _time32(NULL);
	if (pData->OpenFlag != 0)
		return OPERATOR_PRINCE_OPEN_HOURSE_FAILED;

	S8  nPos = mClamp(pData->Peerages, 0, 7);
	S32 nCostGold = s_PrinceOpenHourse[pData->Peerages][0];
	S32 nCostItem = s_PrinceOpenHourse[pData->Peerages][1];
	if (nType == 0 && !SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -nCostGold))
		return OPERATOR_HASNO_GOLD;
	else if (nType == 1 && !ITEMMGR->CanReduceItem(nPlayerID, s_CostItemId, -nCostItem))
		return OPERATOR_HASNO_ITEM;

	pData->OpenTime = _time32(NULL);
	pData->OpenFlag = 1;
	pData->CardUID = "";
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	if (nType == 0)
		SERVER->GetPlayerManager()->AddGold(nPlayerID, -nCostGold, LOG_GOLD_PRINCE);
	else
		ITEMMGR->AddItem(nPlayerID, s_CostItemId, -nCostItem);

	//计算国力
	stBuff buff;
	CaculateBuf(buff, pData);
	PLAYERINFOMGR->AddBuff(nPlayerID, 1, buff);

	SUBMIT(nPlayerID, 44, 1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrinceMgr::AwardItem(U32 nPlayerID, std::string UID, std::pair<S32, S32> DropInfo[3])
{
	PrinceRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCE;

	U32 nCurTime = _time32(NULL);
	if (nCurTime < pData->OpenTime && pData->OpenFlag == 0)
		return OPERATOR_PRINCE_OPEN_HOURSE_FAILED;

	//发送物品
	for (auto i = 0; i < 1; ++i)
	{
		DropInfo[i] = ITEMMGR->DropItem(nPlayerID, s_PrinceItem[pData->Peerages]);
	}

	pData->OpenTime = _time32(NULL) + AWARD_TIME_INTERVAL;
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	SUBMIT(nPlayerID, 45, 1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrinceMgr::AddMaxSlot(U32 nPlayerID)
{
	S32 nTimes = 0;
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PRINCE_SLOT_TIMER);
	if (pTime)
	{
		nTimes = pTime->Flag1;
	}

	S32 nCostGold = (nTimes + 1) * 100;
	if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -nCostGold))
		return OPERATOR_HASNO_GOLD;

	TIMESETMGR->AddTimeSet(nPlayerID, PRINCE_SLOT_TIMER, 0, "", nTimes + 1);
	SERVER->GetPlayerManager()->AddGold(nPlayerID, -nCostGold, LOG_GOLD_PRINCE);
	return OPERATOR_NONE_ERROR;
}

bool CPrinceMgr::AddPrinceSlot(U32 nPlayerID, U32 Number)
{
	S32 nTimes = 0;
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PRINCE_SLOT_TIMER);
	if (pTime)
	{
		nTimes = pTime->Flag1;
	}
	TIMESETMGR->AddTimeSet(nPlayerID, PRINCE_SLOT_TIMER, 0, "", nTimes + Number);
	return true;
}

enOpError CPrinceMgr::AddActivity(U32 nPlayerID, std::string UID, S32 nItemID)
{
	PrinceRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCE;

	if ((nItemID != 600062) && (nItemID != 600063) && (nItemID != 600064))
		return OPERATOR_PARAMETER_ERROR;

	CItemData* pItemData = g_ItemDataMgr->getData(nItemID);
	if (!pItemData || pItemData->CanUse())
		return OPERATOR_HASNO_ITEM;

	if (!ITEMMGR->CanReduceItem(nPlayerID, nItemID, -1))
		return OPERATOR_HASNO_ITEM;

	if ((S32)(pData->Activity + pItemData->m_UseVal) > 99)
		return OPERATOR_PRINCE_ACTIVITY_MAX;

	pData->Activity = mClamp((S8)(pData->Activity + pItemData->m_UseVal), 0, S8_MAX);
	RemoveUpdateMap(nPlayerID, pData);
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	ITEMMGR->AddItem(nPlayerID, nItemID, -1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrinceMgr::Rename(U32 nPlayerID, std::string UID, std::string Name)
{
	PrinceRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCE;

	bool bCostMoney = (pData->Name != "");
	if (bCostMoney)
	{
		if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -10))
			return OPERATOR_HASNO_GOLD;
	}

	char *pName = Util::Utf8ToMbcs(Name.c_str());
	std::string sName = pName;
	delete[] pName;
	if (!StrSafeCheckEx(sName.c_str(), 50))
		return OPERATOR_PARAMETER_ERROR;

	if (FilterOpt::isBanName(sName.c_str()))
		return OPERATOR_BAN_WORD;

	pData->Name = sName;
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	if (bCostMoney)
		SERVER->GetPlayerManager()->AddGold(nPlayerID, -10, LOG_GOLD_PRINCE);
	return OPERATOR_NONE_ERROR;
}

void CPrinceMgr::RemovePlayer(U32 nPlayerID)
{
	PLAYER_ITR  itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		DATA_MAP& PlayerMap = itr->second;
		PlayerMap.clear();
		m_PlayerMap.erase(itr);
	}
	RemoveUpdateMap(nPlayerID);
}

void CPrinceMgr::AddUpdateMap(U32 nPlayerID, PrinceRef pData) {
	if (pData && pData->Activity < MAX_ACTIVITY_NUM + VIPMGR->GetVipActivityTopLimit(pData->PlayerID)) {
		m_UpdateMap[nPlayerID].insert(DATA_MAP::value_type(pData->UID, pData));
	}
}

void CPrinceMgr::RemoveUpdateMap(U32 nPlayerID, PrinceRef pData) {
	if (pData && pData->Activity > MAX_ACTIVITY_NUM + VIPMGR->GetVipActivityTopLimit(pData->PlayerID)) {
		PLAYER_ITR  itr = m_UpdateMap.find(nPlayerID);
		if (itr != m_UpdateMap.end())
		{
			DATA_MAP& PlayerMap = itr->second;
			PlayerMap.erase(pData->UID);
			if (PlayerMap.empty()) {
				m_UpdateMap.erase(itr);
			}
		}
	}
}

void CPrinceMgr::RemoveUpdateMap(U32 nPlayerID) {
	PLAYER_ITR  itr = m_UpdateMap.find(nPlayerID);
	if (itr != m_UpdateMap.end())
	{
		DATA_MAP& PlayerMap = itr->second;
		PlayerMap.clear();
		m_UpdateMap.erase(itr);
	}
}

void CPrinceMgr::UpdateToClient(U32 nPlayerID, PrinceRef pData)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_PRINCE_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CPrinceMgr::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		DATA_MAP& DataMap = m_PlayerMap[nPlayerID];
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_PRINCE_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(DataMap.size(), Base::Bit16);
		for (DATA_ITR itr = DataMap.begin(); itr != DataMap.end(); ++itr)
		{
			if (sendPacket.writeFlag(itr->second != NULL))
			{
				itr->second->WriteData(&sendPacket);
			}
		}

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

DB_Execution* CPrinceMgr::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	//pHandle->SetSql(fmt::sprintf("SELECT UID, PlayerID, PrincessUID, CardUID, Name, Quatily, Level, \
		Exp, Agi, Int, Str, OpenTime, OpenFlag, Peerages, Activity, ActivityTime FROM Tbl_Prince WHERE PlayerID=%d", nPlayerID));
	pHandle->SetSql(GET_SQL_UNIT(stPrince).LoadSql("PlayerID", nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void* pSqlHandle)
	{
		DB_Execution* pHandle = (DB_Execution*)(pSqlHandle);
		if (pHandle && error == NONE_ERROR)
		{
			stBuff buff;
			while (pHandle->More())
			{
				PrinceRef pData = PrinceRef(new stPrince);
				pData->UID = pHandle->GetString();
				pData->PlayerID = pHandle->GetInt();
				pData->PrincessUID = pHandle->GetString();
				pData->CardUID = pHandle->GetString();
				pData->Name = pHandle->GetString();
				pData->Quatily = pHandle->GetInt();
				pData->Level = pHandle->GetInt();
				pData->Exp = pHandle->GetInt();
				pData->Agi = pHandle->GetInt();
				pData->Int = pHandle->GetInt();
				pData->Str = pHandle->GetInt();
				pData->OpenTime = pHandle->GetTime();
				pData->OpenFlag = pHandle->GetInt();
				pData->Peerages = pHandle->GetInt();
				pData->Activity = pHandle->GetInt();
				pData->ActivityTime = pHandle->GetInt();
				Parent::AddData(id, pData->UID, pData);
				AddUpdateMap(id, pData);
				CaculateBuf(buff, pData);
				CaculateStats(pData);
			}
			PLAYERINFOMGR->AddBuff(id, 1, buff, true);
		}
	}
	);
	return pHandle;
}

bool CPrinceMgr::UpdatePrinceActivity(U32 nPlayerID, PrinceRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Prince SET Activity=%d WHERE UID='%s'", \
		pData->Activity, pData->UID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPrinceMgr::UpdateDB(U32 nPlayerID, PrinceRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(GET_SQL_UNIT(stPrince).UpdateSql(pData));
	//pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Prince SET \
		PrincessUID='%s',		CardUID='%s',					Name='%s',							Quatily=%d,						Level=%d,\
		Exp=%d,					Agi=%d,							Int=%d,								Str=%d,							OpenTime=%d,\
		OpenFlag=%d,			Peerages=%d,					Activity=%d,						ActivityTime=%d\
		WHERE UID='%s' AND		PlayerID=%d", \
	pData->PrincessUID.c_str(), pData->CardUID.c_str(),			pData->Name.c_str(),				pData->Quatily,					pData->Level, \
		pData->Exp,				pData->Agi,						pData->Int,							pData->Str,						pData->OpenTime, \
		pData->OpenFlag,		pData->Peerages,				pData->Activity,					pData->ActivityTime,\
		pData->UID.c_str(), nPlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPrinceMgr::InsertDB(U32 nPlayerID, PrinceRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(GET_SQL_UNIT(stPrince).InsertSql(pData));
	//pHandle->SetSql(fmt::sprintf("INSERT INTO Tbl_Prince\
	(UID,						PlayerID,					PrincessUID,					CardUID,				Name,\
	Quatily,					Level,						Exp,							Agi,					Int,\
	Str,						OpenTime,					OpenFlag,						Peerages,				Activity,\
	ActivityTime)\
	VALUES('%s',				%d,							'%s',							'%s',					'%s',\
	%d,							%d,							%d,								%d,						%d,\
	%d,							%d,							%d,								%d,						%d,\
	ActivityTime)",\
	pData->UID.c_str(),			pData->PlayerID,			pData->PrincessUID.c_str(),		pData->CardUID.c_str(), pData->Name.c_str(),\
	pData->Quatily,				pData->Level,				pData->Exp,						pData->Agi,				pData->Int,\
	pData->Str,					pData->OpenTime,			pData->OpenFlag,				pData->Peerages,		pData->Activity,\
	pData->ActivityTime));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPrinceMgr::DeleteDB(U32 nPlayerID, std::string UID)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE FROM Tbl_Prince WHERE UID = '%s'", UID.c_str()));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPrinceMgr::HandleClientAddActivity(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	S32 nItemID = Packet->readInt(Base::Bit32);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = AddActivity(pAccount->GetPlayerId(), UID, nItemID);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PRICESS_ADDACTIVITY_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrinceMgr::HandleClientTrain(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string CUID = Packet->readString(MAX_UUID_LENGTH);
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = TrainPrince(pAccount->GetPlayerId(), CUID, UID);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PRICESS_TRAIN_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrinceMgr::HandleClientRename(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	std::string Name = Packet->readString(MAX_UUID_LENGTH);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = Rename(pAccount->GetPlayerId(), UID, Name);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PRICESS_RENAME_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrinceMgr::HandleClientOpenHourse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	U32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = OpenHourse(pAccount->GetPlayerId(), UID, nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PRICESS_OPENHOURSE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrinceMgr::HandleClientAwardItem(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		DROPITEM_PAIR DropInfo[3];
		U32 nError = AwardItem(pAccount->GetPlayerId(), UID, DropInfo);
		CMemGuard Buffer(512 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 512);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PRICESS_AWARDITEM_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(3, Base::Bit16);
		for(int i = 0; i < 3; ++i)
		{
			sendPacket.writeFlag(true);
			sendPacket.writeInt(DropInfo[i].first, Base::Bit32);
			sendPacket.writeInt(DropInfo[i].second, Base::Bit32);
		}
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrinceMgr::HandleClientDeletePrince(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = RemoveData(pAccount->GetPlayerId(), UID);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PRICESS_DELETE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrinceMgr::HandleClientAddMaxSlotPrince(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = AddMaxSlot(pAccount->GetPlayerId());
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PRICESS_ADDMAXSLOT_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

void __addPrince(int id, char* str)
{
	PRINCEMGR->AddData(id, str);
}

void CPrinceMgr::Export(struct lua_State* L)
{
	lua_tinker::def(L, "addPrince", __addPrince);
}