#ifndef _UTILITY_ITEM_EQUIPMANAGER_H_
#define _UTILITY_ITEM_EQUIPMANAGER_H_
#include "base/types.h"
#include "../../AccountHandler.h"
#include "Common/PlayerStruct.h"

class EquipManager
{
public:
	enum enEquipPos
	{
		EQUIP_WEAPON_POS				 = 0,//武器
		EQUIP_DEFENSE_POS				 = 1,//防具
		EQUIP_SECONDARY_POS				 = 2,//副手装备
		EQUIP_MAX_POS					 = 3,
	};

	EquipManager()
	{
	}

	~EquipManager()
	{
	}

	static EquipManager* getInstance()
	{
		static EquipManager s_cEquipMgr;
		return &s_cEquipMgr;
	}

	//穿上装备
	bool mountEquip(AccountRef pAccount, int SocketHandle, stItemInfo* stItem);

	//卸下装备
	bool unmoutEquip(AccountRef pAccount, int SocketHandle, stItemInfo* stItem);

	//获取装备的部位
	enEquipPos getEquipPose(U32 nItemId);
};

#define  EQUIPMGR EquipManager::getInstance()
#endif