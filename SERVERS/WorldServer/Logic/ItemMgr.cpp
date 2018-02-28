#include "ItemMgr.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "../WorldServer.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "ItemData.h"
#include "Common/MemGuard.h"
#include "Common/mRandom.h"
#include "../PlayerMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "PlayerInfoMgr.h"
#include "TimeSetMgr.h"
#include "CardMgr.h"
#include "platform.h"
#include "TaskMgr.h"
#include "NoviceGuide.h"
#include "PvpMgr.h"

const unsigned int MAX_ITEM_SLOT = 1500;
static std::vector<S32> s_ShopTimer = {
	SHOP_BUY_TIMER1, SHOP_BUY_TIMER2, SHOP_BUY_TIMER3, SHOP_BUY_TIMER4, SHOP_BUY_TIMER5, SHOP_BUY_TIMER6,
	SHOP_BUY_TIMER7, SHOP_BUY_TIMER8, SHOP_BUY_TIMER9, SHOP_BUY_TIMER10,SHOP_BUY_TIMER11,SHOP_BUY_TIMER12,
	SHOP_BUY_TIMER13,SHOP_BUY_TIMER14,SHOP_BUY_TIMER15,SHOP_BUY_TIMER16,SHOP_BUY_TIMER17,SHOP_BUY_TIMER18,
	SHOP_BUY_TIMER19,SHOP_BUY_TIMER20
};

static S32 s_ItemShop[5][4] = {
	{ 0,		0,		0,			0 },
	{ 300100,	200,	600091,		1 },
	{ 300100,	1888,	600091,		10 },
	{ 300101,	1000,	600090,		1 },
	{ 300101,	5000,	600090,		5 },
};

static S32 s_EquipStrengthen[5][11] = {
	{ 0,		0,		0,		0,		0,			0,			0,			0,			0,			0,			0 },
	{ 0,		10,		40,		90,		160,		250,		360,		490,		640,		810,		1000 },
	{ 0,		40,		160,	360,	640,		1000,		1440,		1960,		2560,		3240,		4000 },
	{ 0,		90,		360,	810,	1440,		2250,		3240,		4410,		5760,		7290,		9000 },
	{ 0,		640,	2560,	5760,	10240,		16000,		23040,		31360,		40960,		51840,		64000 },
};

static S32 s_EquipStrengthenExp[5][11] = {
	{ 0,		0,		0,		0,		0,			0,			0,			0,			0,			0,			0 },
	{ 20,		30,		70,		160,	320,		570,		930,		1420,		2060,		2870,		3870 },
	{ 100,		140,	300,	660,	1300,		2300,		3740,		5700,		8260,		11500,		15500 },
	{ 500,		590,	950,	1760,	3200,		5450,		8690,		13100,		18860,		26150,		35150 },
	{ 2500,		3140,	5700,	11460,	21700,		37700,		60740,		92100,		133060,		184900,		248900 },
};

#define DRAW_ITEM_GOLD_ONE_ITEM 600103
#define DRAW_ITEM_GOLD_TEN_ITEM 600104

CItemManager::CItemManager()
{
	g_ItemDataMgr->read();
	g_ComposeDataMgr->read();
	g_DropDataMgr->read();
	g_GiftDataMgr->read();
	g_ShopDataMgr->read();
	g_RandomShopDataMgr->read();
	g_EquipStrengthenDataMgr->read();
	REGISTER_EVENT_METHOD("CW_ITEM_COMPOSE_REQUEST", this, &CItemManager::HandleClientComposeItemRequest);
	REGISTER_EVENT_METHOD("CW_ITEM_USE_REQUEST", this, &CItemManager::HandleClientUseItemRequest);
	REGISTER_EVENT_METHOD("CW_ITEM_BUY_REQUEST", this, &CItemManager::HandleClientBuyItemRequest);
	REGISTER_EVENT_METHOD("CW_ITEM_USEACCCARD_REQUEST", this, &CItemManager::HandleClientUseAccCardRequest);
	REGISTER_EVENT_METHOD("CW_ITEM_USEBATTLECARD_REQUEST", this, &CItemManager::HandleClientUseAddPVPTimesRequest);

	REGISTER_EVENT_METHOD("CW_BUY_RANDOMSHOP_REQUEST", this, &CItemManager::HandleClientBuyRandomShopRequest);
	REGISTER_EVENT_METHOD("CW_OPEN_RANDOMSHOP_REQUEST", this, &CItemManager::HandleClientOpenRandomShopRequest);
	REGISTER_EVENT_METHOD("CW_REFLASH_RANDOMSHOP_REQUEST", this, &CItemManager::HandleClientReflashRandomShopRequest);
	REGISTER_EVENT_METHOD("CW_DRAWITEM_REQUEST", this, &CItemManager::HandleClientDrawItemRequest);
	REGISTER_EVENT_METHOD("CW_EquipStrengthen_REQUEST", this, &CItemManager::HandleClientEquipStrengthenRequest);
}

CItemManager::~CItemManager()
{
	m_PlayerMap.clear();
	g_ItemDataMgr->clear();
	g_ComposeDataMgr->clear();
	g_DropDataMgr->clear();
	g_GiftDataMgr->clear();
	g_ShopDataMgr->clear();
	g_RandomShopDataMgr->clear();
	g_EquipStrengthenDataMgr->clear();
}

CItemManager* CItemManager::Instance()
{
	static CItemManager s_Mgr;
	return &s_Mgr;
}

ItemInfoRef CItemManager::CreateItem(U32 nItemId, U32 nPayerID, U32 nQuantity/* =1*/)
{
	CItemData* pData = g_ItemDataMgr->getData(nItemId);
	if (!pData)
		return NULL;

	DBError err = DBERR_UNKNOWERR;
	try
	{
		ItemInfoRef pItem = ItemInfoRef(new stItemInfo);
		pItem->InitData();
		dStrcpy(pItem->UID, MAX_UUID_LENGTH, uuid());
		pItem->ItemID = nItemId;
		pItem->Quantity = nQuantity;
		pItem->PlayerID = nPayerID;
		return pItem;
	}
	DBECATCH()

		return NULL;
}

bool CItemManager::AddItem(U32 nPlayerID, U32 nItemId, S32 Quantity/*= 1*/)
{
	if (Quantity > 0)
		return _AddItem(nPlayerID, nItemId, Quantity);

	return _ReduceItem(nPlayerID, nItemId, Quantity);
}

bool CItemManager::_AddItem(U32 nPlayerID, U32 nItemId, S32 Quantity/*= 1*/)
{
	CItemData* pData = g_ItemDataMgr->getData(nItemId);
	if (!pData)
		return false;

	S32 iLeftQuantity = Quantity;
	S32 iNeedQuantity = 0;
	bool bError = false;
	DATA_MAP& ItemMap = m_PlayerMap[nPlayerID];
	DATA_MAP  CreateItemMap;
	typedef stdext::hash_map<std::string, S32> QITEMMAP;
	QITEMMAP BatMap;
	for (DATA_ITR itr = ItemMap.begin(); itr != ItemMap.end(); ++itr)
	{
		if (itr->second && itr->second->ItemID == nItemId && itr->second->Quantity < pData->m_MaxDie)
		{
			iNeedQuantity = iLeftQuantity;
			iLeftQuantity -= (pData->m_MaxDie - itr->second->Quantity);

			if (iLeftQuantity > 0)
			{
				BatMap.insert(QITEMMAP::value_type(itr->second->UID, (pData->m_MaxDie - itr->second->Quantity)));
			}
			else
			{
				BatMap.insert(QITEMMAP::value_type(itr->second->UID, iNeedQuantity));
				break;
			}
		}
	}

	//不足
	while (iLeftQuantity > 0)
	{
		iNeedQuantity = iLeftQuantity;
		iLeftQuantity -= pData->m_MaxDie;
		if (iLeftQuantity > 0)
		{
			ItemInfoRef pItem = CreateItem(nItemId, nPlayerID, pData->m_MaxDie);
			if (pItem)
				CreateItemMap.insert(DATA_MAP::value_type(pItem->UID, pItem));
			else
			{
				bError = true;
				break;
			}
		}
		else
		{
			ItemInfoRef pItem = CreateItem(nItemId, nPlayerID, iNeedQuantity);
			if (pItem)
				CreateItemMap.insert(DATA_MAP::value_type(pItem->UID, pItem));
			else
			{
				bError = true;
			}

			break;
		}
	}

	if (bError)
	{
	}
	else
	{
		for (QITEMMAP::iterator itr = BatMap.begin(); itr != BatMap.end(); ++itr)
		{
			ItemInfoRef pItem = ItemMap[itr->first];
			if (pItem)
			{
				pItem->Quantity += itr->second;
				UpdateDB(pItem);
				UpdateToClient(nPlayerID, pItem);
			}
		}

		for (DATA_ITR itr = CreateItemMap.begin(); itr != CreateItemMap.end(); ++itr)
		{
			ItemInfoRef pItem = CreateItemMap[itr->first];
			if (pItem)
			{
				ItemMap.insert(DATA_MAP::value_type(pItem->UID, pItem));
				InsertDB(nPlayerID, pItem);
				UpdateToClient(nPlayerID, pItem);
			}
		}
	}

	CreateItemMap.clear();
	BatMap.clear();

	if (pData->isEquipMent()) {
		SetTimesInfo(nPlayerID, COUNT_TIMER2, 1, abs(Quantity));
		SUBMIT(nPlayerID, 27, 1);
	}

	return !bError;
}

bool CItemManager::_ReduceItem(U32 nPlayerID, U32 nItemId, S32 Quantity/*= 1*/)
{
	CItemData* pData = g_ItemDataMgr->getData(nItemId);
	if (!pData)
		return false;

	S32 iLeftQuantity = abs(Quantity);
	S32 iNeedQuantity = 0;
	bool bError = false;
	DATA_MAP& ItemMap = m_PlayerMap[nPlayerID];
	typedef stdext::hash_map<std::string, S32> QITEMMAP;
	QITEMMAP BatMap;
	for (DATA_ITR itr = ItemMap.begin(); itr != ItemMap.end(); ++itr)
	{
		if (itr->second && itr->second->ItemID == nItemId)
		{
			iNeedQuantity = iLeftQuantity;
			iLeftQuantity -= itr->second->Quantity;

			if (iLeftQuantity > 0)
			{
				BatMap.insert(QITEMMAP::value_type(itr->second->UID, (itr->second->Quantity)));
			}
			else
			{
				BatMap.insert(QITEMMAP::value_type(itr->second->UID, iNeedQuantity));
				break;
			}
		}
	}


	if (iLeftQuantity > 0)
		bError = true;

	if (!bError)
	{
		for (QITEMMAP::iterator itr = BatMap.begin(); itr != BatMap.end(); ++itr)
		{
			DATA_ITR iter = ItemMap.find(itr->first);
			if (iter != ItemMap.end() && iter->second)
			{
				ItemInfoRef pItem = iter->second;
				pItem->Quantity -= itr->second;
				if (pItem->Quantity == 0)
				{
					ItemInfoRef pItem1 = pItem;
					std::string UID = pItem->UID;
					ItemMap.erase(iter);
					DeleteDB(nPlayerID, UID);
					UpdateToClient(nPlayerID, pItem1);
				}
				else
				{
					UpdateDB(pItem);
					UpdateToClient(nPlayerID, pItem);
				}
			}
		}
	}

	BatMap.clear();
	//if (pData->isEquipMent())
	//	SetTimesInfo(nPlayerID, COUNT_TIMER2, 1, -abs(Quantity));

	//翡翠玉石消耗
	if (pData->m_ItemId == 600047) {
		SUBMITACTIVITY(nPlayerID, ACTIVITY_ITEM1, Quantity);
	}
	else if (pData->m_ItemId >= 600005 && pData->m_ItemId <= 600007) {
		SUBMITACTIVITY(nPlayerID, ACTIVITY_ITEM2, Quantity);
	}
	else if (pData->m_ItemId >= 600020 && pData->m_ItemId <= 600028) {
		SUBMITACTIVITY(nPlayerID, ACTIVITY_ITEM3, Quantity);
	}
	else if(pData->m_ItemId >= 600008 && pData->m_ItemId <= 600019)
	{
		SUBMITACTIVITY(nPlayerID, ACTIVITY_ITEM4, Quantity);
	}

	return !bError;
}

bool CItemManager::CanReduceItem(U32 nPlayerID, U32 nItemId, S32 Quantity/*=1*/)
{
	CItemData* pData = g_ItemDataMgr->getData(nItemId);
	if (!pData)
		return false;

	S32 iLeftQuantity = abs(Quantity);
	S32 iNeedQuantity = 0;
	bool bError = false;
	DATA_MAP& ItemMap = m_PlayerMap[nPlayerID];
	typedef stdext::hash_map<std::string, S32> QITEMMAP;
	for (DATA_ITR itr = ItemMap.begin(); itr != ItemMap.end(); ++itr)
	{
		if (itr->second && itr->second->ItemID == nItemId)
		{
			iNeedQuantity = iLeftQuantity;
			iLeftQuantity -= itr->second->Quantity;

			if (iLeftQuantity > 0)
			{
			}
			else
			{
				break;
			}
		}
	}

	if (iLeftQuantity > 0)
		bError = true;

	return !bError;
}

bool CItemManager::UseItem(U32 nPlayerID, U32 nItemId, S32 Quantity/*=1*/)
{
	CItemData* pData = g_ItemDataMgr->getData(nItemId);
	if (!pData || pData->CanUse())
		return false;

	if (Quantity <= 0)
		return false;

	//删除物品成功
	if (AddItem(nPlayerID, nItemId, -Quantity) == true)
	{
		if (pData->isAddMoney())
		{
			PLAYERINFOMGR->AddMoney(nPlayerID, pData->m_UseVal * Quantity);
		}
		else if (pData->isAddOre())
		{
			PLAYERINFOMGR->AddOre(nPlayerID, pData->m_UseVal * Quantity);
		}
		else if (pData->isAddTroops())
		{
			PLAYERINFOMGR->AddTroops(nPlayerID, pData->m_UseVal * Quantity);
		}
		else if (pData->isGift())
		{
			AddGift(nPlayerID, pData->m_UseVal, Quantity);
		}
		return true;
	}

	return false;
}

bool CItemManager::UseItem(U32 nPlayerID, std::string UID, S32 Quantity/*=1*/, std::string UID1/*=""*/)
{
	ItemInfoRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return false;

	CItemData* pItemData = g_ItemDataMgr->getData(pData->ItemID);
	if (!pItemData || pItemData->CanUse())
		return false;

	if (Quantity <= 0)
		return false;

	if (pData->Quantity < Quantity)
		return false;

	pData->Quantity -= Quantity;
	if (pData->Quantity == 0)
	{
		ItemInfoRef pData1 = pData;
		std::string UID = pData1->UID;
		DeleteDB(nPlayerID, UID);
		UpdateToClient(nPlayerID, pData1);
	}
	else
	{
		UpdateDB(pData);
		UpdateToClient(nPlayerID, pData);
	}

	//删除物品成功
	//if (AddItem(nPlayerID, nItemId, -Quantity) == true)
	{
		if (pItemData->isAddMoney())
		{
			PLAYERINFOMGR->AddMoney(nPlayerID, pItemData->m_UseVal * Quantity);
		}
		else if (pItemData->isAddGold())
		{
			SERVER->GetPlayerManager()->AddGold(nPlayerID, pItemData->m_UseVal * Quantity);
		}
		else if (pItemData->isAddOre())
		{
			PLAYERINFOMGR->AddOre(nPlayerID, pItemData->m_UseVal * Quantity);
		}
		else if (pItemData->isAddTroops())
		{
			PLAYERINFOMGR->AddTroops(nPlayerID, pItemData->m_UseVal * Quantity);
		}
		else if (pItemData->isAddVim())
		{
			U32 nTime = 0;
			TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, TURN_CARD_TIMER);
			if (pTime)
			{
				nTime = pTime->Flag1;
				nTime += pItemData->m_UseVal * Quantity;
				TIMESETMGR->AddTimeSet(nPlayerID, TURN_CARD_TIMER, -1, "", nTime);
			}
		}
		else if (pItemData->isAddImpose())
		{
			TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, IMPOSE_TIMER);
			if (pTime)
			{
				TIMESETMGR->AddTimeSet(nPlayerID, IMPOSE_TIMER, -1, "", pTime->Flag1 + 10 * Quantity);
			}
		}
		else if (pItemData->isDrop())
		{
			BatchDropItem(nPlayerID, pItemData->m_UseVal, Quantity);
		}
		else if (pItemData->isGift())
		{
			AddGift(nPlayerID, pItemData->m_UseVal, Quantity);
		}
		else if (pItemData->isAddAchieve())
		{
			CARDMGR->AddCardAchievement(nPlayerID, UID1, pItemData->m_UseVal* Quantity, true, true);
		}
		else if (pItemData->isAddStrength())
		{
			TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, CITY_TIMER2);
			if (pTime)
			{
				TIMESETMGR->AddTimeSet(nPlayerID, CITY_TIMER2, -1, "", pTime->Flag1 + pItemData->m_UseVal * Quantity);
			}
		}
		return true;
	}

	return false;
}

bool CItemManager::ComposeItem(U32 nPlayerID, U32 nComposeID)
{
	CComposeData* pData = g_ComposeDataMgr->getData(nComposeID);
	if (!pData)
		return false;

	for (int i = 0; i < MAX_COMPOSECOST_NUM; ++i)
	{
		if (pData->m_CostItemID[i] && pData->m_CostItemNum[i])
		{
			if (!CanReduceItem(nPlayerID, pData->m_CostItemID[i], pData->m_CostItemNum[i]))
				return false;
		}
	}

	//成功
	if (AddItem(nPlayerID, pData->m_ComposeItemID, 1))
	{
		for (int i = 0; i < MAX_COMPOSECOST_NUM; ++i)
		{
			if (pData->m_CostItemID[i] && pData->m_CostItemNum[i])
			{
				AddItem(nPlayerID, pData->m_CostItemID[i], -pData->m_CostItemNum[i]);
			}
		}
		return true;
	}

	return false;
}

CDropData* CItemManager::DropItem(U32 nDropID)
{
	S32 iMaxRand = g_DropDataMgr->getData(nDropID);
	if (!iMaxRand)
		return 0;

	S32 iVal = gRandGen.randI(0, iMaxRand);
	return g_DropDataMgr->getData(nDropID, iVal);
}

DROPITEM_PAIR CItemManager::DropItem(U32 nPlayerID, U32 nDropID)
{
	CDropData* pDropData = DropItem(nDropID);
	if (pDropData)
	{
		AddItem(nPlayerID, pDropData->m_ItemId, pDropData->m_ItemNum);
		return std::make_pair(pDropData->m_ItemId, pDropData->m_ItemNum);
	}
	return std::make_pair(0, 0);
}

bool CItemManager::BatchDropItem(U32 nPlayerID, U32 nDropID, S32 Quantity/*=1*/)
{
	std::unordered_map<S32, S32> ItemMap;

	for (int i = 0; i < Quantity; ++i)
	{
		CDropData* pDropData = ITEMMGR->DropItem(nDropID);
		if (pDropData)
		{
			ItemMap[pDropData->m_ItemId] += pDropData->m_ItemNum;
		}
	}

	for (auto pPair : ItemMap)
	{
		AddItem(nPlayerID, pPair.first, pPair.second);
	}
	return true;
}

bool CItemManager::AddGift(U32 nPlayerID, U32 nGiftID, S32 Quantity/*=1*/)
{
	std::vector<CGiftData*> GiftVec = g_GiftDataMgr->getData(nGiftID);
	for (auto pData : GiftVec)
	{
		if (pData)
			AddItem(nPlayerID, pData->m_ItemId, pData->m_Quantity * Quantity);
	}
	return true;
}

enOpError CItemManager::BuyItem(U32 nPlayerID, U32 nShopId, S32 Quantity/*=1*/)
{
	if (Quantity <= 0 || Quantity > 255)
		return OPERATOR_PARAMETER_ERROR;

	CShopData* pShopData = g_ShopDataMgr->getData(nShopId);
	if (!pShopData)
		return OPERATOR_PARAMETER_ERROR;

	S32 nCostMoney = pShopData->m_Sale * abs(1.0f - pShopData->m_Discount / 10.0f) * Quantity;
	S32 nTimerId = 0;
	S32 nTimerTimes = 0;
	S32 nTimerTime = 0;
	//黄金
	if (pShopData->m_SaleType == 1)
	{
		if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -nCostMoney))
			return OPERATOR_HASNO_GOLD;
	}

	//限购条件
	if (pShopData->m_LmtScale != 0)
	{
		//vip等级
	}

	if (pShopData->m_BuyTimes != 0)
	{
		for (auto nShopTimerId : s_ShopTimer)
		{
			TimeSetRef pTimer = TIMESETMGR->GetData(nPlayerID, nShopTimerId);
			if (!pTimer && nTimerId == 0)
			{
				nTimerId = nShopTimerId;
				nTimerTimes = 0;
				nTimerTime = Platform::getNextTime(Platform::INTERVAL_DAY) - _time32(NULL);
			}
			else if (pTimer && pTimer->Flag1 == nShopId)
			{
				nTimerId = nShopTimerId;
				nTimerTimes = atoi(pTimer->Flag.c_str());
				nTimerTime = Platform::getNextTime(Platform::INTERVAL_DAY) - _time32(NULL);
				break;
			}
		}

		nTimerTimes += Quantity;
		if (nTimerId == 0 || nTimerTimes > pShopData->m_BuyTimes)
			return OPERATOR_MAXLIMIT_SHOPTIMES;
	}

	if (nTimerId > 0)
		TIMESETMGR->AddTimeSet(nPlayerID, nTimerId, nTimerTime, fmt::sprintf("%d", nTimerTimes), nShopId);

	SERVER->GetPlayerManager()->AddGold(nPlayerID, -nCostMoney, LOG_GOLD_BUY);
	AddItem(nPlayerID, pShopData->m_ItemId, Quantity);
	return OPERATOR_NONE_ERROR;
}

enOpError CItemManager::UseAccCard(U32 nPlayerID, std::string UID, S32 TimerId, S32 Quantity /*= 1*/)
{
	if (0 == TimerId || Quantity <= 0) {
		return OPERATOR_PARAMETER_ERROR;
	}

	ItemInfoRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASNO_ITEM;

	CItemData* pItemData = g_ItemDataMgr->getData(pData->ItemID);
	if (!pItemData)
		return OPERATOR_HASNO_ITEM;

	if (Quantity <= 0)
		return OPERATOR_PARAMETER_ERROR;

	if (pData->Quantity < Quantity)
		return OPERATOR_HASNO_ITEM;

	if (!pItemData->IsAccCard())
		return OPERATOR_PARAMETER_ERROR;

	pData->Quantity -= Quantity;
	if (pData->Quantity == 0)
	{
		ItemInfoRef pData1 = pData;
		std::string UID = pData1->UID;
		DeleteDB(nPlayerID, UID);
		UpdateToClient(nPlayerID, pData1);
	}
	else
	{
		UpdateDB(pData);
		UpdateToClient(nPlayerID, pData);
	}

	U32 AccTime = 0;
	switch (pData->ItemID)
	{
	case 600106:
		AccTime = 5 * 60;
		break;
	case 600107:
		AccTime = 30 * 60;
		break;
	case 600108:
		AccTime = 1 * 60 * 60;
		break;
	case 600109:
		AccTime = 4 * 60 * 60;
		break;
	}
	AccTime = AccTime * Quantity;

	enOpError nError = TIMESETMGR->AdvanceTimeUseOther(nPlayerID, TimerId, AccTime);

	return nError;
}

enOpError CItemManager::UseAddPVPTimesCard(U32 nPlayerID, std::string UID, S32 Quantity /*= 1*/)
{
	if (Quantity <= 0) {
		return OPERATOR_PARAMETER_ERROR;
	}

	ItemInfoRef pData = GetData(nPlayerID, UID);
	if (!pData)
		return OPERATOR_HASNO_CARD;

	CItemData* pItemData = g_ItemDataMgr->getData(pData->ItemID);
	if (!pItemData)
		return OPERATOR_HASNO_CARD;

	if (Quantity <= 0)
		return OPERATOR_HASNO_CARD;

	if (pData->Quantity < Quantity)
		return OPERATOR_HASNO_CARD;

	if (!pItemData->IsAddPVPTimesCard())
		return OPERATOR_PARAMETER_ERROR;

	S8 times = PVEMGR->GetRemainingMaxPVPTimes(nPlayerID);
	if (times <= 0)
		return OPERATOR_PVE_MAXTIMES;
	else if (times < Quantity)
		Quantity = times;

	pData->Quantity -= Quantity;
	if (pData->Quantity == 0)
	{
		ItemInfoRef pData1 = pData;
		std::string UID = pData1->UID;
		DeleteDB(nPlayerID, UID);
		UpdateToClient(nPlayerID, pData1);
	}
	else
	{
		UpdateDB(pData);
		UpdateToClient(nPlayerID, pData);
	}

	return PVEMGR->AddPVPTimes(nPlayerID, Quantity);
}

enOpError CItemManager::EquipStrengthen(U32 nPlayerID, std::string EquipUID[101], std::string CardUID) {
	static const S32 MaxSlot = 101;
	ItemInfoRef EquipList[MaxSlot];
	CItemData*  EquipDataList[MaxSlot];
	CEquipStrengthenData* EquipStrengthenDataList[MaxSlot];
	memset(EquipDataList, 0, sizeof(EquipDataList));

	//std::string EquipUID[MaxSlot] = { UID, UID1, UID2, UID3, UID4, UID5 };
	if (EquipUID[0] == "" || EquipUID[1] == "")//强化和消耗不能为空
		return OPERATOR_PARAMETER_ERROR;

	for (auto i = 0; i < MaxSlot; ++i) {
		if (EquipUID[i] != "") {
			EquipList[i] = GetData(nPlayerID, EquipUID[i]);
			if (!EquipList[i]) {
				//continue;
				return OPERATOR_PARAMETER_ERROR;
			}

			if (i != 0 && EquipUID[i] == EquipUID[0]) {
				return OPERATOR_PARAMETER_ERROR;
			}

			EquipDataList[i] = g_ItemDataMgr->getData(EquipList[i]->ItemID);
			if (!EquipDataList[i])
				return OPERATOR_PARAMETER_ERROR;
			else {
				if(i == 0 && !EquipDataList[i]->isEquipMent())
					return OPERATOR_PARAMETER_ERROR;
				else if(!(EquipDataList[i]->isEquipMent() || EquipDataList[i]->isEquipStrengthenMaterial()))
					return OPERATOR_PARAMETER_ERROR;
			}

			EquipStrengthenDataList[i] = g_EquipStrengthenDataMgr->getData(EquipList[i]->ItemID, EquipList[i]->Level);
			if (!EquipStrengthenDataList[i])
				return OPERATOR_PARAMETER_ERROR;
		}
	}

	//升级到慢
	if (EquipList[0]->Level == 10)
		return OPERATOR_PARAMETER_ERROR;

	EquipStrengthenDataList[0] = g_EquipStrengthenDataMgr->getData(EquipList[0]->ItemID, EquipList[0]->Level + 1);

	S32 nAddExp = EquipList[0]->Exp;
	S32 nLevel = EquipList[0]->Level;
	S32 nColor = EquipDataList[0]->m_Color;
	//增加的经验
	for (auto i = 1; i < MaxSlot; ++i) {
		if (EquipDataList[i] && EquipList[i]) {
			nAddExp += EquipStrengthenDataList[i]->m_Exp;
			//nAddExp += s_EquipStrengthenExp[EquipDataList[i]->m_Color][EquipList[i]->Level];
		}
	}

	do
	{
		/*if (nAddExp >= s_EquipStrengthen[nColor][nLevel]) {
			nAddExp -= s_EquipStrengthen[nColor][nLevel];
			nLevel = mClamp(nLevel + 1, 0, 10);
		}*/
		if (nAddExp >= EquipStrengthenDataList[0]->m_LevelExp) {
			nAddExp -= EquipStrengthenDataList[0]->m_LevelExp;
			nLevel = mClamp(nLevel + 1, 0, 10);
		}
		else {
			break;
		}

		if (nLevel >= 10) {
			nAddExp = 0;
			break;
		}
		else {
			EquipStrengthenDataList[0] = g_EquipStrengthenDataMgr->getData(EquipList[0]->ItemID, nLevel + 1);
		}
	} while (nAddExp);

	EquipList[0]->Level = nLevel;
	EquipList[0]->Exp = nAddExp;
	UpdateDB(EquipList[0]);
	UpdateToClient(nPlayerID, EquipList[0]);
	//删除材料
	for (auto i = 1; i < MaxSlot; ++i) {
		if (EquipList[i]) {
			DeleteDB(nPlayerID, EquipUID[i]);
			EquipList[i]->Quantity = 0;
			UpdateToClient(nPlayerID, EquipList[i]);
			DeleteData(nPlayerID, EquipUID[i]);
		}
	}

	if (CardUID != "")
	{
		CARDMGR->CaculateStats(nPlayerID, CardUID, true);
	}

	return OPERATOR_NONE_ERROR;
}

enOpError CItemManager::BuyRandomShop(U32 nPlayerID, S32 nSlot) {
	if (nSlot > 9 || nSlot < 0)
		return OPERATOR_PARAMETER_ERROR;

	auto nTimeID = nSlot / 6 + DINNER_SHOP_TIME1;
	auto nSlotID = nSlot % 6;

	S32 nVal = GetTimesInfo(nPlayerID, nTimeID, nSlotID);
	S32 nId = (nVal >> 16) & 0xFFFF;
	S32 nBuyTime = nVal & 0xFFFF;

	CRandomShopData* pData = g_RandomShopDataMgr->getData(nId);
	if (!pData)
		return OPERATOR_PARAMETER_ERROR;

	switch (pData->m_SaleType)
	{
	case 1: {
		if (nBuyTime != 0) {
			return OPERATOR_ALREADY_BUY;
		}
		else if (!PLAYERINFOMGR->CanAddValue(nPlayerID, CashGift, -pData->m_Sale)) {
			return OPERATOR_HASNO_COIN;
		}
		else {
			PLAYERINFOMGR->AddCashGift(nPlayerID, -pData->m_Sale);
			nBuyTime++;
			nVal = (nId << 16) & 0xFFFF0000 | (nBuyTime & 0xFFFF);
			SetTimesInfoEx1(nPlayerID, nTimeID, nSlotID, nVal, -1);
			AddItem(nPlayerID, pData->m_ItemId, pData->m_ItemNum);
			return OPERATOR_NONE_ERROR;
		}
	}
			break;
	default:
		break;
	}

	return OPERATOR_PARAMETER_ERROR;
}

auto ReflashRandom = [](U32 nPlayerID) {
	S32 nSlot = 0;
	auto nNextDay = Platform::getNextTime(Platform::INTERVAL_DAY);
	nNextDay -= _time32(NULL);
	Vector<int> Vec = g_RandomShopDataMgr->GetRandomShopEx();
	stTimeSetInfo info[2];
	S32 TimeID[2] = { 0, 0 };
	for (auto itr = Vec.begin(); itr != Vec.end(); ++itr, ++nSlot) {
		auto nID = nSlot / 6;
		auto nTimeID = nSlot / 6 + DINNER_SHOP_TIME1;
		auto nSlotID = nSlot % 6;
		info[nID].nVal[nSlotID] = (*itr) << 16 & 0xFFFF0000;
		TimeID[nID] = nTimeID;
	}

	for (auto i = 0; i < 2; ++i) {
		SetTimesInfoEx(nPlayerID, TimeID[i], info[i], nNextDay);
	}
};

enOpError CItemManager::OpenRandomShop(U32 nPlayerID) {
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, DINNER_SHOP_TIME1);
	//刷新商店
	if (!pTime)
	{
		ReflashRandom(nPlayerID);
	}
	return OPERATOR_NONE_ERROR;
}

enOpError CItemManager::ReflashRandomShop(U32 nPlayerID) {
	if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -20))
		return OPERATOR_HASNO_GOLD;

	ReflashRandom(nPlayerID);
	SERVER->GetPlayerManager()->AddGold(nPlayerID, -20, LOG_GOLD_DRAW);
	return OPERATOR_NONE_ERROR;
}

enOpError CItemManager::DrawItem(U32 nPlayerID, S8 nType, std::vector<DROPITEM_PAIR>& ItemVec) {
	if (nType < 1 || nType > 4)
		return OPERATOR_PARAMETER_ERROR;

	//龙魂，征服首次单抽免费
	if(!((1 == nType && NGUIDE->IsFirstDragonSpirit(nPlayerID)) || (3 == nType && NGUIDE->IsFirstConquer(nPlayerID))))
	{
		auto nItemId = 0;
		if (nType == 1 || nType == 2) {
			if (nType == 1 && CanReduceItem(nPlayerID, DRAW_ITEM_GOLD_ONE_ITEM, -1)) {
				nItemId = DRAW_ITEM_GOLD_ONE_ITEM;
			}
			else if (nType == 2 && CanReduceItem(nPlayerID, DRAW_ITEM_GOLD_TEN_ITEM, -1)) {
				nItemId = DRAW_ITEM_GOLD_TEN_ITEM;
			}
			else if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -s_ItemShop[nType][1]))
				return OPERATOR_HASNO_GOLD;
		}
		else {
			if (!PLAYERINFOMGR->CanAddValue(nPlayerID, Conquest, -s_ItemShop[nType][1]))
				return OPERATOR_HASNO_CONQUEST;
		}

		if (nType == 1 || nType == 2) {
			if (nItemId) {
				AddItem(nPlayerID, nItemId, -1);
			}
			else
				SERVER->GetPlayerManager()->AddGold(nPlayerID, -s_ItemShop[nType][1], LOG_GOLD_DRAW);
		}
		else {
			PLAYERINFOMGR->AddConquest(nPlayerID, -s_ItemShop[nType][1]);
		}
	}

	auto nTimes = 1;
	if (nType == 2 || nType == 4) {
		nTimes = 10;
	}
	else if (nType == 3) {
		nTimes = 2;
	}
	
	for (auto i = 0; i < nTimes; ++i) {
		DROPITEM_PAIR dropPair = DropItem(nPlayerID, s_ItemShop[nType][0]);
		ItemVec.push_back(dropPair);
	}

	AddItem(nPlayerID, s_ItemShop[nType][2], s_ItemShop[nType][3]);

	return OPERATOR_NONE_ERROR;
}

DB_Execution* CItemManager::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(fmt::sprintf("SELECT UID, PlayerID, ItemID, Quantity, Level, Exp FROM Tbl_Item WHERE PlayerID=%d", nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void* pH) {
		DB_Execution* pHandle = (DB_Execution*)(pH);
		if (pHandle && error == NONE_ERROR)
		{
			while (pHandle->More())
			{
				ItemInfoRef pItem = ItemInfoRef(new stItemInfo);
				dStrcpy(pItem->UID, MAX_UUID_LENGTH, pHandle->GetString());
				pItem->PlayerID = pHandle->GetInt();
				pItem->ItemID = pHandle->GetInt();
				pItem->Quantity = pHandle->GetInt();
				pItem->Level = pHandle->GetInt();
				pItem->Exp = pHandle->GetInt();
				m_PlayerMap[id].insert(DATA_MAP::value_type(pItem->UID, pItem));
			}
		}
	});
	return pHandle;
}

bool CItemManager::UpdateDB(ItemInfoRef pItem)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Item SET Quantity=%d, Level=%d, Exp=%d WHERE UID = '%s'", \
		pItem->Quantity, pItem->Level, pItem->Exp, pItem->UID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CItemManager::InsertDB(U32 nPlayerID, ItemInfoRef pItem)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("INSERT INTO  Tbl_Item \
			 (UID,		PlayerID,		ItemID,			Quantity) \
	   VALUES('%s',		%d,				%d,				%d)", \
		pItem->UID, nPlayerID, pItem->ItemID, pItem->Quantity));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CItemManager::DeleteDB(U32 nPlayerID, std::string UID)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE Tbl_Item  WHERE UID = '%s'", \
		UID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

void CItemManager::UpdateToClient(U32 nPlayerID, ItemInfoRef pItem)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_ITEM_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pItem->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CItemManager::SendInitToClient(U32 nPlayerID)
{
	const S32 nMaxSlot = 1000;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		DATA_MAP& ItemMap = m_PlayerMap[nPlayerID];
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		S32 nItemNum = 0;
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_ITEM_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(ItemMap.size() > nMaxSlot ? nMaxSlot : ItemMap.size(), Base::Bit16);
		for (DATA_ITR itr = ItemMap.begin(); itr != ItemMap.end(); ++itr, ++nItemNum)
		{
			if (nItemNum >= nMaxSlot)
				break;

			if (sendPacket.writeFlag(itr->second != NULL))
			{
				itr->second->WriteData(&sendPacket);
			}
		}

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

bool CItemManager::onPlayerEnter(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	LoadDB(pHead->Id);
	return true;
}

bool CItemManager::onPlayerLeave(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	RemovePlayer(pHead->Id);
	return true;
}

bool CItemManager::HandleClientUseItemRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	S32 Quantity = Packet->readInt(Base::Bit16);
	std::string UID1 = Packet->readString(MAX_UUID_LENGTH);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		if (!UseItem(pAccount->GetPlayerId(), UID, Quantity, UID1))
		{
			nError = 1;
		}

		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ITEM_USE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CItemManager::HandleClientComposeItemRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nID = Packet->readInt(Base::Bit8);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		if (!ComposeItem(pAccount->GetPlayerId(), nID))
		{
			nError = 1;
		}

		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ITEM_COMPOSE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CItemManager::HandleClientBuyItemRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nShopId = Packet->readInt(Base::Bit8);
	U32 nQuantity = Packet->readInt(Base::Bit8);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = BuyItem(pAccount->GetPlayerId(), nShopId, nQuantity);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ITEM_BUY_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CItemManager::HandleClientUseAccCardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	S32 Quantity = Packet->readInt(Base::Bit32);
	S32 TimeId = Packet->readInt(Base::Bit16);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = UseAccCard(pAccount->GetPlayerId(), UID, TimeId, Quantity);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ITEM_USEACCCARD_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CItemManager::HandleClientUseAddPVPTimesRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	U32 Quantity = Packet->readInt(Base::Bit8);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = UseAddPVPTimesCard(pAccount->GetPlayerId(), UID, Quantity);

		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ITEM_USEBATTLECARD_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CItemManager::HandleClientBuyRandomShopRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	U32 nSlot = Packet->readInt(Base::Bit8);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = BuyRandomShop(pAccount->GetPlayerId(), nSlot);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_BUY_RANDOMSHOP_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CItemManager::HandleClientOpenRandomShopRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = OpenRandomShop(pAccount->GetPlayerId());
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_OPEN_RANDOMSHOP_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CItemManager::HandleClientReflashRandomShopRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = ReflashRandomShop(pAccount->GetPlayerId());
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_REFLASH_RANDOMSHOP_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CItemManager::HandleClientDrawItemRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	U32 nType = Packet->readInt(Base::Bit8);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		std::vector<DROPITEM_PAIR> ItemVec;
		nError = DrawItem(pAccount->GetPlayerId(), nType, ItemVec);
		CMemGuard Buffer(4096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 4096);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_DRAWITEM_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(ItemVec.size(), Base::Bit16);
		for (auto itr = ItemVec.begin(); itr != ItemVec.end(); ++itr) {
			sendPacket.writeFlag(true);
			sendPacket.writeInt(itr->first, Base::Bit32);
			sendPacket.writeInt(itr->second, Base::Bit32);
		}
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CItemManager::HandleClientEquipStrengthenRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	std::string Equip[101];
	Equip[0] = Packet->readString();
	std::string CardUID  = Packet->readString();
	S32 nLen = Packet->readInt(Base::Bit16);
	nLen = mClamp(nLen, 0, 100);
	for (auto i = 1; i < nLen+1; ++i) {
		if (Packet->readFlag()) {
			Equip[i] = Packet->readString();
		}
	}

	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = EquipStrengthen(pAccount->GetPlayerId(), Equip, CardUID);
		CMemGuard Buffer(8192 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 8192);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_EquipStrengthen_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeString(Equip[0]);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

void ___UseAccCard(int id, std::string itemid, int Quantity/*=1*/, int timerId/*=""*/)
{
	ITEMMGR->UseAccCard(id, itemid, timerId, Quantity);
}

void ___UseBattleCard(int id, std::string itemid, int Quantity/*=1*/)
{
	ITEMMGR->UseAddPVPTimesCard(id, itemid, Quantity);
}

void ___UseItem(int id, int nItemId, int Quantity)
{
	ITEMMGR->UseItem(id, nItemId, Quantity);
}

void LoginItem(int playerid)
{
	ITEMMGR->LoadDB(playerid);
}

void ___AddItem(int id, int nItemId, int Quantity)
{
	ITEMMGR->AddItem(id, nItemId, Quantity);
}

void ___BuyItem(int id, int nItemId, int Quantity)
{
	ITEMMGR->BuyItem(id, nItemId, Quantity);
}

void CItemManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "UseBattleCard", &___UseBattleCard);
	lua_tinker::def(L, "UseAccCard", &___UseAccCard);
	lua_tinker::def(L, "AddItem", &___AddItem);
	lua_tinker::def(L, "UseItem", &___UseItem);
	lua_tinker::def(L, "LoginItem", &LoginItem);
	lua_tinker::def(L, "BuyItem", &___BuyItem);
}
