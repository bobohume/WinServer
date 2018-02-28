#include "CardMgr.h"
#include "Common/UUId.h"
#include "Common/mRandom.h"
#include "Common/MemGuard.h"
#include "WINTCP/dtServerSocket.h"
#include "DBLayer/Common/Define.h"
#include "../WorldServer.h"
#include "../PlayerMgr.h"
#include "../Database/DB_Execution.h"
#include "DBLayer/Data/TBLExecution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "PlayerInfoMgr.h"
#include "ItemMgr.h"
#include "TimeSetMgr.h"
#include "BuffMgr.h"
#include "CardData.h"
#include "SkillData.h"
#include "BuffData.h"
#include "ItemData.h"
#include "CopyData.h"
#include "TaskMgr.h"
#include "TopMgr.h"
#include "VIPMgr.h"
#include "PrincessMgr.h"

struct stCardSimple
{
	std::string UID;
	S32 Level;
};

typedef		std::unordered_map<S32, stCardSimple>	CARDSIMPLEMAP;
const int CCardManager::s_AddAbilityVal[MAX_ADD_SLOT] =
{
	100,
	200,
	500,
	1000
};

const int CCardManager::s_CostGrowItemID[3] =
{
	600005,
	600006,
	600007,
};

const int CCardManager::s_AddAbilityConfig[12][3] = 
{
	//  -道具-		-品质-		-增加值-
	{ 600011,			1,			16 },
	{ 600010,			2,			24 },
	{ 600009,			3,			40 },
	{ 600008,			4,			60 },

	{ 600015,			1,			16 },
	{ 600014,			2,			24 },
	{ 600013,			3,			40 },
	{ 600012,			4,			60 },

	{ 600019,			1,			16 },
	{ 600018,			2,			24 },
	{ 600017,			3,			40 },
	{ 600016,			4,			60 },
};

const int CCardManager::s_OfficalRankConfig[10][2] =
{
	//  -消耗功勋-		-能力加成-
	{ 0,				0 },
	{ 2000,				1000 },
	{ 6000,				3000 },
	{ 10000,			5000 },
	{ 20000,			10000 },
	{ 30000,			15000 },
	{ 40000,			20000 },
	{ 60000,			30000 },
	{ 80000,			40000 },
	{ 100000,			50000 },
};

//DECLARE_REDIS_UNIT(stCard);
DECLARE_REDIS_UNIT(stFate);
CCardManager::CCardManager()
{
	g_CardDataMgr->read();
	g_ExpDataMgr->read();
	g_SkillDataMgr->read();
	g_BuffDataMgr->read();
	g_CopyDataMgr->read();
	g_ApplyDataMgr->read();
	g_LvLimitDataMgr->read();
	g_FateDataMgr->read();
	REGISTER_EVENT_METHOD("CW_CARD_ADDLEVEL_REQUEST",	this, &CCardManager::HandleAddCardLevelRequest);
	REGISTER_EVENT_METHOD("CW_CARD_UPLVLIMIT_REQUEST",	this, &CCardManager::HandleAddCardLevelLimitRequest);
	REGISTER_EVENT_METHOD("CW_CARD_ADDSKILL_REQUEST",	this, &CCardManager::HandleAddCardSkillRequest);
	REGISTER_EVENT_METHOD("CW_CARD_ADDEQUIP_REQUEST",	this, &CCardManager::HandleAddCardEquipRequest);
	REGISTER_EVENT_METHOD("CW_CARD_REMOVEEQUIP_REQUEST",this, &CCardManager::HandleRemoveCardEquipRequest);
	REGISTER_EVENT_METHOD("CW_CARD_ADDABILITY_REQUEST", this, &CCardManager::HandleAddCardAbilityRequest);
	REGISTER_EVENT_METHOD("CW_CARD_ADDGROW_REQUEST",	this, &CCardManager::HandleAddCardGrowRequest);
	REGISTER_EVENT_METHOD("CW_CARD_HIRE_REQUEST",		this, &CCardManager::HandleHireCardRequest);
	REGISTER_EVENT_METHOD("CW_CARD_DISPATCH_REQUEST",	this, &CCardManager::HandleDispatchCardRequest);
	REGISTER_EVENT_METHOD("CW_READ_APPLY_REQUEST",		this, &CCardManager::HandleReadApplyRequest);
	REGISTER_EVENT_METHOD("CW_ADD_CARD_OFFICALRANK_REQUEST", this, &CCardManager::HandleAddCardOfficalRankRequest);
	REGISTER_EVENT_METHOD("CW_DISPATCH_CARD_REQUEST", this, &CCardManager::HandleDispatchCardAwardRequest);

	/*GET_REDIS_UNIT(stCard).SetName("Tbl_Card");
	REGISTER_REDIS_UNITKEY(stCard, PlayerID);
	REGISTER_REDIS_UNITKEY(stCard, UID);
	REGISTER_REDIS_UNIT(stCard, CardID);
	REGISTER_REDIS_UNIT(stCard, Level);
	REGISTER_REDIS_UNIT(stCard, Achievement);
	REGISTER_REDIS_UNIT(stCard, OfficialRank);
	REGISTER_REDIS_UNIT(stCard, OfficialPos);
	REGISTER_REDIS_UNIT(stCard, CardMask);
	REGISTER_REDIS_UNIT(stCard, Agi);
	REGISTER_REDIS_UNIT(stCard, Int);
	REGISTER_REDIS_UNIT(stCard, Str);
	REGISTER_REDIS_UNIT(stCard, Agi_Grow);
	REGISTER_REDIS_UNIT(stCard, Int_Grow);
	REGISTER_REDIS_UNIT(stCard, Str_Grow);
	REGISTER_REDIS_UNIT(stCard, Equip);
	REGISTER_REDIS_UNIT(stCard, Skill);
	REGISTER_REDIS_UNIT(stCard, Agi_Add);
	REGISTER_REDIS_UNIT(stCard, Int_Add);
	REGISTER_REDIS_UNIT(stCard, Str_Add);
	REGISTER_REDIS_UNIT(stCard, BattleVal);*/

	GET_REDIS_UNIT(stFate).SetName("Fate_Map");
	REGISTER_REDIS_UNITKEY(stFate, PlayerID);
	REGISTER_REDIS_UNITKEY(stFate, FateID);
	REGISTER_REDIS_UNIT(stFate, Level);
	REGISTER_REDIS_UNIT(stFate, Val);
	REGISTER_REDIS_UNIT(stFate, UID);
}

CCardManager::~CCardManager()
{
	g_CardDataMgr->clear();
	g_ExpDataMgr->clear();
	g_SkillDataMgr->clear();
	g_BuffDataMgr->clear();
	g_CopyDataMgr->clear();
	g_ApplyDataMgr->clear();
	g_LvLimitDataMgr->clear();
	g_FateDataMgr->clear();
}

CCardManager* CCardManager::Instance()
{
	static CCardManager s_Mgr;
	return &s_Mgr;
}

auto CaculateBuff = [](auto& buff, auto& pCard)
{
	buff.Business += pCard->Agi;
	buff.Military += pCard->Str;
	buff.Technology += pCard->Int;
};

auto CaculateBattleVal = [](auto pCard)
{
	if (!pCard)
		return false;

	S32 nBattleVal = 0;
	for (int j = 0; j < MAX_SKILL_SLOT; ++j)
	{
		if (pCard->Skill[j] > 0)
		{
			CSkillData* pSkillData = g_SkillDataMgr->getData(pCard->Skill[j]);
			if (pSkillData)
			{
				nBattleVal += pSkillData->m_BattleVal;
			}
		}
	}

	nBattleVal = mClamp((S32)(pCard->Agi + pCard->Int + pCard->Str + nBattleVal), (S32)0, (S32)S32_MAX);
	pCard->BattleVal = nBattleVal;
	TOPMGR->NewInData(eTopType_Card, pCard->PlayerID, pCard->BattleVal, pCard->CardID);
};

bool CCardManager::CanAddCard(U32 nPlayerID, U32 nCardID)
{
	//已经拥有不能重复添加
	PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		DATA_MAP& CardMap = itr->second;
		for (DATA_ITR itr = CardMap.begin(); itr != CardMap.end(); ++itr)
		{
			if (itr->second && itr->second->CardID == nCardID)
				return false;
		}
	}

	return true;
}

bool CCardManager::AddCard(U32 nPlayerID, U32 nCardID, bool bNotify/*=true*/)
{
	if (!CanAddCard(nPlayerID, nCardID))
		return false;

	CardRef pCard = CreateCard(nPlayerID, nCardID);
	if (!pCard)
		return false;

	stBuff buff;
	CaculateBuff(buff, pCard);
	PLAYERINFOMGR->AddBuff(nPlayerID, 0, buff);
	AddData(nPlayerID, pCard->UID, pCard);
	InsertDB(nPlayerID, pCard);
	if(bNotify)
		UpdateToClient(nPlayerID, pCard);
	SetTimesInfo(nPlayerID, COUNT_TIMER1, 4, 1);
	SUBMIT(nPlayerID, 24, 1);
	return true;
}

bool CCardManager::RemoveCard(U32 nPlayerID, std::string UID)
{
	DeleteData(nPlayerID, UID);
	DeleteDB(nPlayerID, UID);
	SetTimesInfo(nPlayerID, COUNT_TIMER1, 4, -1);
	return true;
}

void CCardManager::RemovePlayer(U32 nPlayerID)
{
	PLAYER_ITR  itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		DATA_MAP& PlayerMap = itr->second;
		PlayerMap.clear();
		m_PlayerMap.erase(itr);
	}
	RemoveFatePlayer(nPlayerID);
}

CardRef CCardManager::CreateCard(U32 nPlayerID, U32 nCardID)
{
	CCardData* pData = g_CardDataMgr->getData(nCardID);
	if (!pData)
		return NULL;
	
	DBError err = DBERR_UNKNOWERR;
	try
	{
		CardRef pCard = CardRef(new stCard);
		pCard->PlayerID = nPlayerID;
		pCard->CardID = nCardID;
		pCard->UID = uuid();
		pCard->Agi_Grow = pData->m_InitAgiGrow;
		pCard->Int_Grow = pData->m_InitIntGrow;
		pCard->Str_Grow = pData->m_InitStrGrow;
		pCard->Agi = pData->m_InitAgi;
		pCard->Int = pData->m_InitInt;
		pCard->Str = pData->m_InitStr;
		pCard->Level = 1;

		for (int i = 0; i < MAX_INIT_SKILL; ++i)
		{
			if (pData->m_SkillID[i])
				pCard->Skill[i] = pData->m_SkillID[i];
		}

		return pCard;
	}
	DBECATCH()

	return NULL;
}

enOpError CCardManager::DispatchCard(U32 nPlayerID, std::string UID, U32 nType)
{
#define MAX_DISPATCH_NUM 7
#define DISPATCH_TIME_INTERVAL (28800+300)
	S32 iDispatchTime = 0;
	S32 iAddVal = 0;
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	if (nType < 0 || nType > 3)
		return OPERATOR_PARAMETER_ERROR;

	S32 nSlot = -1;
	StatsRef pStats = BUFFMGR->GetData(nPlayerID);
	S32 nMaxSlot = 2 + (pStats ? pStats->DevNum : 0);
	for (int i = CARD_DEVELOP_TIMER1; i <= CARD_DEVELOP_TIMER1 + nMaxSlot; ++i)
	{
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, i);
		if (!pTime)
		{
			nSlot = i;
			break;
		}
		else
		{
			if (pTime->Flag == UID)
			{
				nSlot = -1;
				break;
			}
		}
	}

	if (nSlot == -1)
		return OPERATOR_DISPATCHCARD_FAILED;

	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pPlayer)
		return OPERATOR_DISPATCHCARD_FAILED;

	g_Stats.Clear();
	CaculateCardStats(nPlayerID, UID, g_Stats);
	//计算技能影响的时间
	if (nType == 0)
		iDispatchTime = ((pPlayer->Business * 1.0f) / (pPlayer->Business + 1000000)) * 28800 + 300;
	else if (nType == 1)
		iDispatchTime = ((pPlayer->Technology * 1.0f) / (pPlayer->Technology + 1000000)) * 28800 + 300;
	else
		iDispatchTime = ((pPlayer->Military * 1.0f) / (pPlayer->Military + 1000000)) * 28800 + 300;;

	iDispatchTime = mClamp(iDispatchTime*(1 + (g_Stats.DevTime_Pc / 100.f)), 1, DISPATCH_TIME_INTERVAL);
	if (TIMESETMGR->AddTimeSet(nPlayerID, nSlot, iDispatchTime + 12*30*24*60*60, UID, nType))
	{
		/*S32 iBaseVal = (nType == 0) ? (pPlayer->Business) : ((nType == 1) ? (pPlayer->Technology) : pPlayer->Military);
		iAddVal = mFloor((pCard->Agi * 0.05) * (1 + \
			((nType == 0) ? ((g_Stats.Bus_Pc + (pStats ? pStats->Bus_gPc : 0)) / 100.f) : ((nType == 1) ? ((g_Stats.Tec_Pc + (pStats ? pStats->Tec_gPc : 0)) / 100.f) : (((g_Stats.Mil_Pc + +(pStats ? pStats->Mil_gPc : 0))/ 100.f))))) \
			* (2 - (iBaseVal * 1.0f) / (iBaseVal + 100000)));
		if (nType == 0)
			PLAYERINFOMGR->AddBusiness(nPlayerID, iAddVal);
		else if (nType == 1)
			PLAYERINFOMGR->AddTechnology(nPlayerID, iAddVal);
		else
			PLAYERINFOMGR->AddMilitary(nPlayerID, iAddVal);*/
		SetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 4, 1);
		SUBMIT(nPlayerID, 1, 1);
		SUBMIT(nPlayerID, 57, 1);
		return OPERATOR_NONE_ERROR;
	}

	return OPERATOR_HASNO_ITEM;
}

enOpError CCardManager::DispatchCardAward(U32 nPlayerID, S32 nTimeID, S32& nAddVal, bool& bDouble)
{
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pPlayer)
		return OPERATOR_DISPATCHCARD_FAILED;

	if (nTimeID < CARD_DEVELOP_TIMER1 || nTimeID > CARD_DEVELOP_TIMER6)
		return OPERATOR_DISPATCHCARD_FAILED;

	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, nTimeID);
	if (!pTime)
		return OPERATOR_DISPATCHCARD_FAILED;

	auto nCurTime = (S32)(time(NULL));
	if (pTime->ExpireTime - 12 * 30 * 24 * 60 * 60 > nCurTime)
		return OPERATOR_DISPATCHCARD_FAILED;

	CardRef pCard = GetData(nPlayerID, pTime->Flag);
	if (!pCard)
		return OPERATOR_HASNO_CARD;


	S32 nType = pTime->Flag1;
	S32 iAddVal = 0;
	g_Stats.Clear();
	CaculateCardStats(nPlayerID, pTime->Flag, g_Stats);
	//计算技能影响的时间
	StatsRef pStats = BUFFMGR->GetData(nPlayerID);
	{
		S32 iBaseVal = (nType == 0) ? (pPlayer->Business) : ((nType == 1) ? (pPlayer->Technology) : pPlayer->Military);
		iAddVal = mFloor((pCard->Agi * 0.01) * (1 + \
			((nType == 0) ? ((g_Stats.Bus_Pc + (pStats ? pStats->Bus_gPc : 0)) / 100.f) : ((nType == 1) ? ((g_Stats.Tec_Pc + (pStats ? pStats->Tec_gPc : 0)) / 100.f) : (((g_Stats.Mil_Pc + +(pStats ? pStats->Mil_gPc : 0)) / 100.f))))) \
			* (2 - (iBaseVal * 1.0f) / (iBaseVal + 100000)));

		S32 nRandVal = gRandGen.randI(1, 100);
		if (nRandVal <= 20) {
			iAddVal *= 2;
			bDouble = true;
		}

		if (nType == 0)
			PLAYERINFOMGR->AddBusiness(nPlayerID, iAddVal);
		else if (nType == 1)
			PLAYERINFOMGR->AddTechnology(nPlayerID, iAddVal);
		else
			PLAYERINFOMGR->AddMilitary(nPlayerID, iAddVal);

		nAddVal = iAddVal;
		AddCardAchievement(nPlayerID, pTime->Flag, iAddVal * 2);
		TIMESETMGR->RemoveTimeSet(nPlayerID, nTimeID);
		return OPERATOR_NONE_ERROR;
	}

	return OPERATOR_HASNO_ITEM;
}

enOpError CCardManager::AddCardLevel(U32 nPlayerID, std::string UID, S32 level)
{
	S32 nLimitLevel = 100;
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;
	if (level != 1)
		return OPERATOR_PARAMETER_ERROR;

	CCardData* pCardData = g_CardDataMgr->getData(pCard->CardID);
	if (!pCardData)
		return OPERATOR_HASNO_CARDID;

	CLvLimitData* pLimitData = g_LvLimitDataMgr->getData(pCard->OfficialPos);
	if (pLimitData)
		nLimitLevel = pLimitData->m_LimitLv + 50;

	CExpData* pExpData = g_ExpDataMgr->getData(MACRO_EXP_ID(pCardData->m_Color, pCard->Level));
	if (!pExpData)
		return OPERATOR_CARD_MAXLEVEL;

	S32 iCostMoney = mClamp((S32)pExpData->m_CostMoney, (S32)0, (S32)S32_MAX);
	//升级银两减少
	{
		g_Stats.Clear();
		CARDMGR->CaculateCardStats(nPlayerID, UID, g_Stats);
		StatsRef pStats = BUFFMGR->GetData(nPlayerID);
		iCostMoney = iCostMoney * (1.0f + (g_Stats.LvCost_Pc + (pStats ? pStats->ClvCost_gPc : 0)) / 100.0f);
	}
		
	if (!PLAYERINFOMGR->CanAddValue(nPlayerID, Money, -iCostMoney))
		return OPERATOR_HASNO_MONEY;

	S32 iCurLevel = pCard->Level;
	if (iCurLevel + level <0 || iCurLevel + level > nLimitLevel)
		return OPERATOR_CARD_MAXLEVEL;
	
	PLAYERINFOMGR->AddMoney(nPlayerID, -iCostMoney);
	pCard->Level = mClamp((S32)pCard->Level + level, (S32)0, (S32)S16_MAX);

	//重算缘分加成
	if (pCard->Level == 100 || pCard->Level == 150 || pCard->Level == 200 || pCard->Level == 250 || \
		pCard->Level == 300 || pCard->Level == 350 || pCard->Level == 400)
	{
		CaculteCardFate(nPlayerID);
	}

	//计算属性
	CaculateStats(nPlayerID, UID, false);
	UpdateDB(nPlayerID, pCard);
	UpdateToClient(nPlayerID, pCard);
	SUBMIT(nPlayerID, 6, 1);
	SUBMIT(nPlayerID, 32, pCard->Level);
	if (pCard->Level == 50) {
		SUBMIT(nPlayerID, 53, 1);
	}
	else if (pCard->Level == 100) {
		SUBMIT(nPlayerID, 54, 1);
	}
	else if (pCard->Level == 150) {
		SUBMIT(nPlayerID, 55, 1);
	}

	if (pCard->CardID == 110007) {
		SUBMIT(nPlayerID, 58, pCard->Level);
	}else if (pCard->CardID == 110008) {
		SUBMIT(nPlayerID, 59, pCard->Level);
	}else if (pCard->CardID == 110009) {
		SUBMIT(nPlayerID, 60, pCard->Level);
	}
	
	return OPERATOR_NONE_ERROR;
}

enOpError CCardManager::AddCardLevelLimit(U32 nPlayerID, std::string UID)
{
#define MAX_CARD_LEVEL 400
	CardRef pCard = GetData(nPlayerID, UID);
	U32 nLevelLimit = 100;
	if(!pCard)
		return OPERATOR_HASNO_CARD;

	CCardData* pCardData = g_CardDataMgr->getData(pCard->CardID);
	if (!pCardData)
		return OPERATOR_HASNO_CARDID;

	CLvLimitData* pLimitData = g_LvLimitDataMgr->getData(pCard->OfficialPos + 1);
	if (!pLimitData)
		return OPERATOR_CARD_LEVELMAX;

	nLevelLimit = pLimitData->m_LimitLv;
	if (pCard->Level < nLevelLimit)
		return OPERATOR_CARD_LEVELNOFIT;

	//能否扣除道具
	for (auto i = 0; i < 3; ++i)
	{
		if (!ITEMMGR->CanReduceItem(nPlayerID, pLimitData->m_ItemId[i], -1))
			return OPERATOR_HASNO_ITEM;
	}

	pCard->OfficialPos += 1;
	CaculateStats(nPlayerID, UID, false);
	UpdateDB(nPlayerID, pCard);
	UpdateToClient(nPlayerID, pCard);
	for (auto i = 0; i < 3; ++i)
	{
		ITEMMGR->AddItem(nPlayerID, pLimitData->m_ItemId[i], -1);
	}
	SUBMIT(nPlayerID, 36, pCard->OfficialPos);
	return OPERATOR_NONE_ERROR;
}

enOpError CCardManager::AddCardOfficalRank(U32 nPlayerID, std::string UID)
{
	StatsRef pStats = BUFFMGR->GetData(nPlayerID);
	U32 nLevelLimit = 10 -(pStats ? pStats->OffRank : 10);
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	CCardData* pCardData = g_CardDataMgr->getData(pCard->CardID);
	if (!pCardData)
		return OPERATOR_HASNO_CARDID;

	S32 nNextOfficalRank = pCard->OfficialRank + 1;
	if (nNextOfficalRank > nLevelLimit)
		return OPERATOR_SKILL_LEVELUP_ERROR;

	if (!CanAddCardAchievement(nPlayerID, UID, -s_OfficalRankConfig[nNextOfficalRank][0]))
		return OPERATOR_HASNO_ACHIEVEMENT;

	pCard->OfficialRank += 1;
	AddCardAchievement(nPlayerID, UID, -s_OfficalRankConfig[nNextOfficalRank][0], false, false);
	CaculateStats(nPlayerID, UID, false);
	UpdateDB(nPlayerID, pCard);
	UpdateToClient(nPlayerID, pCard);
	SUBMIT(nPlayerID, 46, pCard->OfficialRank);
	return OPERATOR_NONE_ERROR;
}

enOpError CCardManager::AddCardSkill(U32 nPlayerID, std::string UID, S32 SkillID, bool& bScuess)
{
	CSkillData* pSkill = g_SkillDataMgr->getData(SkillID);
	if (!pSkill || !pSkill->CanLearn())
		return OPERATOR_HASNO_SKILLID;

	U32 nSkillLevel = MACRO_SKILL_LEVEL(SkillID);
	if (nSkillLevel == 1)
		return _LearnSkill(nPlayerID, UID, SkillID, bScuess);

	return _LeaveUpSkill(nPlayerID, UID, SkillID, bScuess);
}

enOpError CCardManager::_LearnSkill(U32 nPlayerID, std::string UID, S32 SkillID, bool& bScuessful)
{
#define MAX_SKILL_LEVEL 10
	bool bSucess = true;
	U32 nSkillLevel = MACRO_SKILL_LEVEL(SkillID);
	U32 nSkillSeries = MACRO_SKILL_SERIES(SkillID);
	CSkillData* pSkill = g_SkillDataMgr->getData(SkillID);
	if (!pSkill || !pSkill->CanLearn())
		return OPERATOR_NOLEARN_SKILLID;

	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	std::vector<U32> SlotVec;
	for (int i = 0; i < MAX_SKILL_SLOT; ++i)
	{
		//已经学到一个技能
		if (MACRO_SKILL_SERIES(pCard->Skill[i]) == nSkillSeries)
		{
			SlotVec.clear();
			break;
		}
		else if (pCard->Skill[i] == 0)
		{
			SlotVec.push_back(i);
		}
	}

	if (SlotVec.empty())
		return OPERATOR_SKILL_MAX;

	for (int i = 0; i < MAX_COMPOSECOST_NUM; ++i)
	{
		if (pSkill->m_LevelUpItem[i] && pSkill->m_CostItemNum[i])
		{
			if (!ITEMMGR->CanReduceItem(nPlayerID, pSkill->m_LevelUpItem[i], pSkill->m_CostItemNum[i]))
				return OPERATOR_HASNO_ITEM;
		}
	}

	//扣除道具
	for (int i = 0; i < MAX_COMPOSECOST_NUM; ++i)
	{
		ITEMMGR->AddItem(nPlayerID, pSkill->m_LevelUpItem[i], -pSkill->m_CostItemNum[i]);
	}

	SUBMIT(nPlayerID, 30, 1);

	{
		S32 iRandVal = gRandGen.randI(1, 100);
		bScuessful = bSucess = (iRandVal < (S32)pSkill->m_SucessVal);
		if (bSucess)
		{
			U32 nSlot = *(SlotVec.begin());
			pCard->Skill[nSlot] = mClamp((S32)(SkillID), (S32)0, (S32)MACRO_SKILL_ID(nSkillSeries, MAX_SKILL_LEVEL));
			UpdateDB(nPlayerID, pCard);
			UpdateToClient(nPlayerID, pCard);
			return OPERATOR_NONE_ERROR;
		}
		return OPERATOR_SKILL_LEARN_FAILED;
	}
	return OPERATOR_HASNO_ITEM;
}

enOpError CCardManager::_LeaveUpSkill(U32 nPlayerID, std::string UID, S32 SkillID, bool& bScuessful)
{
#define MAX_SKILL_LEVEL 10
	bool bSucess = true;
	U32 nSkillLevel = MACRO_SKILL_LEVEL(SkillID);
	U32 nSkillSeries = MACRO_SKILL_SERIES(SkillID);
	if (nSkillLevel > MAX_SKILL_LEVEL)
		return OPERATOR_SKILL_MAXLEVEL;

	CSkillData* pSkill = g_SkillDataMgr->getData(SkillID);
	if (!pSkill || !pSkill->CanLearn())
		return OPERATOR_NOLEARN_SKILLID;

	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	U32 nSlot = MAX_SKILL_SLOT;
	for (int i = 0; i < MAX_SKILL_SLOT; ++i)
	{
		if (pCard->Skill[i] + 1 == SkillID)
		{
			nSlot = i;
			break;
		}
	}

	if (nSlot == MAX_SKILL_SLOT)
		return OPERATOR_NOLEARN_SKILLID;

	for (int i = 0; i < MAX_COMPOSECOST_NUM; ++i)
	{
		if (pSkill->m_LevelUpItem[i] && pSkill->m_CostItemNum[i])
		{
			if (!ITEMMGR->CanReduceItem(nPlayerID, pSkill->m_LevelUpItem[i], pSkill->m_CostItemNum[i]))
				return OPERATOR_HASNO_ITEM;
		}
	}

	//扣除道具
	for (int i = 0; i < MAX_COMPOSECOST_NUM; ++i)
	{
		ITEMMGR->AddItem(nPlayerID, pSkill->m_LevelUpItem[i], -pSkill->m_CostItemNum[i]);
	}

	SUBMIT(nPlayerID, 30, 1);

	{
		S32 iRandVal = gRandGen.randI(1, 100);
		S32 iSkillFailedNum = GetSkillFailedNumBySkillId(pCard->SkillsExFailedNum, nSkillSeries);
		bScuessful = bSucess = (iRandVal < mClamp((S32)pSkill->m_SucessVal + iSkillFailedNum * 5, 0, 100));
		if (bSucess)
		{
			pCard->Skill[nSlot] = mClamp((S32)(pCard->Skill[nSlot] + 1), (S32)0, (S32)MACRO_SKILL_ID(nSkillSeries, MAX_SKILL_LEVEL));
			ClearSkillFailedNumBySkillId(pCard->SkillsExFailedNum, nSkillSeries);
			UpdateDB(nPlayerID, pCard);
			UpdateToClient(nPlayerID, pCard);
			return OPERATOR_NONE_ERROR;
		}
		else {
			CardRef pCard = GetData(nPlayerID, UID);
			if (!pCard)
				return OPERATOR_HASNO_CARD;
			AddSkillFailedNumBySkillId(pCard->SkillsExFailedNum, nSkillSeries);
			UpdateDB(nPlayerID, pCard);
			UpdateToClient(nPlayerID, pCard);
		}
		return OPERATOR_SKILL_LEARN_FAILED;
	}

	return OPERATOR_HASNO_ITEM;
}

enOpError CCardManager::AddCardEquip(U32 nPlayerID, std::string UID, std::string ItemUID)
{
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	ItemInfoRef pItem = ITEMMGR->GetData(nPlayerID, ItemUID);
	if(!pItem)
		return OPERATOR_HASNO_ITEM;

	CItemData* pItemData = g_ItemDataMgr->getData(pItem->ItemID);
	if (!pItemData || !pItemData->isEquipMent())
		return OPERATOR_ITEM_EQUIP_ERROR;

	S32 nSlot = (pItemData->m_Type == 11 ? 0 : (pItemData->m_Type == 12 ? 1 : 2));
	pCard->Equip[nSlot] = ItemUID;
	//计算属性
	CaculateStats(nPlayerID, UID, false);
	UpdateDB(nPlayerID, pCard);
	UpdateToClient(nPlayerID, pCard);
	return OPERATOR_NONE_ERROR;
}

enOpError CCardManager::RemoveCardEquip(U32 nPlayerID, std::string UID, S32 nPos)
{
	if (nPos < 0 || nPos > 2)
		return OPERATOR_PARAMETER_ERROR;

	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	pCard->Equip[nPos] = "";
	//计算属性
	CaculateStats(nPlayerID, UID, false);
	UpdateDB(nPlayerID, pCard);
	UpdateToClient(nPlayerID, pCard);
	return OPERATOR_NONE_ERROR;
}

enOpError CCardManager::AddCardAbility(U32 nPlayerID, std::string UID, U32 nType, S32 nTimes)
{
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	CCardData* pCardData = g_CardDataMgr->getData(pCard->CardID);
	if (!pCardData)
		return OPERATOR_HASNO_CARDID;

	U32 nSlot = nType % 4;
	U16* pAddVal = NULL;
	if (nType < 0 || nType > 12)
		return OPERATOR_PARAMETER_ERROR;

	if (nTimes != 1 && nTimes != 10)
		return OPERATOR_PARAMETER_ERROR;

	if (pCardData->m_Color < s_AddAbilityConfig[nType][1])
		return OPERATOR_CARD_NOFITCOLOR;

	if (nType < 4)
		pAddVal = &pCard->Agi_Add[nSlot];
	else if (nType < 8)
		pAddVal = &pCard->Int_Add[nSlot];
	else
		pAddVal = &pCard->Str_Add[nSlot];

	if ((*pAddVal + nTimes) > pCard->Level)
		return OPERATOR_ABILITY_LEVEL_LIMIT;

	if (ITEMMGR->AddItem(nPlayerID, s_AddAbilityConfig[nType][0], -nTimes))
	{
		*pAddVal = mClamp(*pAddVal + nTimes, 0, pCard->Level);
		//计算属性
		CaculateStats(nPlayerID, UID, false);
		UpdateDB(nPlayerID, pCard);
		UpdateToClient(nPlayerID, pCard);
		return OPERATOR_NONE_ERROR;
	}

	return OPERATOR_HASNO_ITEM;
}

enOpError CCardManager::AddCardGrow(U32 nPlayerID, std::string UID, U32 nType, S32& nAddVal)
{
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	CCardData* pCardData = g_CardDataMgr->getData(pCard->CardID);
	if (!pCardData)
		return OPERATOR_HASNO_CARDID;

	//消耗物品
	S32 nCostItemNum = pCardData->m_Color;
	U16* pAddVal = NULL;
	U16 nInitVal = 0;
	if (nType < 0 || nType > 2)
		return OPERATOR_PARAMETER_ERROR;

	if (nType == 0)
	{
		pAddVal = &pCard->Agi_Grow;
		nInitVal = pCardData->m_InitAgiGrow;
	}
	else if (nType == 1)
	{
		pAddVal = &pCard->Int_Grow;
		nInitVal = pCardData->m_InitIntGrow;
	}
	else
	{
		pAddVal = &pCard->Str_Grow;
		nInitVal = pCardData->m_InitStrGrow;
	}

	if ((S32)(*pAddVal + 1) > U16_MAX)
		return OPERATOR_GROW_MAX;

	//消耗功勋
	U32 nCostAchieve = (U32)(pCardData->m_Color) * 10 * (*pAddVal + 1);
	StatsRef pStats = BUFFMGR->GetData(nPlayerID);
	nCostAchieve = nCostAchieve * (1.0f + (pStats ? pStats->RedAch_gPc / 100 : 0.0f));
	if (!CanAddCardAchievement(nPlayerID, UID, -nCostAchieve))
		return OPERATOR_HASNO_ACHIEVEMENT;

	S32 iRandomVal = gRandGen.randI(1, 100);
	S32 iSucessVal = (S32)mCeil(((F32)nInitVal / (F32)( nInitVal + ((*pAddVal) - nInitVal) * 2)) * 100);

	//扣除材料
	if (ITEMMGR->AddItem(nPlayerID, s_CostGrowItemID[nType], -nCostItemNum))
	{
		iSucessVal += GetGrowFailedNumByType(pCard->AptExFailedNum, nType) * 2;
		//成功
		if (iRandomVal < iSucessVal)
		{
			nAddVal = *pAddVal;
			*pAddVal = mClamp((S32)(*pAddVal + 1), (S32)0, (S32)U16_MAX);
			nAddVal = *pAddVal - nAddVal;
			//计算属性
			CaculateStats(nPlayerID, UID, false);
			TOPMGR->NewInDataEx(eTopType_CardUp, pCard->PlayerID, 0, 1, pCard->CardID);

			ClearGrowFailedNumByType((pCard->AptExFailedNum), nType);
		}
		else {
			AddGrowFailedNumByType(pCard->AptExFailedNum, nType);
		}

		AddCardAchievement(nPlayerID, UID, -nCostAchieve, false, false);
		UpdateDB(nPlayerID, pCard);
		UpdateToClient(nPlayerID, pCard);
		SUBMIT(nPlayerID, 31, 1);
		SUBMIT(nPlayerID, 10, 1);
		return OPERATOR_NONE_ERROR;
	}

	return OPERATOR_HASNO_ITEM;
}

CCardManager::DATA_MAP CCardManager::GetDataMap(U32 nPlayerID)
{
	DATA_MAP map;
	auto itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end()) {
		map = itr->second;
	}
	return map;
}

S32 CCardManager::GetCardNum(U32 nPlayerID, S32 nLevel/*=0*/) {
	S32 nNum = 0;
	auto itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end()) {
		DATA_MAP& map = itr->second;
		for (auto iter = map.begin(); iter != map.end(); ++iter) {
			if (iter->second && iter->second->Level >= nLevel) {
				nNum++;
			}
		}
	}
	return nNum;
}

S32	CCardManager::GetCardLevel(U32 nPlayerID, S32 CardID) {
	auto itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end()) {
		DATA_MAP& map = itr->second;
		for (auto iter = map.begin(); iter != map.end(); ++iter) {
			if (iter->second && iter->second->CardID == CardID) {
				return iter->second->Level;
			}
		}
	}
	return 0;
}

bool CCardManager::CaculateCardStats(U32 nPlayerID, std::string UID, Stats& stats)
{
	CardRef pCard = CARDMGR->GetData(nPlayerID, UID);
	if (pCard)
	{
		S32 nSeries[MAX_SKILL_SLOT];
		S32 nLevel[MAX_SKILL_SLOT];
		for (int j = 0; j < MAX_SKILL_SLOT; ++j)
		{
			nSeries[j] = MACRO_SKILL_SERIES(pCard->Skill[j]);
			nLevel[j]  = MACRO_SKILL_LEVEL(pCard->Skill[j]);
		}

		//获取装备buff
		for (int j = 0; j < MAX_EQUIP_SLOT; ++j)
		{
			std::string strItemUID = pCard->Equip[j];
			if (!strItemUID.empty())
			{
				ItemInfoRef pItem = ITEMMGR->GetData(nPlayerID, strItemUID);
				if (pItem)
				{
					CItemData* pItemData = g_ItemDataMgr->getData(pItem->ItemID);
					if (pItemData && (pItemData->m_SkillId > 0))
					{
						CSkillData* pSkillData = g_SkillDataMgr->getData(MACRO_SKILL_ID(pItemData->m_SkillId, pItem->Level + 1));
						if (pSkillData)
						{
							Stats buff1;
							pSkillData->getStats(buff1);
							for (int j = 0; j < MAX_SKILL_SLOT; ++j)
							{
								if (buff1.Skill[j] > 0)
								{
									nSeries[j] = 50001 + j;
									nLevel[j] += buff1.Skill[j];
								}
							}
						}
					}
				}
			}
		}

		for (int j = 0; j < MAX_SKILL_SLOT; ++j)
		{
			if (nSeries[j] > 0)
			{
				CSkillData* pSkillData = g_SkillDataMgr->getData(MACRO_SKILL_ID(nSeries[j], nLevel[j]));
				if (pSkillData)
				{
					pSkillData->getStats(stats);
				}
			}
		}
	}
	return true;
}

bool CCardManager::CaculateEquipStats(U32 nPlayerID, std::string UID, U32 nEquipAddPro[MAX_EQUIP_SLOT])
{
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return false;

	CCardData* pCardData = g_CardDataMgr->getData(pCard->CardID);
	if (!pCardData)
		return false;

	for (int i = 0; i < MAX_EQUIP_SLOT; ++i)
	{
		std::string strItemUID = pCard->Equip[i];
		if (!strItemUID.empty())
		{
			ItemInfoRef pItem = ITEMMGR->GetData(nPlayerID, strItemUID);
			if (pItem)
			{
				CItemData* pItemData = g_ItemDataMgr->getData(pItem->ItemID);
				if (pItemData && (pItemData->m_SkillId > 0))
				{
					CSkillData* pSkillData = g_SkillDataMgr->getData(MACRO_SKILL_ID(pItemData->m_SkillId, pItem->Level + 1));
					if (pSkillData)
					{
						for (int j = 0; j < MAX_SKILL_EFFECT; ++j)
						{
							if (pSkillData->m_BuffId[j] > 0)
							{
								CBuffData * pBuffData = g_BuffDataMgr->getData(pSkillData->m_BuffId[j]);
								if (pBuffData && (pBuffData->IsEquipAddAgi() || pBuffData->IsEquipAddInt() || pBuffData->IsEquipAddStr()))
								{
									U32 nSlot = pBuffData->IsEquipAddAgi() ? (0) : (pBuffData->IsEquipAddInt() ? (1) : (2));
									nEquipAddPro[nSlot] += pBuffData->m_BuffVal;
								}
							}
						}
					}
				}
			}
		}
	}

	return true;

}

bool CCardManager::CaculateStats(U32 nPlayerID, std::string UID, bool bNotify/*= true*/)
{
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return false;

	CCardData* pCardData = g_CardDataMgr->getData(pCard->CardID);
	if (!pCardData)
		return false;

	U32 EquipAddPro[MAX_EQUIP_SLOT] = { 0, 0, 0 };
	CaculateEquipStats(nPlayerID, UID, EquipAddPro);
	U32 PeerageAddPro = 0;
	{
		CLvLimitData* pLimitData = g_LvLimitDataMgr->getData(pCard->OfficialPos);
		if (pLimitData)
		{
			PeerageAddPro = pLimitData->m_AddV;
		}
	}
	S32 nBase_gPc[3] = { 0, 0, 0 };
	S32 nOffRankAddPro = s_OfficalRankConfig[pCard->OfficialRank][1];
	CaculteFateState(nPlayerID, UID, nBase_gPc);
	StatsRef pStats = BUFFMGR->GetData(nPlayerID);
	stBuff buff;
	CaculateBuff(buff, pCard);
	-buff;
	PLAYERINFOMGR->AddBuff(nPlayerID, 0, buff);
	S32 nLevel = mClamp(pCard->Level - 1, 0, 400);
	pCard->Agi = (pCardData->m_InitAgi + (pCard->Agi_Grow + PeerageAddPro) * nLevel) * (1.0 + ((nBase_gPc[0] + (pStats ? pStats->Agi_gPc : 0)) / 100.0f)) + pCard->Agi_Add[0] * 25 + \
		pCard->Agi_Add[1] * 50 + pCard->Agi_Add[2] * 75 + pCard->Agi_Add[3] * 100 + EquipAddPro[0] + nOffRankAddPro;

	pCard->Int = (pCardData->m_InitInt + (pCard->Int_Grow + PeerageAddPro) * nLevel) * (1.0 + ((nBase_gPc[1] + (pStats ? pStats->Int_gPc : 0)) / 100.0f)) + pCard->Int_Add[0] * 25 + \
		pCard->Int_Add[1] * 50 + pCard->Int_Add[2] * 75 + pCard->Int_Add[3] * 100 + EquipAddPro[1] + nOffRankAddPro;

	pCard->Str = (pCardData->m_InitStr + (pCard->Str_Grow + PeerageAddPro) * nLevel) * (1.0 + ((nBase_gPc[2] + (pStats ? pStats->Str_gPc : 0)) / 100.0f)) + pCard->Str_Add[0] * 25 + \
		pCard->Str_Add[1] * 50 + pCard->Str_Add[2] * 75 + pCard->Str_Add[3] * 100 + EquipAddPro[2] + nOffRankAddPro;

	CaculateBattleVal(pCard);
	buff.Clear();
	CaculateBuff(buff, pCard);
	PLAYERINFOMGR->AddBuff(nPlayerID, 0, buff);

	if (bNotify)
	{
		UpdateDB(nPlayerID, pCard);
		UpdateToClient(nPlayerID, pCard);
	}
	return true;
}

bool CCardManager::CaculateStats(U32 nPlayerID) 
{
	auto itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end()) {
		DATA_MAP& DataMap = itr->second;
		for (auto itr = DataMap.begin(); itr != DataMap.end(); ++itr) {
			if (itr->second) {
				CaculateStats(nPlayerID, itr->second->UID);
			}
		}
	}
	return true;
}

bool CCardManager::AddCardAchievement(U32 nPlayerID, std::string UID, S32 val, bool bAddItem/*=false*/, bool bNotify/*=true*/)
{
	//功勋获得量提升
	if (!bAddItem && val > 0)
	{
		g_Stats.Clear();
		CARDMGR->CaculateCardStats(nPlayerID, UID, g_Stats);
		StatsRef pStats = BUFFMGR->GetData(nPlayerID);
		val = val * (1.0f + (g_Stats.Ach_Pc + (pStats ?  pStats->Ach_gPc : 0)) / 100.0f);
	}
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return false;

	pCard->Achievement = mClamp((S32)pCard->Achievement + val, (S32)0, (S32)S32_MAX);
	if (bNotify)
	{
		UpdateDB(nPlayerID, pCard);
		UpdateToClient(nPlayerID, pCard);
	}
	return true;
}

bool CCardManager::CanAddCardAchievement(U32 nPlayerID, std::string UID, S32 val)
{
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return false;

	if ((pCard->Achievement + val < 0) || (pCard->Achievement + val > S32_MAX))
		return false;

	return true;
}

enOpError CCardManager::HireCard(U32 nPlayerID, U32 nCardID)
{
	CCardData* pCardData = g_CardDataMgr->getData(nCardID);
	if (!pCardData)
		return OPERATOR_HASNO_CARDID;

	if (!CanAddCard(nPlayerID, nCardID))
		return OPERATOR_ALREADY_OWN_CARD;

	//大臣队列
	StatsRef pStats = BUFFMGR->GetData(nPlayerID);
	auto DataMap = GetDataMap(nPlayerID);
	S32 nMaxSlot = 20 + (pStats ? pStats->CardNum : 0);
	if (DataMap.size() >= nMaxSlot)
		return  OPERATOR_HIRE_CARD_MAX;

	bool bSucess = false;
	S32 iReachVal = 0;

	if (pCardData->m_HireType == 2 || pCardData->m_HireType == 3 || pCardData->m_HireType == 4 || \
		pCardData->m_HireType == 5 || pCardData->m_HireType == 6 || pCardData->m_HireType == 7 || \
		pCardData->m_HireType == 10 || pCardData->m_HireType == 12 || pCardData->m_HireType == 13 || \
		pCardData->m_HireType == 14 || pCardData->m_HireType == 16 || pCardData->m_HireType == 17 || \
		pCardData->m_HireType == 18 || pCardData->m_HireType == 19 || pCardData->m_HireType == 20 ||\
		pCardData->m_HireType == 21 || pCardData->m_HireType == 22 || pCardData->m_HireType == 23)
	{
		if (pCardData->m_HireType == 2 || pCardData->m_HireType == 3 || pCardData->m_HireType == 4 || \
			pCardData->m_HireType == 5 || pCardData->m_HireType == 6 || pCardData->m_HireType == 7|| \
			pCardData->m_HireType == 14)
		{
			PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
			if (pPlayer)
			{
				if (pCardData->m_HireType == 2)
					iReachVal = pPlayer->BattleVal;
				else if (pCardData->m_HireType == 3)
				{ 
					iReachVal = mFloor((pPlayer->Business + pPlayer->getBuff(0).Business + pPlayer->getBuff(1).Business + pPlayer->getBuff(2).Business + \
							pPlayer->getBuff(3).Business));
				}
				else if (pCardData->m_HireType == 4)
				{ 
					iReachVal = mFloor((pPlayer->Technology + pPlayer->getBuff(0).Technology + pPlayer->getBuff(1).Technology + pPlayer->getBuff(2).Technology + \
						pPlayer->getBuff(3).Technology));
				}
				else if (pCardData->m_HireType == 5)
				{
					iReachVal = mFloor((pPlayer->Military + pPlayer->getBuff(0).Military + pPlayer->getBuff(1).Military + pPlayer->getBuff(2).Military + \
						pPlayer->getBuff(3).Military));
				}
				else if (pCardData->m_HireType == 6)
					iReachVal = pPlayer->Land;
				else if (pCardData->m_HireType == 7)
					iReachVal = pPlayer->QueenVal;
				else if (pCardData->m_HireType == 14)
				{
					TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, GATE_IMTER);
					if(pTime)
						iReachVal = pTime->Flag1;
				}
			}
		}
		else if (pCardData->m_HireType == 10)
		{
			PlayerSimpleDataRef spd = SERVER->GetPlayerManager()->GetPlayerData(nPlayerID);
			if (spd)
				iReachVal = spd->VipLv;
		}
		else if (pCardData->m_HireType == 16 || pCardData->m_HireType == 17 || pCardData->m_HireType == 18 || \
			pCardData->m_HireType == 21 || pCardData->m_HireType == 22)
		{
			S32 nSlot = (pCardData->m_HireType == 16 ? 0 : (pCardData->m_HireType == 17 ? 1 : (\
				pCardData->m_HireType == 18 ? 2 : (pCardData->m_HireType == 21 ? 3 : 4))));
			nSlot = mClamp(nSlot, 0, 5);
			iReachVal = GetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, nSlot);
		}
		else if (pCardData->m_HireType == 12)
		{
			S32 nSlot = (pCardData->m_HireType == 12 ? 0 : 0);
			nSlot = mClamp(nSlot, 0, 5);
			iReachVal = GetTimesInfo(nPlayerID, CARD_HIRE_TIMER2, nSlot);
		}
		else if (pCardData->m_HireType == 19)
		{
			iReachVal = GetTimesInfo(nPlayerID, COUNT_TIMER5, 5);
		}
		else if (pCardData->m_HireType == 20)
		{
			iReachVal = GetTimesInfo(nPlayerID, COUNT_TIMER2, 0);
		}
		else if (pCardData->m_HireType == 23)
		{
			iReachVal = GetTimesInfo(nPlayerID, COUNT_TIMER2, 2);
		}
		else if (pCardData->m_HireType == 13)
		{
			iReachVal = PRINCESSMGR->GetPrincessNum(nPlayerID);
		}

		if (iReachVal >= pCardData->m_HireVal)
			bSucess = true;
	}
	else if (pCardData->m_HireType == 8 || pCardData->m_HireType == 9 || pCardData->m_HireType == 11 || \
		pCardData->m_HireType == 15 || pCardData->m_HireType == 25)
	{
		if (pCardData->m_HireType == 8)
		{
			if (PLAYERINFOMGR->CanAddValue(nPlayerID, Money, -pCardData->m_HireVal))
			{
				PLAYERINFOMGR->AddMoney(nPlayerID, -pCardData->m_HireVal);
				bSucess = true;
			}
		}
		else if (pCardData->m_HireType == 9)
		{
			if (PLAYERINFOMGR->CanAddValue(nPlayerID, Ore, -pCardData->m_HireVal))
			{
				PLAYERINFOMGR->AddOre(nPlayerID, -pCardData->m_HireVal);
				bSucess = true;
			}
		}
		else if (pCardData->m_HireType == 11)
		{
			//扣除道具
			if (ITEMMGR->CanReduceItem(nPlayerID, 600100, -pCardData->m_HireVal)) {
				ITEMMGR->AddItem(nPlayerID, 600100, -pCardData->m_HireVal);
				bSucess = true;
			}
		}
		else if (pCardData->m_HireType == 25)
		{
			//扣除道具
			if (ITEMMGR->CanReduceItem(nPlayerID, 600111, -pCardData->m_HireVal)) {
				ITEMMGR->AddItem(nPlayerID, 600111, -pCardData->m_HireVal);
				bSucess = true;
			}
		}
		else
		{
			if (SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -pCardData->m_HireVal))
			{
				SERVER->GetPlayerManager()->AddGold(nPlayerID, -pCardData->m_HireVal, LOG_GOLD_HIRECARD);
				bSucess = true;
			}
		}
	}

	if (bSucess)
	{
		AddCard(nPlayerID, nCardID);
		//重算缘分加成
		CaculteCardFate(nPlayerID);
		SUBMIT(nPlayerID, 33, 1);
	}

	return  (bSucess ? OPERATOR_NONE_ERROR : OPERATOR_HIRE_FAILED);
}

enOpError CCardManager::UpdateApply(U32 nPlayerID, S32 iCreateNum)
{
	S32 nMaxSlot = 3 + VIPMGR->GetVipApplyTopLimit(nPlayerID);
	StatsRef pStats = BUFFMGR->GetData(nPlayerID);
	nMaxSlot = mClamp((nMaxSlot + (pStats ? pStats->ApplyNum : 0)), 0, 6);
	iCreateNum = mClamp(iCreateNum, 0, nMaxSlot);
	{
		bool bCreate = false;
		for (S32 i = APPLY_TIMER1; i < APPLY_TIMER1+nMaxSlot; ++i)
		{
			TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, i);
			if (!pTime)
			{
				bCreate = true;
				break;
			}
		}

		if (!bCreate)
			return OPERATOR_NONE_ERROR;

		auto iCurTime = _time32(NULL);
		//iCreateNum = mFloor((iCurTime - pTime->Flag1) / (30 * 60 * 1.0f)) + 1;
	}

	//生成奏章
	if (iCreateNum > 0)
	{
		PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
		if (itr == m_PlayerMap.end())
			return OPERATOR_PARAMETER_ERROR;

		DATA_MAP& CardMap = m_PlayerMap[nPlayerID];
		Vector<std::string> CardVec;
		Vector<U32> TimeVec;
		for (auto iter = CardMap.begin(); iter != CardMap.end(); ++iter)
		{
			if (iter->second)
				CardVec.push_back(iter->second->UID);
		}


		//生成奏章
		for (S32 i = APPLY_TIMER1; i < APPLY_TIMER1 + nMaxSlot; ++i)
		{
			TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, i);
			if (!pTime)
			{
				if (CardVec.empty() || iCreateNum <= 0)
					break;

				auto iCardNum = gRandGen.randI(0, CardVec.size()-1);
				auto UID = CardVec[iCardNum];
				TimeVec.push_back(i);
				iCreateNum--;
				auto iApplyId = gRandGen.randI(8001, 8016);
				TIMESETMGR->AddTimeSet(nPlayerID, i, 30*24*60*60, UID, iApplyId);
			}
		}

		CardVec.clear();
		TimeVec.clear();
	}

	return OPERATOR_NONE_ERROR;
}

enOpError CCardManager::ReadApply(U32 nPlayerID, S32 TimeID, bool bApply)
{
	TimeSetRef pTime;
	U32 nType = 0;
	S32 iAddVal = 0;

	pTime = TIMESETMGR->GetData(nPlayerID, TimeID);
	if (!pTime)
		return OPERATOR_PARAMETER_ERROR;

	CApplyData* pApplyData = g_ApplyDataMgr->getData(pTime->Flag1);
	if (!pApplyData)
		return OPERATOR_PARAMETER_ERROR;

	CardRef pCard = CARDMGR->GetData(nPlayerID, pTime->Flag);
	if (bApply)
		nType = pApplyData->m_Type[0];
	else 
		nType = pApplyData->m_Type[1];

	if (nType == 1)
	{
		iAddVal = pCard ? pCard->Agi * 0.01 : 100;
		PLAYERINFOMGR->AddBusiness(nPlayerID, iAddVal);
	}
	else if (nType == 2)
	{
		iAddVal = pCard ? pCard->Int * 0.01 : 100;
		PLAYERINFOMGR->AddTechnology(nPlayerID, iAddVal);
	}
	else if (nType == 3)
	{
		iAddVal = pCard ? pCard->Str * 0.01 : 100;
		PLAYERINFOMGR->AddMilitary(nPlayerID, iAddVal);
	}
	else if (nType == 4)
	{
		iAddVal = pCard ? pCard->Agi * 100: 100;
		PLAYERINFOMGR->AddMoney(nPlayerID, iAddVal);
	}
	else if (nType == 5)
	{
		iAddVal = pCard ? pCard->Int * 1 : 100;
		PLAYERINFOMGR->AddOre(nPlayerID, iAddVal);
	}
	else if (nType == 6)
	{
		iAddVal = pCard ? pCard->Str * 10 : 100;
		PLAYERINFOMGR->AddTroops(nPlayerID, iAddVal);
	}
	else if (nType == 7)
	{
		ITEMMGR->DropItem(nPlayerID, 300001);
	}
	else
	{
		iAddVal = pCard ? pCard->Level + 200 : 100;
		CARDMGR->AddCardAchievement(nPlayerID, pTime->Flag, iAddVal);
	}

	SUBMIT(nPlayerID, 2, 1);
	SUBMITACTIVITY(nPlayerID, ACTIVITY_READAPPLY, 1);

	TIMESETMGR->RemoveTimeSet(nPlayerID, pTime->ID);
	return OPERATOR_NONE_ERROR;
}

bool CCardManager::BanishCard(U32 nPlayerID, std::string UID)
{
	CardRef pCard = GetData(nPlayerID, UID);
	if (!pCard)
		return false;

	
	//返回技能书
	for (int i = 0; i < MAX_ADD_SLOT; ++i)
	{
		if (pCard->Agi_Add[i] > 0)
		{
			ITEMMGR->AddItem(nPlayerID, s_AddAbilityConfig[i][1], pCard->Agi_Add[i]);
		}
	}

	for (int i = 0; i < MAX_ADD_SLOT; ++i)
	{
		if (pCard->Int_Add[i] > 0)
		{
			ITEMMGR->AddItem(nPlayerID, s_AddAbilityConfig[4 + i][1], pCard->Int_Add[i]);
		}
	}

	for (int i = 0; i < MAX_ADD_SLOT; ++i)
	{
		if (pCard->Str_Add[i] > 0)
		{
			ITEMMGR->AddItem(nPlayerID, s_AddAbilityConfig[8 + i][1], pCard->Str_Add[i]);
		}
	}

	return RemoveCard(nPlayerID, UID);
}

auto GetCardDB = [](auto pHandle) {
	CardRef pCard = CardRef(new stCard);
	pCard->UID = pHandle->GetString();
	pCard->PlayerID = pHandle->GetInt();
	pCard->CardID = pHandle->GetInt();
	pCard->Level = pHandle->GetInt();
	pCard->Achievement = pHandle->GetInt();
	pCard->OfficialRank = pHandle->GetInt();
	pCard->OfficialPos = pHandle->GetInt();
	pCard->CardMask = pHandle->GetInt();

	pCard->Agi = pHandle->GetInt();
	pCard->Int = pHandle->GetInt();
	pCard->Str = pHandle->GetInt();

	pCard->Agi_Grow = pHandle->GetInt();
	pCard->Int_Grow = pHandle->GetInt();
	pCard->Str_Grow = pHandle->GetInt();

	for (int i = 0; i < MAX_EQUIP_SLOT; ++i)
	{
		pCard->Equip[i] = pHandle->GetString();
	}

	for (int i = 0; i < MAX_SKILL_SLOT; ++i)
	{
		pCard->Skill[i] = pHandle->GetInt();
	}

	for (int i = 0; i < MAX_ADD_SLOT; ++i)
	{
		pCard->Agi_Add[i] = pHandle->GetInt();
	}

	for (int i = 0; i < MAX_ADD_SLOT; ++i)
	{
		pCard->Int_Add[i] = pHandle->GetInt();
	}

	for (int i = 0; i < MAX_ADD_SLOT; ++i)
	{
		pCard->Str_Add[i] = pHandle->GetInt();
	}

	pCard->BattleVal = pHandle->GetInt();

	pCard->SkillsExFailedNum = pHandle->GetBigInt();

	pCard->AptExFailedNum = pHandle->GetInt();

	return pCard;
};

DB_Execution* CCardManager::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(fmt::sprintf("SELECT UID, PlayerID, CardID, Level, Achievement, OfficialRank, OfficialPos, \
		CardMask, Agi, Int, Str, Agi_Grow, Int_Grow, Str_Grow, Equip0, Equip1, Equip2, Skill0, Skill1, Skill2, Skill3,\
	    Skill4, Skill5, Skill6, Skill7, Skill8, Agi_Add0, Agi_Add1, Agi_Add2, Agi_Add3, Int_Add0, Int_Add1, Int_Add2, \
		Int_Add3, Str_Add0, Str_Add1, Str_Add2, Str_Add3, BattleVal, SkillsFailedNum, AptFailedNum  FROM Tbl_Card WHERE PlayerID=%d", nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void * pH)
	{
		DB_Execution* pHandle = (DB_Execution*)(pH);
		if (pHandle && error == NONE_ERROR)
		{
			stBuff buff;
			while (pHandle->More())
			{
				CardRef pCard = GetCardDB(pHandle);
				AddData(id, pCard->UID, pCard);
				CaculateBuff(buff, pCard);
			}

			PLAYERINFOMGR->AddBuff(id, 0, buff, true);
			LoadCardFateDB(id);
		}
		return true;
	});
	return pHandle;
}

CCardManager::DATA_MAP CCardManager::SyncLoadDB(U32 nPlayerID)
{
	DATA_MAP DataMap;
	TBLExecution pHandle(SERVER->GetActorDB());
	pHandle.SetId(nPlayerID);
	pHandle.SetSql(fmt::sprintf("SELECT UID, PlayerID, CardID, Level, Achievement, OfficialRank, OfficialPos, \
		CardMask, Agi, Int, Str, Agi_Grow, Int_Grow, Str_Grow, Equip0, Equip1, Equip2, Skill0, Skill1, Skill2, Skill3,\
	    Skill4, Skill5, Skill6, Skill7, Skill8, Agi_Add0, Agi_Add1, Agi_Add2, Agi_Add3, Int_Add0, Int_Add1, Int_Add2, \
		Int_Add3, Str_Add0, Str_Add1, Str_Add2, Str_Add3, BattleVal, SkillsFailedNum, AptFailedNum  FROM Tbl_Card WHERE PlayerID=%d", nPlayerID));
	pHandle.RegisterFunction([&](int id, int error, void * pH)
	{
		CDBConn* pHandle = (CDBConn*)(pH);
		if (pHandle && error == NONE_ERROR)
		{
			while (pHandle->More())
			{
				CardRef pCard = GetCardDB(pHandle);
				DataMap.insert(DATA_MAP::value_type(pCard->UID, pCard));
			}
		}
		return true;
	});

	pHandle.Commint();
	return DataMap;
}

bool CCardManager::DeleteDB(U32 nPlayerID, std::string UID)
{
	//CardRef pCard = CardRef(new stCard);
	//pCard->UID = UID;
	//pCard->PlayerID = nPlayerID;
	//CRedisDB conn(SERVER->GetRedisPool());
	//HashCmd<stCard> cmdRedis(&conn);
	//cmdRedis.hdel(pCard, nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	/*pHandle->SetSql(cmdRedis.DeleteSql(pCard));*/
	pHandle->SetSql(fmt::sprintf("DELETE FROM Tbl_Card WHERE UID='%s'", UID.c_str()));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CCardManager::InsertDB(U32 nPlayerID, CardRef pCard)
{
	//CRedisDB conn(SERVER->GetRedisPool());
	//HashCmd<stCard> cmdRedis(&conn);
	//cmdRedis.hset(pCard, nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	//pHandle->SetSql(cmdRedis.InsertSql(pCard));
	pHandle->SetSql(fmt::sprintf("INSERT INTO Tbl_Card \
			(UID,				PlayerID,				CardID,					Level,					Achievement,			OfficialRank,\
			OfficialPos,		CardMask,				Agi,					Int,					Str,					Agi_Grow,	\
			Int_Grow,			Str_Grow,				Equip0,					Equip1,					Equip2,\
			Skill0,				Skill1,					Skill2,					Skill3,		\
			Skill4,				Skill5,					Skill6,					Skill7,					Skill8,\
			Agi_Add0,			Agi_Add1,				Agi_Add2,				Agi_Add3,\
			Int_Add0,			Int_Add1,				Int_Add2,				Int_Add3,\
			Str_Add0,			Str_Add1,				Str_Add2,				Str_Add3,				BattleVal, \
			SkillsFailedNum,	AptFailedNum)\
			VALUES('%s',		%d,						%d,						%d,						%d,						%d,\
			%d,					%d,						%d,						%d,						%d,						%d,\
			%d,					%d,						'%s',					'%s',					'%s',\
			%d,					%d,						%d,						%d,	\
			%d,					%d,						%d,						%d,						%d,\
			%d,					%d,						%d,						%d,\
			%d,					%d,						%d,						%d,\
			%d,					%d,						%d,						%d,						%d,	\
			%lld,				%d) ",\
		pCard->UID.c_str(),		nPlayerID,				pCard->CardID,			pCard->Level,			pCard->Achievement,		pCard->OfficialRank,\
		pCard->OfficialPos,		pCard->CardMask,		pCard->Agi,				pCard->Int,				pCard->Str,				pCard->Agi_Grow,\
		pCard->Int_Grow,		pCard->Str_Grow,		pCard->Equip[0].c_str(),pCard->Equip[1].c_str(),pCard->Equip[2].c_str(),\
		pCard->Skill[0],		pCard->Skill[1],		pCard->Skill[2],		pCard->Skill[3],\
		pCard->Skill[4],		pCard->Skill[5],		pCard->Skill[6],		pCard->Skill[7],		pCard->Skill[8], \
		pCard->Agi_Add[0],		pCard->Agi_Add[1],		pCard->Agi_Add[2],		pCard->Agi_Add[3],\
		pCard->Int_Add[0],		pCard->Int_Add[1],		pCard->Int_Add[2],		pCard->Int_Add[3],\
		pCard->Str_Add[0],		pCard->Str_Add[1],		pCard->Str_Add[2],		pCard->Str_Add[3],		pCard->BattleVal,
		pCard->SkillsExFailedNum, pCard->AptExFailedNum));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);

	return true;
}

bool CCardManager::UpdateDB(U32 nPlayerID, CardRef pCard)
{
	//CRedisDB conn(SERVER->GetRedisPool());
	//HashCmd<stCard> cmdRedis(&conn);
	//cmdRedis.hset(pCard, nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	//pHandle->SetSql(cmdRedis.UpdateSql(pCard));
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Card \
	SET Level=%d,			Achievement=%d,			OfficialRank=%d,			OfficialPos=%d,			CardMask=%d,			Agi=%d,	\
		Int=%d,				Str=%d,					Agi_Grow=%d,				Int_Grow=%d,			Str_Grow=%d,			Equip0='%s',\
		Equip1='%s',		Equip2='%s',			Skill0=%d,					Skill1=%d,				Skill2=%d,				Skill3=%d,\
		Skill4=%d,			Skill5=%d,				Skill6=%d,					Skill7=%d,				Skill8=%d,				Agi_Add0=%d,\
		Agi_Add1=%d,		Agi_Add2=%d,			Agi_Add3=%d,				Int_Add0=%d,			Int_Add1=%d,			Int_Add2 =%d,\
		Int_Add3=%d, 		Str_Add0=%d,			Str_Add1=%d,				Str_Add2=%d,			Str_Add3=%d,			BattleVal=%d,\
		SkillsFailedNum=%lld, AptFailedNum=%d	\
		WHERE UID = '%s'AND PlayerID=%d", \
		pCard->Level,		pCard->Achievement,		pCard->OfficialRank,		pCard->OfficialPos,		pCard->CardMask,		pCard->Agi,\
		pCard->Int,			pCard->Str,				pCard->Agi_Grow,			pCard->Int_Grow,		pCard->Str_Grow,		pCard->Equip[0].c_str(),\
 pCard->Equip[1].c_str(),   pCard->Equip[2].c_str(),pCard->Skill[0],			pCard->Skill[1],		pCard->Skill[2],		pCard->Skill[3],\
		pCard->Skill[4],	pCard->Skill[5],		pCard->Skill[6],			pCard->Skill[7],		pCard->Skill[8],		pCard->Agi_Add[0],\
		pCard->Agi_Add[1],	pCard->Agi_Add[2],		pCard->Agi_Add[3],			pCard->Int_Add[0],		pCard->Int_Add[1],		pCard->Int_Add[2],\
		pCard->Int_Add[3],	pCard->Str_Add[0],		pCard->Str_Add[1],			pCard->Str_Add[2],		pCard->Str_Add[3],		pCard->BattleVal,\
		pCard->SkillsExFailedNum, pCard->AptExFailedNum,
		pCard->UID.c_str(), nPlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

void CCardManager::UpdateToClient(U32 nPlayerID, CardRef pCard)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_CARD_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pCard->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CCardManager::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		DATA_MAP& CardMap = m_PlayerMap[nPlayerID];
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_CARD_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(CardMap.size(), Base::Bit16);
		for (DATA_ITR itr = CardMap.begin(); itr != CardMap.end(); ++itr)
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

bool CCardManager::onPlayerEnter(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	LoadDB(pHead->Id);
	return true;
}

bool CCardManager::onPlayerLeave(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	RemovePlayer(pHead->Id);
	return true;
}

bool CCardManager::LoadCardFateDB(U32 nPlayerID)
{
	std::vector<std::string> VecStr;
	CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stFate> cmdRedis(&conn);
	cmdRedis.hvals(nPlayerID, VecStr);
	if (!VecStr.empty())
	{
		stBuff buff;
		for (auto itr : VecStr)
		{
			FateRef pFate = FateRef(new stFate);
			cmdRedis.GetVal(pFate, itr);
			m_FatePlayerMap[nPlayerID].insert(FATEMAP::value_type(pFate->FateID, pFate));
		}
	}
	else
	{
		CaculteCardFate(nPlayerID, false);
	}

	return true;
}

void CCardManager::CaculteFateState(U32 nPlayerID, std::string UID, S32 nBase_gPc[3])
{
	auto itr = m_FatePlayerMap.find(nPlayerID);
	if (itr == m_FatePlayerMap.end())
		return;

	FATEMAP& DataMap = itr->second;
	for (auto iter : DataMap)
	{
		auto pData = iter.second;
		if (!pData)
			continue;

		CFateData* pFateData = g_FateDataMgr->getData(pData->FateID);
		if (!pFateData)
			continue;

		for (auto FUID : pData->UID)
		{
			if (FUID == UID)
			{
				nBase_gPc[0]+= (pFateData->m_AddType == 1 || pFateData->m_AddType == 4) ? pData->Val : 0;
				nBase_gPc[1]+= (pFateData->m_AddType == 2 || pFateData->m_AddType == 4) ? pData->Val : 0;
				nBase_gPc[2]+= (pFateData->m_AddType == 3 || pFateData->m_AddType == 4) ? pData->Val : 0;
				break;
			}
		}
	}
}

void CCardManager::RemoveFatePlayer(U32 nPlayerID)
{
	auto iter = m_FatePlayerMap.find(nPlayerID);
	if (iter != m_FatePlayerMap.end())
	{
		FATEMAP& FateMap = iter->second;
		FateMap.clear();
		m_FatePlayerMap.erase(iter);
	}
}

bool CCardManager::CaculteCardFate(U32 nPlayerID, bool bNotify/*=true*/)
{
	auto GetFate = [&](auto CardMap, auto pFateData)
	{
		std::vector<stCardSimple> TempCardVec;
		S32 nLevel = 1;
		bool bActive = true;
		if (pFateData->m_Type == FATE_TYPE_ALL)
		{
			for (auto nCardId : pFateData->m_CardVec)
			{
				auto itr = CardMap.find(nCardId);
				if (itr != CardMap.end())
				{
					TempCardVec.push_back(itr->second);
				}
				else
				{
					bActive = false;
					break;
				}
			}

			if (bActive)
			{
				std::sort(TempCardVec.begin(), TempCardVec.end(), [](auto Left, auto Right) {
					return (Left).Level < (Right).Level;
				});

				if (!TempCardVec.empty())
				{
					S32 nLv = (*(TempCardVec.begin())).Level;
					if (nLv >= 100 && nLv < 150)
						nLevel = 2;
					else if (nLv >= 150 && nLv < 200)
						nLevel = 3;
					else if (nLv >= 200 && nLv < 250)
						nLevel = 4;
					else if (nLv >= 250 && nLv < 300)
						nLevel = 5;
					else if (nLv >= 300 && nLv < 350)
						nLevel = 6;
					else if (nLv >= 350 && nLv < 400)
						nLevel = 7;
					else if (nLv >= 400)
						nLevel = 8;
				}
			}
		}
		else if (pFateData->m_Type == FATE_TYPE_ONE)
		{
			for (auto nCardId : pFateData->m_CardVec)
			{
				auto itr = CardMap.find(nCardId);
				if (itr != CardMap.end())
				{
					TempCardVec.push_back(itr->second);
				}
			}

			if (TempCardVec.empty())
				bActive = false;

			nLevel = TempCardVec.size();
		}
		else
		{
			bActive = false;
		}

		if (bActive)
		{
			FateRef pFate = FateRef(new stFate);
			pFate->PlayerID = nPlayerID;
			pFate->FateID = pFateData->m_Id;
			pFate->Val = pFateData->m_AddVal * nLevel;
			pFate->Level = nLevel;
			auto nIndex = 0;
			for (auto simplecard : TempCardVec)
			{
				pFate->UID[nIndex++] = simplecard.UID;
			}
			CRedisDB conn(SERVER->GetRedisPool());
			HashCmd<stFate> cmdRedis(&conn);
			cmdRedis.hset(pFate, nPlayerID);
			m_FatePlayerMap[nPlayerID].insert(FATEMAP::value_type(pFate->FateID, pFate));
		}

		TempCardVec.clear();
	};

	RemoveFatePlayer(nPlayerID);
	PLAYER_ITR iter = m_PlayerMap.find(nPlayerID);
	if (iter != m_PlayerMap.end())
	{
		DATA_MAP& DataMap = iter->second;
		CARDSIMPLEMAP CardMap;
		DATA_MAP TempMap;
		for (auto itr : DataMap)
		{
			CardRef pData = itr.second;
			if (pData)
			{
				stCardSimple simple;
				simple.UID = pData->UID;
				simple.Level = pData->Level;
				CardMap.insert(CARDSIMPLEMAP::value_type(pData->CardID, simple));
				TempMap.insert(DATA_MAP::value_type(pData->UID, pData));
			}
		}

		std::vector<CFateData*>& FateVec = g_FateDataMgr->getFateVec();
		for (auto pFateData : FateVec)
		{
			if (pFateData)
			{
				GetFate(CardMap, pFateData);
			}
		}

		for (auto itr : TempMap)
		{
			if (itr.second)
			{
				CaculateStats(nPlayerID, itr.second->UID, bNotify);
				//UpdateDB(nPlayerID, itr.second);
			}
		}

		CardMap.clear();
	}
	return true;
}

//----------消息处理----------//
bool CCardManager::HandleAddCardLevelRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	S32 iLevel = Packet->readInt(Base::Bit16);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = AddCardLevel(pAccount->GetPlayerId(), UID, iLevel);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CARD_ADDLEVEL_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleAddCardLevelLimitRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = AddCardLevelLimit(pAccount->GetPlayerId(), strUID);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CARD_UPLVLIMIT_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleAddCardSkillRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	S32 iSkillID = Packet->readInt(Base::Bit32);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		bool bScuess = false;
		nError = AddCardSkill(pAccount->GetPlayerId(), strUID, iSkillID, bScuess);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CARD_ADDSKILL_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(iSkillID, Base::Bit32);
		sendPacket.writeFlag(bScuess);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleAddCardEquipRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	std::string itemUID = Packet->readString(MAX_UUID_LENGTH);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = AddCardEquip(pAccount->GetPlayerId(), strUID, itemUID);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CARD_ADDEQUIP_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleRemoveCardEquipRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	S32 nPos = Packet->readInt(Base::Bit8);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = RemoveCardEquip(pAccount->GetPlayerId(), strUID, nPos);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CARD_REMOVEEQUIP_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleAddCardAbilityRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	U32 nType = Packet->readInt(Base::Bit8);
	S32 nTimes = Packet->readInt(Base::Bit8);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = AddCardAbility(pAccount->GetPlayerId(), strUID, nType, nTimes);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CARD_ADDABILITY_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleAddCardGrowRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	U32 nType = Packet->readInt(Base::Bit8);
	S32 nError = 0, nAddVal = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = AddCardGrow(pAccount->GetPlayerId(), strUID, nType, nAddVal);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CARD_ADDGROW_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nType, Base::Bit8);
		sendPacket.writeInt(nAddVal, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleHireCardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nCardID = Packet->readInt(Base::Bit32);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = HireCard(pAccount->GetPlayerId(), nCardID);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CARD_HIRE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nCardID, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleDispatchCardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	U32 nType = Packet->readInt(Base::Bit8);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = DispatchCard(pAccount->GetPlayerId(), strUID, nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_CARD_DISPATCH_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleReadApplyRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 TimeID = Packet->readInt(Base::Bit8);
	bool bAgreen = Packet->readFlag();
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = ReadApply(pAccount->GetPlayerId(), TimeID, bAgreen);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_READ_APPLY_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleAddCardOfficalRankRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = AddCardOfficalRank(pAccount->GetPlayerId(), UID);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ADD_CARD_OFFICALRANK_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CCardManager::HandleDispatchCardAwardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 nTimeId = Packet->readInt(Base::Bit16);
	S32 nAddVal = 0;
	bool bDouble = false;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = DispatchCardAward(pAccount->GetPlayerId(), nTimeId, nAddVal, bDouble);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_DISPATCH_CARD_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nAddVal, Base::Bit32);                                                                                                                                                                         
		sendPacket.writeFlag(bDouble);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

S32 CCardManager::GetSkillFailedNumBySkillId(U64 SkillsFailedNum, S32 SkillId)
{
	U8 index = mClamp(SkillId % 10, 1, 9);
	SkillsFailedNum >>= (index - 1) * 7;
	return SkillsFailedNum & 127;
}

void CCardManager::AddSkillFailedNumBySkillId(U64& SkillsFailedNum, S32 SkillId)
{
	uint64_t Failed = 1;
	U8 index = mClamp(SkillId % 10, 1, 9);
	Failed <<= (index - 1) * 7;
	SkillsFailedNum += Failed;
}

void CCardManager::ClearSkillFailedNumBySkillId(U64& SkillsFailedNum, S32 SkillId)
{
	uint64_t Failed = 1;
	U8 index = mClamp(SkillId % 10, 1, 9);
	uint64_t tmp = 127;
	tmp = ~(tmp << ((index - 1) * 7));
	SkillsFailedNum &= tmp;
}

S32 CCardManager::GetGrowFailedNumByType(U32 GrowFialedNum, S32 Type)
{
	Type = mClamp(Type, 0, 2);
	GrowFialedNum >>= Type * 7;
	return GrowFialedNum & 127;
}

void CCardManager::AddGrowFailedNumByType(U32& GrowFialedNum, S32 Type)
{
	uint64_t Failed = 1;
	Type = mClamp(Type, 0, 2);
	Failed <<= Type * 7;
	GrowFialedNum += Failed;
}

void CCardManager::ClearGrowFailedNumByType(U32& GrowFialedNum, S32 Type)
{
	uint64_t Failed = 1;
	Type = mClamp(Type, 0, 2);
	uint64_t tmp = 127;
	tmp = ~(tmp << (Type * 7));
	GrowFialedNum &= tmp;
}

void LoginCard(int id)
{
	CARDMGR->LoadDB(id);
}
void ___AddCardDB(int id, int cardid)
{
	if(CARDMGR->CanAddCard(id, cardid))
		CARDMGR->AddCard(id, cardid);
}
void ___DispatchCard(int id, std::string UID, int type)
{
	CARDMGR->DispatchCard(id, UID, type);
}
void ___AddCardLevel(int id, std::string UID, int level)
{
	CARDMGR->AddCardLevel(id, UID, level);
}
void ___AddCardSkill(int id, std::string UID, int skillid)
{
	bool bScuess = false;
	CARDMGR->AddCardSkill(id, UID, skillid, bScuess);
}
void ___AddCardAbility(int id, std::string UID, int type, int times)
{
	CARDMGR->AddCardAbility(id, UID, type, times);
}
void ___AddCardGrow(int id, std::string UID, int type)
{
	S32 nAddVal = 0;
	CARDMGR->AddCardGrow(id, UID, type, nAddVal);
}
void ___CaculateStats(int id, std::string UID)
{
	CARDMGR->CaculateStats(id, UID);
}
void ___SyncLoadDB(int id)
{
	CARDMGR->SyncLoadDB(id);
}

void ___AddCardOfficalRank(int id, std::string UID)
{
	CARDMGR->AddCardOfficalRank(id, UID);
}

void CCardManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "SLoginCard", &___SyncLoadDB);
	lua_tinker::def(L, "LoginCard", &LoginCard);
	lua_tinker::def(L, "AddCardDB", &___AddCardDB);
	lua_tinker::def(L, "DispatchCard", &___DispatchCard);
	lua_tinker::def(L, "AddCardLevel", &___AddCardLevel);
	lua_tinker::def(L, "AddCardSkill", &___AddCardSkill);
	lua_tinker::def(L, "AddCardAbility", &___AddCardAbility);
	lua_tinker::def(L, "AddCardGrow", &___AddCardGrow);
	lua_tinker::def(L, "CaculateStats", &___CaculateStats);
	lua_tinker::def(L, "AddCardOfficalRank", &___AddCardOfficalRank);
}
