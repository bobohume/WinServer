#include "../../stdafx.h"
#include "ItemManager.h"
#include "EquipManager.h"
#include "../../AccountHandler.h"
#include "../../WorldServer.h"
#include "../../PlayerMgr.h"
#include "../../data/ItemData.h"
#include "../../DB_SavePlayerItem.h"
#include "WINTCP/dtServerSocket.h"
#include "Common/UUId.h"
#include "Common/MemGuard.h"
#include "DropManager.h"
#include "../Script/lua_tinker.h"

CItemManager::CItemManager()
{
	registerEvent(CW_EQUIP_MOUNT_REQUEST,		&CItemManager::HandleClientMountEquipRequest);
	registerEvent(CW_EQUIP_UNMOUNT_REQUEST,		&CItemManager::HandleClientUnmountEquipRequest);
	registerEvent(CW_EQUIP_ADDEXP_REQUEST,		&CItemManager::HandleClientEquipAddExpRequest);
	registerEvent(CW_EQUIP_STRENGTHEN_REQUEST,	&CItemManager::HandleClientStrengthenEquipRequest);
	registerEvent(CW_EQUIP_COMPOSE_REQUEST,     &CItemManager::HandleClientComposeEquipRequest);
	registerEvent(CW_USE_ITEM_REQUEST,			&CItemManager::HandleClientUseItemRequset);
	registerEvent(CW_ADD_ITEM_REQUEST,		    &CItemManager::HandleClientAddItemRequset);
	registerEvent(CW_BANK_STORE_REQUEST,		&CItemManager::HandlePlayerBankStoreItemRequest);
	registerEvent(CW_BANK_GET_REQUEST,			&CItemManager::HandlePlayerBankGetItemRequest);
	registerEvent(CW_DROP_ITEM_REQUEST,			&CItemManager::HandleClientDropItemRequest);
	registerEvent(CW_PICK_ITEM_REQUEST,			&CItemManager::HandleClientPickItemRequest);
}

CItemManager::~CItemManager()
{

}

bool CItemManager::ItemAddExp(stItemInfo* stItem, AccountRef pAccount, int SocketHandle, U32 nExp/*= 1*/)
{
	if(!stItem || !pAccount)
		return false;

	CItemData* pData =  g_ItemDataMgr->getItemData(stItem->ItemID);
	if(!pData || !pData->isEquipMent())
		return false;

	if(pAccount)
	{
		stItem->Exp += nExp;
		//升级
		if(CanLevelUp(stItem))
		{
			return ItemLevelUp(stItem, pAccount, SocketHandle);
		}

		//通知客户端
		pAccount->SendClientItem(stItem, SocketHandle, ITEM_FLAG_EXP, NOTIFY_OPERATER_UPDATE, SHORTCUTTYPE_EQUIP);
		//更新到数据库
		pAccount->ItemSaveToDB(stItem, ITEM_FLAG_EXP, SHORTCUTTYPE_EQUIP);
		return true;
	}

	return false;
}

bool CItemManager::ItemLevelUp(stItemInfo* stItem, AccountRef pAccount, int SocketHandle)
{
	if(!stItem || !CanLevelUp(stItem) || !pAccount)
		return false;

	if(pAccount)
	{
		//升级
		if(stItem->Exp >= getLevelUpExp(stItem))
		{
			//U32 preExp = getLevelUpExp(stItem);
			if(stItem->Level + 1 >= EQUIP_PROMOTE_LEVEL)
			{
				return ItemPromote(stItem, pAccount, SocketHandle);
			}

			stItem->Level += 1;
			stItem->Exp = 0;
		}

		//通知客户端
		pAccount->SendClientItem(stItem, SocketHandle, ITEM_FLAG_EXP|ITEM_FLAG_LEVEL, NOTIFY_OPERATER_UPDATE, SHORTCUTTYPE_EQUIP);
		//更新到数据库
		pAccount->ItemSaveToDB(stItem, ITEM_FLAG_EXP|ITEM_FLAG_LEVEL, SHORTCUTTYPE_EQUIP);
		
		return true;
	}

	return false;
}

U32 CItemManager::getLevelUpExp(stItemInfo* stItem)
{
	switch (stItem->Quality)
	{
	case EQUIP_COLOR_WHITE:
		{
			if(stItem->Level ==  0)
				return 13;
			else if(stItem->Level == 1)
				return 45;
			else if(stItem->Level == 2)
				return 83;
			else if(stItem->Level == 3)
				return 126;
			else
				return 174;
		}
		break;
	case EQUIP_COLOR_GREEN:
		{
			if(stItem->Level ==  0)
				return 224;
			else if(stItem->Level == 1)
				return 278;
			else if(stItem->Level == 2)
				return 334;
			else if(stItem->Level == 3)
				return 393;
			else
				return 452;
		}
		break;
	case EQUIP_COLOR_BLUE:
		{
			if(stItem->Level ==  0)
				return 516;
			else if(stItem->Level == 1)
				return 580;
			else if(stItem->Level == 2)
				return 646;
			else if(stItem->Level == 3)
				return 714;
			else
				return 783;
		}
		break;
	case EQUIP_COLOR_PURPLE:
		{
			if(stItem->Level ==  0)
				return 854;
			else if(stItem->Level == 1)
				return 927;
			else if(stItem->Level == 2)
				return 1000;
			else if(stItem->Level == 3)
				return 1075;
			else
				return 1151;
		}
		break;
	case EQUIP_COLOR_ORANGE:
		{
			if(stItem->Level ==  0)
				return 1229;
			else if(stItem->Level == 1)
				return 1306;
			else if(stItem->Level == 2)
				return 1386;
			else if(stItem->Level == 3)
				return 1466;
			else
				return 1550;
		}
		break;
	case EQUIP_COLOR_RED:
		{
			return U32_MAX;
		}
		break;
	default:
		{
			return U32_MAX;
		}
		break;
	}

	return U32_MAX;
}

bool CItemManager::CanLevelUp(stItemInfo* stItem)
{
	if(!stItem)
		return false;

	CItemData* pData =  g_ItemDataMgr->getItemData(stItem->ItemID);
	if(!pData || !pData->isEquipMent())
		return false;

	U32 nLvUpExp = getLevelUpExp(stItem);
	if(nLvUpExp <= stItem->Exp)
	{
		return true;
	}
	return false;
}

bool CItemManager::ItemPromote(stItemInfo* stItem, AccountRef pAccount, int SocketHandle)
{
	if(!pAccount || !stItem || !CanPromote(stItem))
		return false;

	if(stItem->Quality < EQUIP_COLOR_WHITE || stItem->Quality >= EQUIP_COLOR_RED)
		return false;

	//装备颜色变更

	if(pAccount)
	{
		//升级
		stItem->Quality += 1;
		stItem->Level	= 1;
		stItem->Exp		= 0;

		//通知客户端
		pAccount->SendClientItem(stItem, SocketHandle, ITEM_FLAG_QUALITY|ITEM_FLAG_LEVEL, NOTIFY_OPERATER_UPDATE, SHORTCUTTYPE_EQUIP);
		//更新到数据库
		pAccount->ItemSaveToDB(stItem, ITEM_FLAG_QUALITY|ITEM_FLAG_LEVEL, SHORTCUTTYPE_EQUIP);
		return true;
	}

	return false;
}

bool CItemManager::CanPromote(stItemInfo* stItem)
{
	if(!stItem)
		return false;

	CItemData* pData =  g_ItemDataMgr->getItemData(stItem->ItemID);
	if(!pData || !pData->isEquipMent())
		return false;

	if(stItem->Level + 1 != EQUIP_PROMOTE_LEVEL)
		return false;

	return true;
}

bool CItemManager::ItemStrengthen(stItemInfo* stItem, AccountRef pAccount, int SocketHandle, U8 lv,enShortcutType slotType/*=SHORTCUTTYPE_EQUIP*/)
{
	if(!pAccount || !stItem || !CanStrengthen(stItem))
		return false;

	U32 nMaxStrengthen = 0, nMaxGemHole = 0;
	getEquipStrenthenAndGemHole(stItem, nMaxStrengthen, nMaxGemHole);
	if(stItem->Level >= nMaxStrengthen)
		return false;

	if (lv>=3)
	{
		lv = 3;
	}
	//装备颜色变更
	if(pAccount)
	{
		//升级
		stItem->EquipStrengthens += lv;

		//通知客户端
		pAccount->SendClientItem(stItem, SocketHandle, ITEM_FLAG_EQUIPSTRENGTHENS, NOTIFY_OPERATER_UPDATE, slotType);
		//更新到数据库
		pAccount->ItemSaveToDB(stItem, ITEM_FLAG_EQUIPSTRENGTHENS, slotType);

		//触发脚本
		lua_tinker::call<void>(L, "EquipStrengthen_Trigger", pAccount->GetPlayerId(), (U32)EQUIPMGR->getEquipPose(stItem->ItemID));
		return true;
	}
	return false;
}

bool CItemManager::CanStrengthen(stItemInfo* stItem)
{
	if(!stItem || !stItem->ItemID)
		return false;

	CItemData* pData =  g_ItemDataMgr->getItemData(stItem->ItemID);
	if(!pData || !pData->isEquipMent() || !(pData->m_Limit & CItemData::LIMIT_STRENGTHEN_FLAG))
		return false;

	return true;
}

bool CItemManager::ItemCompose(stItemInfo* stItem, AccountRef pAccount, int SocketHandle, std::string UID[MAX_COMPOSE_SIZE], enShortcutType slotType/*=SHORTCUTTYPE_EQUIP*/)
{
	if(!stItem || !CanCompose(stItem))
		return false;

	CItemData* pItemData = g_ItemDataMgr->getItemData(stItem->ItemID);
	if(!pItemData)
		return false;

	bool bWeapon = pItemData->isWeapon();

	if(!pAccount)
		return false;

	stItemInfo* stItemArray[MAX_COMPOSE_SIZE];
	static U32 nComposeMatrialSize = 0;
	nComposeMatrialSize = 0;
	U32 nMaxStrengthen = 0, nMaxGemHole = 0;
	for(int i = 0; i < MAX_COMPOSE_SIZE; ++i)
	{
		stItemArray[i] = NULL;
		if(UID[i].empty())
			continue;

		stItemArray[i] = pAccount->getInventory(UID[i]);
		if(stItemArray[i])
		{
			CItemData* pSubItemData = g_ItemDataMgr->getItemData(stItemArray[i]->ItemID);
			//类型是否一样
			if(!pSubItemData || !pSubItemData->isEquipMent())
				return false;

			if(bWeapon)
			{
				if(!pSubItemData->isWeapon())
					return false;
			}
			else
			{
				if(!pSubItemData->isDefense())
					return false;
			}

			++nComposeMatrialSize;
		}
	}

	if(nComposeMatrialSize == 0)
		return false;

	getEquipStrenthenAndGemHole(stItem, nMaxStrengthen, nMaxGemHole);
	for(int i = 0; i < MAX_COMPOSE_SIZE; ++i)
	{
		if(stItemArray[i])
		{
			//转移强化等级
			if(stItemArray[i]->EquipStrengthens != 0 && stItem->EquipStrengthens <  nMaxStrengthen)
			{
				if(stItem->EquipStrengthens + stItemArray[i]->EquipStrengthens <= nMaxStrengthen)
				{
					stItem->EquipStrengthens += stItemArray[i]->EquipStrengthens;//强化等级
				}
			}

			//转移特效
			if(stItem->getGemHoleNum() < nMaxGemHole)
			{
				TransferEmbedSlot(stItem, stItemArray[i]);
			}
		}
	}

	//扣除材料
	for(int i = 0; i < MAX_COMPOSE_SIZE; ++i)
	{
		if(!stItemArray[i])
			continue;

		U32 nMask = 0;
		//通知客户端
		pAccount->SendClientItem(stItemArray[i], SocketHandle, nMask, NOTIFY_OPERATER_DEL, SHORTCUTTYPE_INVENTORY);
		//更新到数据库
		pAccount->ItemSaveToDB(stItemArray[i], nMask, SHORTCUTTYPE_INVENTORY, false, true);

		//清空内存
		stItemArray[i]->InitData();
	}

	U32 nMask = ITEM_FLAG_EQUIPSTRENGTHENS|ITEM_FLAG_EMBEDSLOT0|ITEM_FLAG_EMBEDSLOT1|ITEM_FLAG_EMBEDSLOT2|ITEM_FLAG_EMBEDSLOT3\
		|ITEM_FLAG_EMBEDSLOT4|ITEM_FLAG_EMBEDSLOT5|ITEM_FLAG_EMBEDSLOT6|ITEM_FLAG_EMBEDSLOT7|ITEM_FLAG_EMBEDSLOT8\
		|ITEM_FLAG_EMBEDSLOT9;

	//通知客户端
	pAccount->SendClientItem(stItem, SocketHandle, nMask, NOTIFY_OPERATER_UPDATE, slotType);
	//更新到数据库
	pAccount->ItemSaveToDB(stItem, nMask, slotType);
	pAccount->SendClientComposeFinished(SocketHandle);
	//触发脚本
	lua_tinker::call<void>(L, "EquipSynthesis_Trigger", pAccount->GetPlayerId());
	return true;
}

bool CItemManager::CanCompose(stItemInfo* stItem)
{
	if(!stItem)
		return false;

	CItemData* pData =  g_ItemDataMgr->getItemData(stItem->ItemID);
	if(!pData || !pData->isEquipMent())
		return false;

	return true;
}

bool CItemManager::TransferEmbedSlot(stItemInfo* stItem, stItemInfo* stSubItem)
{
	if(!stItem || !stSubItem)
		return false;

	DEQ_U32 EmptyDeq;//找空栏位
	SET_U32 EmbedSet;//已经有的特效不能重复
	for(int i = 0; i < MAX_EMBEDSLOTS; ++i)
	{
		if(stItem->EmbedSlot[i] == 0xFFFFFFFF)
			EmptyDeq.push_back(i);
		else
			EmbedSet.insert(stItem->EmbedSlot[i]);
	}

	for(int i = 0; i < MAX_EMBEDSLOTS; ++i)
	{
		if(stSubItem->EmbedSlot[i] != 0xFFFFFFFF)
		{
			if(EmbedSet.find(stSubItem->EmbedSlot[i]) == EmbedSet.end())
			{
				U32 nSLot = EmptyDeq.front();
				EmptyDeq.pop_front();
				stItem->EmbedSlot[nSLot] = stSubItem->EmbedSlot[i];
			}
		}
		else
		{
			continue;
		}
	}

	return true;
}

void CItemManager::getEquipStrenthenAndGemHole(stItemInfo* stItem, U32& nMaxStrengthen, U32& nMaxGemHole)
{
	if(!stItem)
	{
		nMaxGemHole = 0;
		nMaxStrengthen = 0;
		return;
	}

	switch (stItem->Quality)
	{
	case EQUIP_COLOR_WHITE:
		{
			nMaxStrengthen = 10;
			nMaxGemHole	= 0;
		}
		break;
	case EQUIP_COLOR_GREEN:
		{
			nMaxStrengthen = 25;
			nMaxGemHole	= 2;
		}
		break;
	case EQUIP_COLOR_BLUE:
		{
			nMaxStrengthen = 40;
			nMaxGemHole	= 4;
		}
		break;
	case EQUIP_COLOR_PURPLE:
		{
			nMaxStrengthen = 60;
			nMaxGemHole	= 6;
		}
		break;
	case EQUIP_COLOR_ORANGE:
		{
			nMaxStrengthen = 80;
			nMaxGemHole	= 8;
		}
		break;
	case EQUIP_COLOR_RED:
		{
			nMaxStrengthen = 99;
			nMaxGemHole	= 10;
		}
		break;
	default:
		{
			nMaxGemHole = 0;
			nMaxStrengthen = 0;
		}
		break;
	}
}

stItemInfo* CItemManager::CreateItem(U32 nItemId, U32 nQuantity/* =1*/)
{
	CItemData* pData =  g_ItemDataMgr->getItemData(nItemId);
	if(!pData)
		return NULL;


	stItemInfo* stItem = new stItemInfo();
	stItem->InitData();

	dStrcpy(stItem->UID,  MAX_UUID_LENGTH, uuid());
	stItem->ItemID			= nItemId;
	stItem->Quantity		= nQuantity;
	if(pData->m_Skill > 0)
		stItem->EmbedSlot[0]	= pData->m_Skill;
	stItem->Quality			= EQUIP_COLOR_WHITE;
	stItem->Level			= 1;

	if(pData->isEquipMent())
	{
		stItem->ActiveFlag	|= EAPF_ATTACH;
	}

	return stItem;
}

bool CItemManager::UseItem(stItemInfo* stItem, AccountRef pAccount, int SocketHandle)
{
	
	CItemData* pData =  g_ItemDataMgr->getItemData(stItem->ItemID);

	if(!pData || !(pData->m_Limit & CItemData::LIMIT_USE_FLAG) || pData->isEquipMent())
	{
		return false;
	}
	else
	{
		pAccount->ItemSaveToDB(stItem, 0, SHORTCUTTYPE_INVENTORY, false, true);
		pAccount->SendClientItem(stItem, SocketHandle,0, NOTIFY_OPERATER_DEL, SHORTCUTTYPE_INVENTORY);
	}
	return true;
}

bool CItemManager::AddItem(U32 nItemId,  AccountRef pAccount, int SocketHandle, U32 nQuantity/*= 1*/)
{
	CItemData* pData =  g_ItemDataMgr->getItemData(nItemId);
	if(!pData || stItemInfo::CheckItemID(nItemId) || (pData->m_Limit & CItemData::LIMIT_NO_SAVE))
		return false;

	if(!pAccount)
		return false;

	PlayerDataRef pPlayerData =  pAccount->GetPlayerData();
	if(!pPlayerData)
		return false;

	S32 iSlot = pAccount->findEmptyShortcut(SHORTCUTTYPE_INVENTORY);
	if(iSlot == -1 || iSlot >= INVENTROY_MAXSLOTS)
		return false;


	stItemInfo* pNewItem =  CreateItem(nItemId, 1);
	if(!pNewItem)
		return false;

	pPlayerData->MainData.InventoryInfo[iSlot] = *pNewItem;
	pAccount->ItemSaveToDB(&pPlayerData->MainData.InventoryInfo[iSlot], DB_UPDATE_ALL_MASK, SHORTCUTTYPE_INVENTORY, true);
	//通知客户端
	pAccount->SendClientItem(&pPlayerData->MainData.InventoryInfo[iSlot], pAccount->GetGateSocketId(), DB_UPDATE_ALL_MASK, NOTIFY_OPERATER_ADD, SHORTCUTTYPE_INVENTORY);
	SAFE_DELETE(pNewItem);
	//低下是可叠层处理，暂时删除掉
	/*Player::PAIRVEC emptyVec = pAccount->findEmptyShortcut(nItemId, nQuantity);
	if(emptyVec.empty())
		return false;

	for(Player::PAIRVECITR itr = emptyVec.begin(); itr != emptyVec.end(); ++itr)
	{
		if(strcmp(pPlayerData->MainData.InventoryInfo[(*itr).first].UID, "") == 0)
		{
			stItemInfo* pNewItem =  ITEMMGR->CreateItem(nItemId, 1);
			if(!pNewItem)
				return false;

			pPlayerData->MainData.InventoryInfo[(*itr).first] = *pNewItem;
			pAccount->ItemSaveToDB(&pPlayerData->MainData.InventoryInfo[(*itr).first], DB_UPDATE_ALL_MASK, SHORTCUTTYPE_INVENTORY, true);
			//通知客户端
			pAccount->SendClientItem(&pPlayerData->MainData.InventoryInfo[(*itr).first], SocketHandle, DB_UPDATE_ALL_MASK, NOTIFY_OPERATER_ADD, SHORTCUTTYPE_INVENTORY);

			SAFE_DELETE(pNewItem);
		}
		else
		{
			pPlayerData->MainData.InventoryInfo[(*itr).first].Quantity += (*itr).second;
			pAccount->ItemSaveToDB(&pPlayerData->MainData.InventoryInfo[(*itr).first], ITEM_FLAG_QUANTITY, SHORTCUTTYPE_INVENTORY);
			//通知客户端
			pAccount->SendClientItem(&pPlayerData->MainData.InventoryInfo[(*itr).first], SocketHandle, ITEM_FLAG_QUANTITY, NOTIFY_OPERATER_UPDATE, SHORTCUTTYPE_INVENTORY);
		}

	}*/

	return true;
}

bool CItemManager::AddItemEx(stItemInfo* pItem, AccountRef pAccount)
{
	CItemData* pData =  g_ItemDataMgr->getItemData(pItem->ItemID);
	if(!pData || stItemInfo::CheckItemID(pItem->ItemID) || (pData->m_Limit & CItemData::LIMIT_NO_SAVE))
		return false;

	if(pItem->Quantity > pData->m_MaxDie)
		pItem->Quality = pData->m_MaxDie;

	if(!pAccount || !pItem)
		return false;

	PlayerDataRef pPlayerData =  pAccount->GetPlayerData();
	if(!pPlayerData)
		return false;

	S32 iSlot = pAccount->findEmptyShortcut(SHORTCUTTYPE_INVENTORY);
	if(iSlot == -1 || iSlot >= INVENTROY_MAXSLOTS)
		return false;

	
	pPlayerData->MainData.InventoryInfo[iSlot] = *pItem;
	pAccount->ItemSaveToDB(&pPlayerData->MainData.InventoryInfo[iSlot], DB_UPDATE_ALL_MASK, SHORTCUTTYPE_INVENTORY, true);
	//通知客户端
	pAccount->SendClientItem(&pPlayerData->MainData.InventoryInfo[iSlot], pAccount->GetGateSocketId(), DB_UPDATE_ALL_MASK, NOTIFY_OPERATER_ADD, SHORTCUTTYPE_INVENTORY);

	return true;
}

bool CItemManager::StoreItemToBank(UIDVEC& uidVec, AccountRef pAccount)
{
	U32 iNeedSlotNum = uidVec.size();
	if(iNeedSlotNum > 10 || iNeedSlotNum == 0)
		return false;

	if(!pAccount)
		return false;

	PlayerDataRef pPlayerData = pAccount->GetPlayerData();
	if(!pPlayerData)
		return false;

	S32 iBankSize = pAccount->getMaxShortcut(SHORTCUTTYPE_BANK);
	if(iBankSize == -1 || iBankSize >= BANK_MAXSLOTS)
		return false;

	if(iNeedSlotNum + iBankSize >= pPlayerData->MainData.PartData.BankSize)
		return false;

	std::vector<S32> iSlotVec;
	pAccount->findEmptyShortcut(SHORTCUTTYPE_BANK, iNeedSlotNum, iSlotVec);
	if(iSlotVec.empty())
		return false;

	for(UIDITR itr  = uidVec.begin(); itr != uidVec.end(); ++itr)
	{
		stItemInfo* pItem = pAccount->getInventory(*itr);
		if(pItem)
		{
			S32 iSlot = iSlotVec.back();
			iSlotVec.pop_back();
			//添加物品到仓库
			pPlayerData->MainData.BankInfo[iSlot] = *pItem;
			pAccount->ItemSaveToDB(pItem, DB_UPDATE_ALL_MASK, SHORTCUTTYPE_BANK, true, false);
			pAccount->SendClientItem(pItem, pAccount->GetGateSocketId(), DB_UPDATE_ALL_MASK, NOTIFY_OPERATER_ADD, SHORTCUTTYPE_BANK);
			//从物品栏删除
			pAccount->ItemSaveToDB(pItem, 0, SHORTCUTTYPE_INVENTORY, false, true);
			pAccount->SendClientItem(pItem, pAccount->GetGateSocketId(), 0, NOTIFY_OPERATER_DEL, SHORTCUTTYPE_INVENTORY);
			pItem->InitData();
		}
	}	

	iSlotVec.clear();
	return true;
}

bool CItemManager::GetItemFromBank(UIDVEC& uidVec, AccountRef pAccount)
{
	U32 iNeedSlotNum = uidVec.size();
	if(iNeedSlotNum > 10 || iNeedSlotNum == 0)
		return false;

	if(!pAccount)
		return false;

	PlayerDataRef pPlayerData = pAccount->GetPlayerData();
	if(!pPlayerData)
		return false;

	S32 iItemSize = pAccount->getMaxShortcut(SHORTCUTTYPE_INVENTORY);
	if(iItemSize == -1 || iItemSize >= INVENTROY_MAXSLOTS)
		return false;

	if(iNeedSlotNum + iItemSize >= pPlayerData->MainData.PartData.BagSize)
		return false;

	std::vector<S32> iSlotVec;
	pAccount->findEmptyShortcut(SHORTCUTTYPE_INVENTORY, iNeedSlotNum, iSlotVec);
	if(iSlotVec.empty())
		return false;

	for(UIDITR itr  = uidVec.begin(); itr != uidVec.end(); ++itr)
	{
		stItemInfo* pItem = pAccount->getShortcut<stItemInfo>(SHORTCUTTYPE_BANK, *itr);
		if(pItem)
		{
			S32 iSlot = iSlotVec.back();
			iSlotVec.pop_back();
			//添加物品栏
			pPlayerData->MainData.InventoryInfo[iSlot] = *pItem;
			pAccount->ItemSaveToDB(pItem, DB_UPDATE_ALL_MASK, SHORTCUTTYPE_INVENTORY, true, false);
			pAccount->SendClientItem(pItem, pAccount->GetGateSocketId(), DB_UPDATE_ALL_MASK, NOTIFY_OPERATER_ADD, SHORTCUTTYPE_INVENTORY);
			//删除仓库栏
			pAccount->ItemSaveToDB(pItem, 0, SHORTCUTTYPE_BANK, false, true);
			pAccount->SendClientItem(pItem, pAccount->GetGateSocketId(), 0, NOTIFY_OPERATER_DEL, SHORTCUTTYPE_BANK);
			pItem->InitData();
		}
	}	

	iSlotVec.clear();
	return true;
}


//-----------------------------------处理消息函数-----------------------------------------------
void CItemManager::HandleClientMountEquipRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	char tempStr[MAX_UUID_LENGTH] = "";
	Packet->readString(tempStr, MAX_UUID_LENGTH);
	std::string UID = tempStr;

	stItemInfo* pItem = pAccount->getShortcut<stItemInfo>(SHORTCUTTYPE_INVENTORY, UID);
	if(!pItem)
		return;

	EQUIPMGR->mountEquip(pAccount, SocketHandle, pItem);
	return;
}

void CItemManager::HandleClientUnmountEquipRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	char tempStr[MAX_UUID_LENGTH] = "";
	Packet->readString(tempStr, MAX_UUID_LENGTH);
	std::string UID = tempStr;

	stItemInfo* pItem = pAccount->getShortcut<stItemInfo>(SHORTCUTTYPE_EQUIP, UID);
	if(!pItem)
		return;

	EQUIPMGR->unmoutEquip(pAccount, SocketHandle, pItem);
	return;
}

void CItemManager::HandleClientEquipAddExpRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	char tempStr[MAX_UUID_LENGTH] = "";
	Packet->readString(tempStr, MAX_UUID_LENGTH);
	std::string UID = tempStr;

	stItemInfo* pItem = pAccount->getShortcut<stItemInfo>(SHORTCUTTYPE_EQUIP, UID);
	if(!pItem)
		return;

	ItemAddExp(pItem, pAccount, SocketHandle, 1);
	return;
}

void CItemManager::HandleClientStrengthenEquipRequest( int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet )
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	char tempStr[MAX_UUID_LENGTH] = "";
	Packet->readString(tempStr, MAX_UUID_LENGTH);
	U8 lv = Packet->readInt(Base::Bit8);
	std::string UID = tempStr;

	stItemInfo* pItem = pAccount->getShortcut<stItemInfo>(SHORTCUTTYPE_EQUIP, UID);
	if(!pItem)
		return;

	ItemStrengthen(pItem,pAccount,SocketHandle,lv);
}

void CItemManager::HandleClientComposeEquipRequest( int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet )
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	//读取主装备
	char tempStr[MAX_UUID_LENGTH] = "";
	Packet->readString(tempStr, MAX_UUID_LENGTH);
	std::string UID = std::string(tempStr);
	stItemInfo* pItem = pAccount->getShortcut<stItemInfo>(SHORTCUTTYPE_INVENTORY, UID);
	if(!pItem)
		return;

	//读取次装备数量
	S32 count=Packet->readInt(Base::Bit8);
	std::string equipUIDs[MAX_COMPOSE_SIZE];
	if (count<=0 || count>MAX_COMPOSE_SIZE)
	{
		return;
	}
	for (int i=0;i<count;++i )
	{  

		Packet->readString(tempStr,MAX_UUID_LENGTH);
		equipUIDs[i]=tempStr;
	}


	ItemCompose(pItem,pAccount,SocketHandle,equipUIDs, SHORTCUTTYPE_INVENTORY);
}

void CItemManager::HandleClientUseItemRequset( int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet )
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if (!pAccount) //失败 提示客户端刷新界面
	{
		return;
	}


	//读取物品
	char tempStr[MAX_UUID_LENGTH] = "";
	Packet->readString(tempStr, MAX_UUID_LENGTH);
	std::string UID = std::string(tempStr);
	stItemInfo* pItem = pAccount->getShortcut<stItemInfo>(SHORTCUTTYPE_INVENTORY, UID);
	if(!pItem)
	{
		goto notify;
	}

	bool result=UseItem(pItem,pAccount,SocketHandle);
	if (!result)
	{
		goto notify;
	}
	else
	{
		return;
	}
	{
notify:
		U32 AccountId = pAccount->GetAccountId();
		CMemGuard Buffer(256 MEM_GUARD_PARAM);
		char str[20];
		Base::BitStream SendPacket(Buffer.get(), 256);
		stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket, WC_USE_ITEM_RESPONSE, AccountId, SERVICE_CLIENT);
		SendPacket.writeFlag(false);
		pSendHead->PacketSize = SendPacket.getPosition() - IPacket::GetHeadSize();
		SERVER->GetServerSocket()->Send(SocketHandle, SendPacket);
		return;
	}
	return;
}

// 响应客户端additem
void CItemManager::HandleClientAddItemRequset( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if (!pAccount)
		return;
	int itemId = Packet->readInt(Base::Bit32);


	AddItem(itemId, pAccount, pAccount->GetGateSocketId(), 1);
	return;   
}

void CItemManager::HandlePlayerBankStoreItemRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	std::vector<std::string> StrVec;
	U32 iBatchNum =  Packet->readInt(Base::Bit8);
	if(iBatchNum > 10)
		return;

	for(int i = 0; i < iBatchNum; ++i)
	{
		char tempUID[MAX_UUID_LENGTH] = "";
		Packet->readString(tempUID, MAX_UUID_LENGTH);
		std::string UID = tempUID;
		StrVec.push_back(UID);
	}

	if(StrVec.size() != iBatchNum)
		return;

	StoreItemToBank(StrVec, pAccount);
}

void CItemManager::HandlePlayerBankGetItemRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	std::vector<std::string> StrVec;
	U32 iBatchNum =  Packet->readInt(Base::Bit8);
	if(iBatchNum > 10)
		return;

	for(int i = 0; i < iBatchNum; ++i)
	{
		char tempUID[MAX_UUID_LENGTH] = "";
		Packet->readString(tempUID, MAX_UUID_LENGTH);
		std::string UID = tempUID;
		StrVec.push_back(UID);
	}

	if(StrVec.size() != iBatchNum)
		return ;

	GetItemFromBank(StrVec, pAccount);
}

void CItemManager::HandleClientDropItemRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if (!pAccount)
		return;

	U32 nNpcId = Packet->readInt(Base::Bit32);
	U32 nRow = Packet->readInt(Base::Bit32);
	U32 nCol = Packet->readInt(Base::Bit32);
	U32 nItemId = 0;
	std::string UID;

	if(nNpcId == 0)
		return;

	U32 nDropEventId = DROPMGR->dropItem(nNpcId);
	if(nDropEventId == 0)
		return;

	if(!DROPMGR->doDropItem(nDropEventId, nPlayerId, nItemId, UID))
		return;

	if(nItemId > 0)
	{
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream SendPacket(Buffer.get(), 128);
		stPacketHead* pSendHead = IPacket::BuildPacketHead(SendPacket, WC_DROP_ITME_RESPONSE, pAccount->GetAccountId(), SERVICE_CLIENT, DROP_TYPE_NPC);
		SendPacket.writeString(UID.c_str(), MAX_UUID_LENGTH);
		SendPacket.writeInt(nItemId, Base::Bit32);
		SendPacket.writeInt(nRow, Base::Bit32);
		SendPacket.writeInt(nCol, Base::Bit32);

		pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
		SERVER->GetServerSocket()->Send(SocketHandle, SendPacket);
	}
	return;
}

void CItemManager::HandleClientPickItemRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if (!pAccount)
		return;

	char tempStr[MAX_UUID_LENGTH] = "";
	Packet->readString(tempStr, MAX_UUID_LENGTH);
	std::string UID = tempStr;
	U32 nNpcId = Packet->readInt(Base::Bit32);
	if(nNpcId == 0 || UID.empty())
		return;

	stDropInfo* pDropInfo = DROPMGR->getDropInfo(nPlayerId, UID);
	if(!pDropInfo)
		return;

	if(pDropInfo->nItemId == 0)
		return;

	//添加物品
	if(AddItem(pDropInfo->nItemId, pAccount, SocketHandle, 1))
	{
		//这里面发回一个消息
		DROPMGR->delDropInfo(nPlayerId, UID);
	}
	return;
}