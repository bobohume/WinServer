#include "EquipManager.h"
#include "ItemManager.h"
#include "../../AccountHandler.h"
#include "../../WorldServer.h"
#include "../../PlayerMgr.h"
#include "../../data/ItemData.h"
#include "../../DB_SavePlayerItem.h"

//穿上装备
bool EquipManager::mountEquip(AccountRef pAccount, int SocketHandle, stItemInfo* stItem)
{
	if(!stItem)
		return false;

	CItemData* pData = g_ItemDataMgr->getItemData(stItem->ItemID);
	if(!pData  || !(pData->isEquipMent() || pData->isSecondaryEquip()))
		return false;

	enEquipPos equipIndex = getEquipPose(pData->m_ItemId);
	if(equipIndex == EQUIP_MAX_POS)
		return false;

	if(!pAccount)
		return false;

	PlayerDataRef pPlayerData =  pAccount->GetPlayerData();
	if(!pPlayerData)
		return false;

	{
		if(strcmp(pPlayerData->MainData.EquipInfo[equipIndex].UID, "") != 0)
		{
			stItemInfo* pDelItem = &pPlayerData->MainData.EquipInfo[equipIndex];
			unmoutEquip(pAccount, SocketHandle, pDelItem);
		}

		pPlayerData->MainData.EquipInfo[equipIndex] = *stItem;
		pAccount->ItemSaveToDB(stItem, DB_UPDATE_ALL_MASK, SHORTCUTTYPE_EQUIP, true, false);
		pAccount->SendClientItem(stItem, SocketHandle, DB_UPDATE_ALL_MASK, NOTIFY_OPERATER_ADD, SHORTCUTTYPE_EQUIP);
		//删除物品栏数据
		{
			pAccount->SendClientItem(stItem, SocketHandle, 0, NOTIFY_OPERATER_DEL, SHORTCUTTYPE_INVENTORY);
			pAccount->ItemSaveToDB(stItem, 0, SHORTCUTTYPE_INVENTORY, false, true);
			stItem->InitData();
		}
		return true;
	}
	return false;
}

//卸下装备
bool EquipManager::unmoutEquip(AccountRef pAccount, int SocketHandle, stItemInfo* stItem)
{
	if(!stItem)
		return false;

	CItemData* pData = g_ItemDataMgr->getItemData(stItem->ItemID);
	if(!pData  || !(pData->isEquipMent() || pData->isSecondaryEquip()))
		return false;

	enEquipPos equipIndex = getEquipPose(pData->m_ItemId);
	if(equipIndex == EQUIP_MAX_POS)
		return false;

	if(!pAccount)
		return false;

	//找空栏位
	S32 iSLot = pAccount->findEmptyShortcut(SHORTCUTTYPE_INVENTORY);
	if(iSLot == -1 || iSLot >= INVENTROY_MAXSLOTS)
		return false;

	PlayerDataRef pPlayerData =  pAccount->GetPlayerData();
	if(!pPlayerData)
		return false;

	if(strcmp(pPlayerData->MainData.EquipInfo[equipIndex].UID, "") != 0)
	{
		//添加到物品栏
		{
			pPlayerData->MainData.InventoryInfo[iSLot] = *stItem;
			pAccount->SendClientItem(stItem, SocketHandle, DB_UPDATE_ALL_MASK, NOTIFY_OPERATER_ADD, SHORTCUTTYPE_INVENTORY);
			pAccount->ItemSaveToDB(stItem, DB_UPDATE_ALL_MASK, SHORTCUTTYPE_INVENTORY, true);
		}

		//通知客户端
		pAccount->SendClientItem(stItem, SocketHandle, 0, NOTIFY_OPERATER_DEL, SHORTCUTTYPE_EQUIP);
		//更新到数据库
		pAccount->ItemSaveToDB(stItem, 0, SHORTCUTTYPE_EQUIP, false, true);
		pPlayerData->MainData.EquipInfo[equipIndex].InitData();
		return true;
	}

	return false;
}

//获取装备的部位
EquipManager::enEquipPos EquipManager::getEquipPose(U32 nItemId)
{
	CItemData* pData = g_ItemDataMgr->getItemData(nItemId);
	if(!pData || !(pData->isEquipMent() || pData->isSecondaryEquip()))
		return EQUIP_MAX_POS;

	if(pData->isWeapon())
		return EQUIP_WEAPON_POS;
	else if(pData->isDefense())
		return EQUIP_DEFENSE_POS;
	else if(pData->isSecondaryEquip())
		return EQUIP_SECONDARY_POS;

	return EQUIP_MAX_POS;
}

