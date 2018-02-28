#include "PrincessMgr.h"
#include "PrinceMgr.h"
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
#include "BuffMgr.h"
#include "TimeSetMgr.h"
#include "platform.h"
#include "WINTCP/dtServerSocket.h"
#include "PrincessData.h"
#include "BASE/tVector.h"
#include "BASE/mMathFn.h"
#include "ItemData.h"
#include "SkillData.h"
#include "BuffData.h"
#include "CommLib/format.h"
#include "TaskMgr.h"
#include "DBLayer/Data/TBLExecution.h"
#include "TopMgr.h"
#include "NoviceGuide.h"
#include "VIPMgr.h"

const U32 static s_PrincessPos[9][3] =
{
//  num			Rite		  charm
	{ U32_MAX,	0,				0 },
	{ U32_MAX,	200,			5 },
	{ U32_MAX,	1000,			10 },
	{ U32_MAX,  5000,			25 },
	{ 6,		10000,			50 },
	{ 4,		20000,			100 },
	{ 2, 		50000,			150 },
	{ 1,		100000,			250 },
	{ 1,		200000,			500 },
};

const U32 static s_PrincessDispatch[4][8] =
{
// slot		waittime		  battle       charm        item		itemnum		item1		rate
	{0,		60 * 60,			60,			20,			600004,		1,			600096,		25},
	{1,		2 * 60 * 60,		120,		50,			600004,		2,			600096,		50},
	{2,		3 *	60 * 60,		180,		100,		600004,		3,			600096,		75},
	{3,		4 * 60 * 60,		240,		100,		600004,		4,			600096,		100},
};

CPrincessMgr::CPrincessMgr()
{
	g_PrincessDataMgr->read();
	g_FlowerDataMgr->read();

	REGISTER_EVENT_METHOD("CW_SUPPORT_PRINCESS_REQUEST",	 this, &CPrincessMgr::HandleClientSupportRequest);
	REGISTER_EVENT_METHOD("CW_AWARD_PRINCESS_REQUEST",		 this, &CPrincessMgr::HandleClientAwardRequest);
	REGISTER_EVENT_METHOD("CW_TURNCARD_PRINCESS_REQUEST",	 this, &CPrincessMgr::HandleClientTurnCardRequest);
	REGISTER_EVENT_METHOD("CW_CHOOSE_PRINCESS_REQUEST",		 this, &CPrincessMgr::HandleClientChooseRequest);
	REGISTER_EVENT_METHOD("CW_BESTOWED_PRINCESS_REQUEST",	 this, &CPrincessMgr::HandleClientBestowedRequest);
	REGISTER_EVENT_METHOD("CW_DISPATCH_PRINCESS_REQUEST",	 this, &CPrincessMgr::HandleClientDispatchRequest);
	REGISTER_EVENT_METHOD("CW_LEARNRITE_PRINCESS_REQUEST",	 this, &CPrincessMgr::HandleClientLearnRiteRequest);
	REGISTER_EVENT_METHOD("CW_PLANTFLOWER_PRINCESS_REQUEST", this, &CPrincessMgr::HandleClientPlantFlowerRequest);
	REGISTER_EVENT_METHOD("CW_FINISH_PRINCESS_REQUEST",      this, &CPrincessMgr::HandleClientFinishRequest);
	REGISTER_EVENT_METHOD("CW_ADDMAXSLOT_PRINCESS_REQUEST",  this, &CPrincessMgr::HandleClientAddMaxSlotRequest);
}

CPrincessMgr::~CPrincessMgr()
{
	g_PrincessDataMgr->clear();
	g_FlowerDataMgr->clear();
}

CPrincessMgr* CPrincessMgr::Instance()
{
	static CPrincessMgr s_Mgr;
	return &s_Mgr;
}

auto CaculatePrincessStats = [](auto& pData) {
	g_Stats.Clear();
	PRINCESSMGR->CaculateStats(pData->PlayerID, pData->UID, g_Stats);
	auto pStats = BUFFMGR->GetData(pData->PlayerID);
	pData->CharmEx = pData->Charm + s_PrincessPos[pData->PrincessPos][2] + g_Stats.Charm + (pStats ? pStats->OrgCharm : 0);
};

auto GetPlantFlowerTime = [](auto nPlayerID, auto nSlot) {
	if (nSlot < 0 || nSlot > 4)
		return 0;

	auto nTimes = 0;
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PLANT_FLOWER_TIMER5);
	if (pTime)
	{
		nTimes = pTime->Flag[nSlot]
	}
};

auto CaculatePrcinessBattleVal = [](auto pData) {
	if (!pData)
		return false;

	S32 nBattleVal = mClamp(pData->Rite + pData->Culture + pData->Battle + pData->Support * 100 + pData->CharmEx * 100, 0, S32_MAX);
	PLAYERINFOMGR->AddQueenVal(pData->PlayerID, -pData->BattleVal);
	pData->BattleVal = nBattleVal;
	PLAYERINFOMGR->AddQueenVal(pData->PlayerID, pData->BattleVal);
	TOPMGR->NewInData(eTopType_Princess, pData->PlayerID, pData->BattleVal, pData->PrincessID);
	PRINCESSMGR->SKillLvUpPrincess(pData->PlayerID, pData->UID);
	SUBMIT(pData->PlayerID, 41, pData->CharmEx);
	SUBMIT(pData->PlayerID, 43, pData->Support);
};

//获取妃位数量
auto GetPrincessPosNum = [](auto& DataMap, S32 nPrincessPos) {
	auto nPrincessNum = 0;
	for (auto itr : DataMap)
	{
		if (itr.second->PrincessPos == nPrincessPos)
		{
			++nPrincessNum;
		}
	}
	return nPrincessNum;
};

auto CaculatePrincessBuff = [](auto nPlayerID, auto& buff, auto& pData)
{
	g_Stats.Clear();
	for (auto i = 0; i < 3; ++i)
	{
		if (pData->Skill[i])
		{
			CSkillData* pSkill = g_SkillDataMgr->getData(pData->Skill[i]);
			if (pSkill)
			{
				pSkill->getStats(g_Stats);
			}
		}
	}
	BUFFMGR->AddStatus(nPlayerID, g_Stats);
	buff.Business += g_Stats.Bus;
	buff.Military += g_Stats.Mil;
	buff.Technology += g_Stats.Tec;
};

auto CaculateBuffByID = [](auto nPlayerID, auto& buff, auto nSkillID)
{
	g_Stats.Clear();
	if (nSkillID)
	{
		CSkillData* pSkill = g_SkillDataMgr->getData(nSkillID);
		if (pSkill)
		{
			pSkill->getStats(g_Stats);
		}
	}

	buff.Business += g_Stats.Bus;
	buff.Military += g_Stats.Mil;
	buff.Technology += g_Stats.Tec;
};

auto CaculateStatsByID = [](auto nPlayerID, auto& stats, auto nSkillID)
{
	stats.Clear();
	if (nSkillID)
	{
		CSkillData* pSkill = g_SkillDataMgr->getData(nSkillID);
		if (pSkill)
		{
			pSkill->getStats(stats);
		}
	}
};


bool CPrincessMgr::CaculateStats(U32 nPlayerID, std::string UID, Stats& stats)
{
	PrincessRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return false;

	stats.Clear();
	for (auto i = 0; i < 3; ++i)
	{
		if (pData->Skill[i])
		{
			CSkillData* pSkill = g_SkillDataMgr->getData(pData->Skill[i]);
			if (pSkill)
			{
				pSkill->getStats(stats);
			}
		}
	}
	return true;
}

bool CPrincessMgr::AddSupport(U32 nPlayerID, U32 nID)
{
	//已经拥有不能重复添加
	PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		DATA_MAP& DataMap = itr->second;
		for (DATA_ITR itr = DataMap.begin(); itr != DataMap.end(); ++itr)
		{
			PrincessRef pData = itr->second;
			if (pData && pData->PrincessID == nID)
			{
				++pData->Support;
				UpdateDB(nPlayerID, pData);
				UpdateToClient(nPlayerID, pData);
				return true;
			}
		}
	}

	return false;
}

PrincessRef CPrincessMgr::CreateData(U32 nPlayerID, U32 nID)
{
	CPrincessData* pPrincessData = g_PrincessDataMgr->getData(nID);
	if (!pPrincessData)
		return NULL;

	DBError err = DBERR_UNKNOWERR;
	try
	{
		PrincessRef pData = PrincessRef(new stPrincess);
		pData->PlayerID = nPlayerID;
		pData->PrincessID = nID;
		pData->UID = uuid();
		pData->Charm = pPrincessData->m_InitCharm;
		for (int i = 0; i < 3; ++i)
		{
			if (pPrincessData->m_SkillID[i])
				pData->Skill[i] = pPrincessData->m_SkillID[i];
		}

		return pData;
	}
	DBECATCH()

	return NULL;
}

bool CPrincessMgr::CanAddData(U32 nPlayerID, U32 nID)
{
	//已经拥有不能重复添加
	PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		DATA_MAP& DataMap = itr->second;
		for (DATA_ITR itr = DataMap.begin(); itr != DataMap.end(); ++itr)
		{
			if (itr->second && itr->second->PrincessID == nID)
				return false;
		}
	}

	return true;
}

bool CPrincessMgr::AddData(U32 nPlayerID, U32 nID, S32 nSupport/*=0*/, bool bNotify/*=true*/)
{
	if (!CanAddData(nPlayerID, nID))
		return false;

	PrincessRef pData = CreateData(nPlayerID, nID);
	if (!pData)
		return false;

	pData->Support = nSupport;
	stBuff buff;
	CaculatePrincessBuff(nPlayerID, buff, pData);
	PLAYERINFOMGR->AddBuff(nPlayerID, 2, buff);
	CaculatePrincessStats(pData);
	Parent::AddData(nPlayerID, pData->UID, pData);
	InsertDB(nPlayerID, pData);
	if(bNotify)
		UpdateToClient(nPlayerID, pData);
	SetTimesInfo(nPlayerID, COUNT_TIMER1, 5, 1);
	SUBMIT(nPlayerID, 25, 1);
	return true;
}

void CPrincessMgr::DeleteData(U32 nPlayerID, std::string nID)
{
	auto& UVec = m_TPlayerMap[nPlayerID].UVec;
	auto itr = std::find(UVec.begin(), UVec.end(), nID);
	if (itr != UVec.end())
	{
		UVec.erase(itr);
	}
	Parent::DeleteData(nPlayerID, nID);
}

bool CPrincessMgr::FindPrincessIdsBySupport(U32 nSupport, std::vector<U32>& UIDs)
{
	return g_PrincessDataMgr->GetUIDsByHireVal(nSupport, UIDs);
}

S32 CPrincessMgr::GetPrincessNum(U32 nPlayerID) {
	auto itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end()) {
		return itr->second.size();
	}
	return 0;
}

void CPrincessMgr::RemovePlayer(U32 nPlayerID)
{
	if (!m_TPlayerMap[nPlayerID].selUID.empty())
		ChoosePrincess(nPlayerID, 0);

	m_TPlayerMap.erase(nPlayerID);
	Parent::RemovePlayer(nPlayerID);
}

enOpError CPrincessMgr::TurnCard(U32 nPlayerID)
{
	if (!m_TPlayerMap[nPlayerID].InitFlag && m_TPlayerMap[nPlayerID].UVec.empty())
	{
		DATA_MAP& DataMap = m_PlayerMap[nPlayerID];
		Vector<std::string> DataVec;
		for (auto itr : DataMap)
		{
			DataVec.push_back(itr.second->UID);
		}

		if (DataVec.empty())
			return OPERATOR_PRINCESS_EMPTY;

		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, TURN_CARD_TIMER);
		if (pTime && pTime->Flag1 <= 0)
			return OPERATOR_PRINCESS_HASNO_VIM;

		m_TPlayerMap[nPlayerID].InitFlag = true;
		for (auto i = 0; (i < 6) && !DataVec.empty(); ++i)
		{
			S32 iRanVal = 0;
			if (DataVec.size() > 1)
				iRanVal = gRandGen.randI(0, DataVec.size() - 1);

			m_TPlayerMap[nPlayerID].UVec.push_back(DataVec[iRanVal]);
			DataVec.erase(iRanVal);
		}
	}

	S32 iRanVal = 0;
	if (m_TPlayerMap[nPlayerID].UVec.size() > 1)
		iRanVal = gRandGen.randI(0, m_TPlayerMap[nPlayerID].UVec.size()-1);
	m_TPlayerMap[nPlayerID].selUID = m_TPlayerMap[nPlayerID].UVec[iRanVal];
	m_TPlayerMap[nPlayerID].UVec.erase(iRanVal);
	SUBMIT(nPlayerID, 5, 1);
	SUBMIT(nPlayerID, 52, 1);
	SUBMITACTIVITY(nPlayerID, ACTIVITY_TURNCARD, 1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrincessMgr::ChoosePrincess(U32 nPlayerID, U32 nType)
{
	if (nType == 0)//同意
	{
		PrincessRef pData = GetData(nPlayerID, m_TPlayerMap[nPlayerID].selUID);
		if (!pData)
			return OPERATOR_HASHNO_PRINCESS;

		m_TPlayerMap[nPlayerID].Clear();

		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, TURN_CARD_TIMER);
		if (!pTime || pTime->Flag1 < 0)
		{
			return OPERATOR_PRINCESS_HASNO_VIM;
		}

		S32 ExpireTime = -1;
		if (pTime->Flag1 == 5 + VIPMGR->GetVipVIMTopLimit(nPlayerID)) {
			ExpireTime = TRUNCARDCD * (1.f + VIPMGR->GetVipVSAReducePercent(nPlayerID) / 100.f);
		}
		TIMESETMGR->AddTimeSet(nPlayerID, TURN_CARD_TIMER, ExpireTime, "", --pTime->Flag1);
		AddPrincessAbility(pData, 0);
		CaculatePrcinessBattleVal(pData);
		UpdateDB(nPlayerID, pData);
		UpdateToClient(nPlayerID, pData);

		//生皇子
		S32 nRandVal = 0;
		//第一次操作必生皇子
		if (!NGUIDE->IsFirstChoosePrincess(nPlayerID)) 
		{
			nRandVal = gRandGen.randI(1, 100);
		}
		
		if (nRandVal < 20)
		{
			PRINCEMGR->AddData(nPlayerID, pData->UID);
		}

		return OPERATOR_NONE_ERROR;
	}
	else
	{
		if (m_TPlayerMap[nPlayerID].UVec.empty())
			return OPERATOR_PRINCESS_TURN_CARD_FAILED;

		S32 iCostGold = (m_TPlayerMap[nPlayerID].chooseTime + 1) * 5;
		if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -iCostGold))
			return OPERATOR_HASNO_GOLD;

		SERVER->GetPlayerManager()->AddGold(nPlayerID, -iCostGold, LOG_GOLD_PRINCESS);
		++m_TPlayerMap[nPlayerID].chooseTime;
		return OPERATOR_NONE_ERROR;
	}
}

enOpError CPrincessMgr::SupportPrincess(U32 nPlayerID, std::string UID)
{
	PrincessRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCESS;

	S32 iCostGold = (pData->Support + 1) * 5;
	if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -iCostGold))
		return OPERATOR_HASNO_GOLD;

	//生皇子
	S32 nRandVal = gRandGen.randI(1, 100);
	if (nRandVal < 20)
	{
		PRINCEMGR->AddData(nPlayerID, pData->UID);
	}
	//auto iTime = Platform::getNextTime(Platform::INTERVAL_DAY) - _time32(NULL);
	//第一次宠幸免费
	if (!NGUIDE->IsFirstSupportRequest(nPlayerID))
	{
		SERVER->GetPlayerManager()->AddGold(nPlayerID, -iCostGold, LOG_GOLD_PRINCESS);
	}
	++pData->SupportTimes;
	++pData->Support;
	pData->SupportTime = Platform::getNextTime(Platform::INTERVAL_DAY);
	AddPrincessAbility(pData, 1);
	CaculatePrcinessBattleVal(pData);
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	return OPERATOR_NONE_ERROR;
}

void CPrincessMgr::AddPrincessAbility(PrincessRef pData, U32 nType)
{
	S32 iAddVal = 0;
	S32 iRanVal = gRandGen.randI(1, 3);
	if (nType == 1)
		iAddVal = pData->CharmEx + 50;
	else
		iAddVal = mFloor(pData->CharmEx * 0.5) + 20;

	S32 *pAddVal = NULL;
	(iRanVal == 1) ? (pAddVal = &pData->Battle) : ((iRanVal == 2) ? (pAddVal = &pData->Culture) : (pAddVal = &pData->Rite));
	*pAddVal = mClamp(*pAddVal + iAddVal, 0, S32_MAX);
}

enOpError CPrincessMgr::AddPrincessPos(U32 nPlayerID, std::string UID, S32 nPrincessPos, S32 nPrePrincessPos/*=0*/)
{
	PrincessRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCESS;

	if (nPrincessPos < 0 || nPrincessPos > 8)
		return OPERATOR_PARAMETER_ERROR;

	if (pData->Rite < s_PrincessPos[nPrincessPos][1])
		return OPERATOR_HASNO_RITE;

	S32 nPrincessNum = GetPrincessPosNum(m_PlayerMap[nPlayerID], nPrincessPos);
	if (nPrincessNum >= s_PrincessPos[nPrincessPos][0])
		return OPERATOR_PRINCESSPOS_LIMIT;

	nPrePrincessPos = pData->PrincessPos;
	pData->PrincessPos = nPrincessPos;
	CaculatePrcinessBattleVal(pData);
	CaculatePrincessStats(pData);
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	SUBMIT(nPlayerID, 37, pData->PrincessPos);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrincessMgr::RemovePrincessPos(U32 nPlayerID, std::string UID)
{
	PrincessRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCESS;

	if (SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -20))
		return OPERATOR_HASNO_GOLD;

	SERVER->GetPlayerManager()->AddGold(nPlayerID, -20, LOG_GOLD_PRINCESS);

	pData->PrincessPos = 0;
	CaculatePrcinessBattleVal(pData);
	CaculatePrincessStats(pData);
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrincessMgr::DispatchPrincess(U32 nPlayerID, std::string UID, U32 nType)
{
	S32 curTime = (S32)_time32(0);
	S32 iAddVal = 0, iDispatchTime = 0;
	PrincessRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCESS;

	if (nType < 0 || nType > 4)
		return OPERATOR_PARAMETER_ERROR;

	for (auto i = 0 + DISPATCH_PRINCESS_TIMER1; i <= DISPATCH_PRINCESS_TIMER4; ++i)
	{
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, i);
		if (pTime)
		{
			//ID重复
			if (pTime->Flag == UID)
			{
				return OPERATOR_DISPATCHPRINCESS_FAILED;
			}
		}
	}

	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, DISPATCH_PRINCESS_TIMER1 + nType);
	if (pTime)
		OPERATOR_DISPATCHPRINCESS_FAILED;

	if (pData->CharmEx < s_PrincessDispatch[nType][3])
		return OPERATOR_HASNO_CHARM;


	if (TIMESETMGR->AddTimeSet(nPlayerID, nType + DISPATCH_PRINCESS_TIMER1, 0, UID, curTime + s_PrincessDispatch[nType][1]))
	{
		SUBMIT(nPlayerID, 8, 1);
		SUBMITACTIVITY(nPlayerID, ACTIVITY_DISPATCH, 1);
		return OPERATOR_NONE_ERROR;
	}

	return OPERATOR_DISPATCHPRINCESS_FAILED;
}

enOpError CPrincessMgr::SKillLvUpPrincess(U32 nPlayerID, std::string UID)
{
	PrincessRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCESS;
	
	for (int i = 0; i < 3; ++i)
	{
		S32 nSkillID = pData->Skill[i] + 1;
		//判断条件
		CSkillData* pSkillData = g_SkillDataMgr->getData(nSkillID);
		if (!pSkillData)
			continue;

		if (pSkillData->m_LevelUpItem[0] == 2001)
		{
			if (pData->Rite < pSkillData->m_CostItemNum[0])
				continue;
		}
		else if (pSkillData->m_LevelUpItem[0] == 2002)
		{
			if (pData->Battle < pSkillData->m_CostItemNum[0])
				continue;
		}
		else if (pSkillData->m_LevelUpItem[0] == 2003)
		{
			if (pData->Culture < pSkillData->m_CostItemNum[0])
				continue;
		}

		Stats preStats;
		CaculateStatsByID(nPlayerID, preStats, pData->Skill[i]);
		Stats aftStats;
		CaculateStatsByID(nPlayerID, aftStats, nSkillID);
		aftStats -= preStats;
		BUFFMGR->AddStatus(nPlayerID, aftStats);
		AddPrinceAbility(nPlayerID, UID, aftStats.PrinceAbi_gPc);
		pData->Skill[i] = nSkillID;
		CaculatePrincessStats(pData);
		stBuff buff;
		CaculateBuffByID(nPlayerID, buff, nSkillID);
		PLAYERINFOMGR->AddBuff(nPlayerID, 2, buff);
		UpdateDB(nPlayerID, pData);
		SUBMIT(nPlayerID, 38, nSkillID % 100);
		//UpdateToClient(nPlayerID, pData);
	}
	
	return OPERATOR_NONE_ERROR;
}


enOpError CPrincessMgr::LearnRitePrincesss(U32 nPlayerID, std::string CUID, std::string UID, S32 nPos)
{
	if (nPos < 0 || nPos > 3)
		return OPERATOR_PARAMETER_ERROR;

	CardRef pCard = CARDMGR->GetData(nPlayerID, CUID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	PrincessRef pData = PRINCESSMGR->GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCESS;

	PlayerBaseInfoRef pBase = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pBase)
		return OPERATOR_PRINCESS_LEARN_RITE_FAILED;

	S32 curTime = (S32)_time32(0);
	S32 nMaxSlot = mClamp((S32)pBase->PLearnNum, 0, 3);
	S32 nSlot = -1, iCostGold = 0;
	TimeSetRef pCTime = TIMESETMGR->GetData(nPlayerID, LEARN_RITE_TIMER1);
	if(pCTime)
	{
		if (pCTime->Flag != CUID)
			return OPERATOR_PRINCESS_LEARN_RITE_FAILED;
	}

	for (auto i = 0 + LEARN_RITE_TIMER2; i <= LEARN_RITE_TIMER5; ++i)
	{
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, i);
		if (pTime)
		{
			//ID重复
			if (pTime->Flag == UID)
			{
				return OPERATOR_PRINCESS_LEARN_RITE_FAILED;
			}
		}
	}

	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, nPos + LEARN_RITE_TIMER2);
	if (pTime)
	{
		return OPERATOR_PRINCESS_LEARN_RITE_FAILED;
	}

	TIMESETMGR->AddTimeSet(nPlayerID, nPos + LEARN_RITE_TIMER2, 0, UID, curTime + 4 * 60 * 60);
	TIMESETMGR->AddTimeSet(nPlayerID, LEARN_RITE_TIMER1, 0, CUID, 0);

	SUBMIT(nPlayerID, 7, 1);
	SUBMITACTIVITY(nPlayerID, ACTIVITY_LEARNRITE, 1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrincessMgr::AwardItemPrincess(U32 nPlayerID, std::string UID, U32 nItemID)
{
	PrincessRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCESS;

	if (nItemID != 600065 && nItemID != 600066 && nItemID != 600067 && nItemID != 600068)
		return OPERATOR_PARAMETER_ERROR;

	if (!ITEMMGR->CanReduceItem(nPlayerID, nItemID, -1))
		return OPERATOR_HASNO_ITEM;

	S32 nAddVal = (nItemID == 600065) ? 2 : ((nItemID == 600066) ? 1 : ((nItemID == 600067) ? 2 : 1));
	if (nItemID == 600065 || nItemID == 600066)
		pData->Support = mClamp((S32)pData->Support + nAddVal, (S32)0, (S32)S32_MAX);
	else
		pData->Charm = mClamp((S32)pData->Charm + nAddVal, (S32)0, (S32)S32_MAX);

	CaculatePrcinessBattleVal(pData);
	CaculatePrincessStats(pData);
	ITEMMGR->AddItem(nPlayerID, nItemID, -1);
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrincessMgr::PlantFlowerPrincess(U32 nPlayerID, std::string UID, S32 nFlowerID, S32 nPos)
{
	if (nPos < 0 || nPos > 3)
		return OPERATOR_PARAMETER_ERROR;

	CFlowerData* pFData = g_FlowerDataMgr->getData(nFlowerID);
	if (!pFData)
		return OPERATOR_PARAMETER_ERROR;

	PrincessRef pData = PRINCESSMGR->GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCESS;

	PlayerBaseInfoRef pBase = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pBase)
		return OPERATOR_PRINCESS_PLANT_FAILED;

	if (pData->CharmEx < pFData->m_Charm)
		return OPERATOR_HASNO_CHARM;

	S32 curTime = (S32)_time32(0);
	S32 nMaxSlot = mClamp((S32)pBase->PPlantNum, 0, 3);

	for (auto i = 0 + PLANT_FLOWER_TIMER1; i <= PLANT_FLOWER_TIMER4; ++i)
	{
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, i);
		if (pTime)
		{
			//ID重复
			if (pTime->Flag == UID)
			{
				return OPERATOR_PRINCESS_PLANT_FAILED;
			}
		}
	}

	{
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, nPos + PLANT_FLOWER_TIMER1);
		if (pTime)
		{
			return OPERATOR_PRINCESS_PLANT_FAILED;
		}
	}

	TIMESETMGR->AddTimeSet(nPlayerID, nPos + PLANT_FLOWER_TIMER1, 0, UID, curTime + pFData->m_FlowerTime * 60);
	S32 nFlag1 = 0;
	{
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PLANT_FLOWER_TIMER5);
		if (pTime)
		{
			nFlag1 = pTime->Flag1;
		}
	}
	nFlag1 = (nFlag1 & ~(0x000000FF*BIT(nPos * 8))) | ((nFlowerID % 100) * BIT(nPos * 8));
	TIMESETMGR->AddTimeSet(nPlayerID, PLANT_FLOWER_TIMER5, 0, "", nFlag1);
	SUBMIT(nPlayerID, 9, 1);
	SUBMITACTIVITY(nPlayerID, ACTIVITY_PLANT, 1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrincessMgr::FinishPrincess(U32 nPlayerID, S32 nType, S32 nSlot, S32& nPreVal, S32& nAftVal)
{
	S32 nTimeID = 0;
	if (nType < 0 || nType > 3)
		return OPERATOR_PARAMETER_ERROR;

	//交泰殿
	if (nType == 0)
	{
		if (nSlot < 0 || nSlot > 4)
			return OPERATOR_PARAMETER_ERROR;

		nTimeID = DISPATCH_PRINCESS_TIMER1 + nSlot;
	}
	else if (nType == 1)
	{
		if (nSlot < 0 || nSlot > 4)
			return OPERATOR_PARAMETER_ERROR;

		nTimeID = LEARN_RITE_TIMER2 + nSlot;
	}
	else if (nType == 2)
	{
		if (nSlot < 0 || nSlot > 4)
			return OPERATOR_PARAMETER_ERROR;

		nTimeID = PLANT_FLOWER_TIMER1 + nSlot;
	}

	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, nTimeID);
	if (!pTime)
		return OPERATOR_PRINCESS_FINISH_FAILED;

	S32 curTime = (S32)_time32(0);
	if (pTime->Flag1 > curTime)
		return OPERATOR_PRINCESS_FINISH_FAILED;

	if (nType == 0)
	{
		PrincessRef pData = GetData(nPlayerID, pTime->Flag);
		if (!pData)
			return OPERATOR_HASHNO_PRINCESS;

		S32 iAddVal = s_PrincessDispatch[nSlot][2];
		nPreVal = pData->Battle;
		g_Stats.Clear();
		CaculateStats(nPlayerID, pTime->Flag, g_Stats);
		iAddVal = iAddVal * (1.0f + (g_Stats.PBattle_Pc) / 100.0f);
		//pData->Battle += s_PrincessDispatch[nSlot][2];
		pData->Battle = mClamp((S32)pData->Battle + iAddVal, (S32)0, (S32)S32_MAX);
		nAftVal = pData->Battle;
		if (s_PrincessDispatch[nSlot][4] > 0 && s_PrincessDispatch[nSlot][5] > 0)
			ITEMMGR->AddItem(nPlayerID, s_PrincessDispatch[nSlot][4], s_PrincessDispatch[nSlot][5]);

		//随机掉落
		S32 nRate = gRandGen.randI(1, 100);
		if (nRate <= s_PrincessDispatch[nSlot][7])
		{
			ITEMMGR->AddItem(nPlayerID, s_PrincessDispatch[nSlot][6], 1);
		}


		CaculatePrcinessBattleVal(pData);
		UpdateDB(nPlayerID, pData);
		UpdateToClient(nPlayerID, pData);
	}
	else if (nType == 1)
	{
		g_Stats.Clear();
		CardRef pCard;
		{
			TimeSetRef pCTime = TIMESETMGR->GetData(nPlayerID, LEARN_RITE_TIMER1);
			if (pCTime)
			{
				pCard = CARDMGR->GetData(nPlayerID, pCTime->Flag);
			}
			CARDMGR->CaculateCardStats(nPlayerID, pCTime->Flag, g_Stats);
		}

		PrincessRef pData = GetData(nPlayerID, pTime->Flag);
		if (!pData)
			return OPERATOR_HASHNO_PRINCESS;

		S32 nTimeNum = 0;
		S32 nBaseVal = (pCard) ? (pCard->Int) : 100;
		S32 iAddVal = mFloor(200 * (1 + (0) + (nBaseVal) / (nBaseVal + 100000.0f) + (0) + (0)));
		StatsRef pStats = BUFFMGR->GetData(nPlayerID);
		CaculateStats(nPlayerID, pTime->Flag, g_Stats);
		iAddVal = iAddVal * (1.0f + (g_Stats.Rite_Pc + (pStats ? pStats->Rite_gPc + pStats->Queen_gPc + g_Stats.Queen_gPc : 0)) / 100.0f);
		nPreVal = pData->Rite;
		pData->Rite = mClamp((S32)pData->Rite + iAddVal, (S32)0, (S32)S32_MAX);
		nAftVal = pData->Rite;
		CaculatePrcinessBattleVal(pData);
		UpdateDB(nPlayerID, pData);
		UpdateToClient(nPlayerID, pData);

		ITEMMGR->AddItem(nPlayerID, 600002, 4);
		ITEMMGR->AddItem(nPlayerID, 600096, 1);

		for (auto i = 0 + LEARN_RITE_TIMER2; i <= LEARN_RITE_TIMER5; ++i)
		{
			TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, i);
			if (pTime)
			{
				++nTimeNum;
			}
		}

		if(nTimeNum <= 1)
			TIMESETMGR->RemoveTimeSet(nPlayerID, LEARN_RITE_TIMER1);
	}
	else if (nType == 2)
	{
		PrincessRef pData = GetData(nPlayerID, pTime->Flag);
		if (!pData)
			return OPERATOR_HASHNO_PRINCESS;

		S32 nFlag1 = 0;
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PLANT_FLOWER_TIMER5);
		CFlowerData* pFloData = NULL;
		S32 nDataID = 0, nItemNum = gRandGen.randI(2, 3);
		if (pTime)
		{
			nFlag1 = pTime->Flag1;
			nDataID = 600000 + ((nFlag1 >>(nSlot * 8)) & 0x000000FF);
			pFloData = g_FlowerDataMgr->getData(nDataID);
		}
		nFlag1 =  (nFlag1 & ~(0x000000FF * BIT(nSlot * 8)));
		S32 nBaseVal = (pFloData) ? (pFloData->m_FlowerTime) : 120;
		//大成功概率
		g_Stats.Clear();
		CaculateStats(nPlayerID, pTime->Flag, g_Stats);
		StatsRef pStats = BUFFMGR->GetData(nPlayerID);
		S32 nBigSuc = 5 + g_Stats.FlowerSuc_Pc + (pStats ? pStats->FlowerSuc_gPc : 0);
		S32 nAddVal = mFloor(nBaseVal) * (1.0f + (pStats ? pStats->Culture_gPc : 0) / 100.0f);

		S32 nRandVal = gRandGen.randI(0, 100);
		if (nRandVal < nBigSuc)
		{
			nAddVal = mFloor(nAddVal * 1.5);
			nItemNum = gRandGen.randI(5, 6);
		}
		pData->Culture = mClamp((S32)pData->Culture + nAddVal, (S32)0, (S32)S32_MAX);
		CaculatePrcinessBattleVal(pData);
		UpdateDB(nPlayerID, pData);
		UpdateToClient(nPlayerID, pData);
		ITEMMGR->AddItem(nPlayerID, nDataID, nItemNum);

		auto AwardItem =[=](){
			S32 nAwardItemId = 0, nAwardItemNum = 1, nAwardRate = 25;
			if (nDataID >= 600048 && nDataID <= 600051)
			{
				nAwardItemId = 600003; nAwardItemNum = 1; nAwardRate = 25;
			}
			else if (nDataID >= 600052 && nDataID <= 600053)
			{
				nAwardItemId = 600003; nAwardItemNum = 2; nAwardRate = 50;
			}
			else
			{
				nAwardItemId = 600003; nAwardItemNum = 4; nAwardRate = 100;
			}

			ITEMMGR->AddItem(nPlayerID, nAwardItemId, nAwardItemNum);
			//随机掉落
			S32 nRate = gRandGen.randI(1, 100);
			if (nRate <= nAwardRate)
			{
				ITEMMGR->AddItem(nPlayerID, 600096, 1);
			}
		};

		AwardItem();

		if(nFlag1)
			TIMESETMGR->AddTimeSet(nPlayerID, PLANT_FLOWER_TIMER5, 0, "", nFlag1);
		else 
			TIMESETMGR->RemoveTimeSet(nPlayerID, PLANT_FLOWER_TIMER5);
	}

	TIMESETMGR->RemoveTimeSet(nPlayerID, nTimeID);
	return OPERATOR_NONE_ERROR;
}

enOpError CPrincessMgr::AddMaxSlot(U32 nPlayerID, S32 nType)
{
	PlayerBaseInfoRef pBase = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pBase)
		return OPERATOR_PRINCESS_ADDMAXSLOT_FAILED;

	if (nType < 0 || nType > 1)
		return OPERATOR_PARAMETER_ERROR;

	S32 nMaxSlot = 0;
	S32 nCurSlot = 0;
	S32 nCostGold = 0;
	if (nType == 0)
	{
		nMaxSlot = 4;
		nCurSlot = pBase->PLearnNum;
		nCostGold = (nCurSlot) * 100;
	}
	else
	{
		nMaxSlot = 4;
		nCurSlot = pBase->PPlantNum;
		nCostGold = (nCurSlot) * 100;
	}

	if (nCurSlot >= nMaxSlot)
		return OPERATOR_PRINCESS_ADDMAXSLOT_FAILED;


	if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -nCostGold))
		return OPERATOR_HASNO_GOLD;

	SERVER->GetPlayerManager()->AddGold(nPlayerID, -nCostGold, LOG_GOLD_PRINCESS);
	if (nType == 0)
	{
		PLAYERINFOMGR->AddPLearnNum(nPlayerID, 1);
	}
	else
	{
		PLAYERINFOMGR->AddPPlantNum(nPlayerID, 1);
	}

	return OPERATOR_NONE_ERROR;
}

enOpError CPrincessMgr::AddPrinceAbility(U32 nPlayerID, std::string UID, S32 nAddVal)
{
	PrincessRef pData = PRINCESSMGR->GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASHNO_PRINCESS;

	pData->PrinceAbility = mClamp((S32)(pData->PrinceAbility + nAddVal), (S32)0, (S32)S8_MAX);
	UpdateDB(nPlayerID, pData);
	//UpdateToClient(nPlayerID, pData);
	return OPERATOR_NONE_ERROR;
}

void CPrincessMgr::UpdatePrincess(U32 nPlayerID)
{
	DATA_MAP& DataMap = m_PlayerMap[nPlayerID];
	for (DATA_ITR itr = DataMap.begin(); itr != DataMap.end(); ++itr)
	{
		PrincessRef& pData = itr->second;
		if (pData && pData->SupportTimes)
		{
			pData->SupportTime = 0;
			pData->SupportTimes = 0;
			UpdatePrincessTime(nPlayerID, pData);
			UpdateToClient(nPlayerID, pData);
		}
	}
}

void CPrincessMgr::UpdateToClient(U32 nPlayerID, PrincessRef pData)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_PRINCESS_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CPrincessMgr::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		DATA_MAP& DataMap = m_PlayerMap[nPlayerID];
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_PRINCESS_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
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

auto GetPrincessDB = [](auto pHandle) {
	PrincessRef pData = PrincessRef(new stPrincess);
	pData->UID = pHandle->GetString();
	pData->PlayerID = pHandle->GetInt();
	pData->PrincessID = pHandle->GetInt();
	pData->Support = pHandle->GetInt();
	pData->SupportTime = pHandle->GetInt();
	pData->SupportTimes = pHandle->GetInt();
	pData->Charm = pHandle->GetInt();
	pData->Rite = pHandle->GetInt();
	pData->Culture = pHandle->GetInt();
	pData->Battle = pHandle->GetInt();
	pData->PrincessPos = pHandle->GetInt();
	for (auto i = 0; i < 3; ++i)
	{
		pData->Skill[i] = pHandle->GetInt();
	}
	pData->PrinceNum = pHandle->GetInt();
	pData->PrinceAbility = pHandle->GetInt();
	pData->BattleVal = pHandle->GetInt();
	return pData;
};

DB_Execution* CPrincessMgr::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(fmt::sprintf("SELECT UID, PlayerID, PrincessID, Support, SupportTime, SupportTimes, Charm, Rite, Culture, Battle, \
		PrincessPos, Skill0, Skill1, Skill2, PrinceNum, PrinceAbility, BattleVal FROM Tbl_Princess \
		WHERE PlayerID=%d", nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void* pSqlHandle)
	{
		DB_Execution* pHandle = (DB_Execution*)(pSqlHandle);
		if (pHandle && error == NONE_ERROR)
		{
			stBuff buff;
			while (pHandle->More())
			{
				PrincessRef pData = GetPrincessDB(pHandle);
				CaculatePrincessBuff(id, buff, pData);
				Parent::AddData(id, pData->UID, pData);
				CaculatePrincessStats(pData);
				PLAYERINFOMGR->AddQueenVal(id, pData->BattleVal, true);
			}
			PLAYERINFOMGR->AddBuff(id, 2, buff, true);
		}
	}
	);
	return pHandle;
}

PrincessRef CPrincessMgr::SyncLoadDB(U32 nPlayerID, std::string UID)
{
	TBLExecution tbl_handle(SERVER->GetActorDB());
	tbl_handle.SetId(nPlayerID);
	tbl_handle.SetSql(fmt::sprintf("SELECT UID, PlayerID, PrincessID, Support, SupportTime, SupportTimes, Charm, Rite, Culture, Battle, \
		PrincessPos, Skill0, Skill1, Skill2, PrinceNum, PrinceAbility, BattleVal FROM Tbl_Princess \
		WHERE PlayerID=%d AND UID='%s'", nPlayerID, UID.c_str()));
	tbl_handle.RegisterFunction([this](int id, int error, void * pH)
	{
		CDBConn* pHandle = (CDBConn*)(pH);
		if (pHandle && error == NONE_ERROR)
		{
			if (pHandle->More())
			{
				PrincessRef pData = GetPrincessDB(pHandle);
				CaculatePrincessStats(pData);
				return pData;
			}
		}
	}
	);
	tbl_handle.Commint();
	return PrincessRef();
}

bool CPrincessMgr::UpdateDB(U32 nPlayerID, PrincessRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Princess SET \
		Support=%d,				SupportTime=%d,					SupportTimes=%d,					Charm=%d,						Rite=%d,\
		Culture=%d,				Battle=%d,						PrincessPos=%d,						Skill0=%d,						Skill1=%d,\
		Skill2=%d,				PrinceNum=%d,					PrinceAbility=%d,					BattleVal=%d\
		WHERE UID='%s' AND		PlayerID=%d",\
		pData->Support,			pData->SupportTime,				pData->SupportTimes,				pData->Charm,					pData->Rite,\
		pData->Culture,			pData->Battle,					pData->PrincessPos,					pData->Skill[0],				pData->Skill[1],\
		pData->Skill[2],		pData->PrinceNum,				pData->PrinceAbility,				pData->BattleVal,\
		pData->UID.c_str(),		nPlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPrincessMgr::InsertDB(U32 nPlayerID, PrincessRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("INSERT INTO Tbl_Princess  \
		(Support,				SupportTime,					SupportTimes,						Charm,							Rite,\
		Culture,				Battle,							PrincessPos,						Skill0,							Skill1,\
		Skill2,					PrinceNum,						PrinceAbility,						BattleVal,						PrincessID,\
		UID,					PlayerID)\
	VALUES(%d,					%d,								%d,									%d,								%d,\
		   %d,					%d,								%d,									%d,								%d,\
		   %d,					%d,								%d,									%d,								%d,\
		   '%s',				%d)", \
		pData->Support,			pData->SupportTime,				pData->SupportTimes,				pData->Charm,					pData->Rite, \
		pData->Culture,			pData->Battle,					pData->PrincessPos,					pData->Skill[0],				pData->Skill[1], \
		pData->Skill[2],		pData->PrinceNum,				pData->PrinceAbility,				pData->BattleVal,				pData->PrincessID,\
		pData->UID.c_str(),		nPlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPrincessMgr::DeleteDB(U32 nPlayerID, std::string UID)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE Tbl_Princess  WHERE UID = '%s'", \
		UID.c_str()));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPrincessMgr::UpdatePrincessTime(U32 nPlayerID, PrincessRef pData)
{
	//CRedisDB conn(SERVER->GetRedisPool());
	//HashCmd<stPrincess> cmdRedis(&conn);
	//cmdRedis.hset(pData, nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Princess SET SupportTime=%d, SupportTimes=%d WHERE UID='%s'", \
		pData->SupportTime, pData->SupportTimes, pData->UID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPrincessMgr::HandleClientSupportRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	std::string UID = strUID;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = SupportPrincess(pAccount->GetPlayerId(), UID);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_SUPPORT_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrincessMgr::HandleClientAwardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	U32 nItemID = Packet->readInt(Base::Bit32);
	std::string UID = strUID;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = AwardItemPrincess(pAccount->GetPlayerId(), UID, nItemID);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_AWARD_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrincessMgr::HandleClientTurnCardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = TurnCard(pAccount->GetPlayerId());
		stTurnCard& tCard = m_TPlayerMap[pAccount->GetPlayerId()];
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_TURNCARD_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeString(tCard.selUID, MAX_UUID_LENGTH);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrincessMgr::HandleClientChooseRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = ChoosePrincess(pAccount->GetPlayerId(), nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CHOOSE_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrincessMgr::HandleClientBestowedRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	S32 nPos = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		S32 nPrePos = 0;
		U32 nError = AddPrincessPos(pAccount->GetPlayerId(), UID, nPos, nPrePos);
		CMemGuard Buffer(256 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 256);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_BESTOWED_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nPos, Base::Bit8);
		sendPacket.writeInt(nPrePos, Base::Bit8);
		sendPacket.writeString(UID, MAX_UUID_LENGTH);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrincessMgr::HandleClientDispatchRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	S32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = DispatchPrincess(pAccount->GetPlayerId(), UID, nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_DISPATCH_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrincessMgr::HandleClientLearnRiteRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string CUID = Packet->readString(MAX_UUID_LENGTH);
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	S32 nPos = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = LearnRitePrincesss(pAccount->GetPlayerId(), CUID, UID, nPos);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_LEARNRITE_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrincessMgr::HandleClientPlantFlowerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	S32 nFlowerID = Packet->readInt(Base::Bit32);
	S32 nPos = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = PlantFlowerPrincess(pAccount->GetPlayerId(), strUID, nFlowerID, nPos);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PLANTFLOWER_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrincessMgr::HandleClientFinishRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	S32 nType = Packet->readInt(Base::Bit8);
	S32 nSlot = Packet->readInt(Base::Bit8);
	S32 nPreVal = 0, nAftVal = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = FinishPrincess(pAccount->GetPlayerId(), nType, nSlot, nPreVal, nAftVal);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_FINISH_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nType,   Base::Bit8);
		sendPacket.writeInt(nPreVal, Base::Bit32);
		sendPacket.writeInt(nAftVal, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPrincessMgr::HandleClientAddMaxSlotRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	S32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = AddMaxSlot(pAccount->GetPlayerId(), nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ADDMAXSLOT_PRINCESS_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nType, Base::Bit8);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

void ___AddPrincess(int PlayerID, int PrincessID)
{
	PRINCESSMGR->AddData(PlayerID, PrincessID);
}

void CPrincessMgr::Export(struct lua_State* L)
{
	lua_tinker::def(L, "AddPrincess", &___AddPrincess);
}