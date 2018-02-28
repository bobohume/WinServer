#ifndef _UTILITY_ITEM_ITEMMANAGER_H_
#define _UTILITY_ITEM_ITEMMANAGER_H_
#include "base/types.h"
#include "Common/PlayerStruct.h"
#include "../../AccountHandler.h"
#include "../../ManagerBase.h"
#include <set>
#include <vector>
#include <deque>
#include <string>

class CItemManager:public CManagerBase<CItemManager, MSG_ITEM_BEGIN, MSG_ITEM_END >
{
public:
	enum
	{
		EQUIP_PROMOTE_LEVEL				= 5,
	};

	enum enItemOPError
	{
		ITEM_OP_NONE					 = 0,
		ITEM_OP_UNKNOW					 = 1,
		ITEM_OP_ITEMSTRUCTERROR			 = 2,
		ITEM_OP_STRENGTHENLEVEL_LIMIT	 = 3,
		ITEM_OP_STRENGTHEN_SUCESS		 = 4,
	};

	typedef std::set<U32>			SET_U32;
	typedef SET_U32::iterator		SETITR_U32;
	typedef std::deque<U32>			DEQ_U32;
	typedef DEQ_U32::iterator		DEQITR_U32;
	typedef std::vector<std::string>UIDVEC;
	typedef UIDVEC::iterator        UIDITR;

	CItemManager();
	virtual ~CItemManager();

	//-------------------装备升级处理---------------------------------
	bool ItemAddExp(stItemInfo* stItem, AccountRef pAccount, int SocketHandle, U32 nExp = 1);
	bool ItemLevelUp(stItemInfo* stItem, AccountRef pAccount, int SocketHandle);//装备升级
	U32  getLevelUpExp(stItemInfo* stItem);				//获取装备升级经验
	bool CanLevelUp(stItemInfo* stItem);				//装备能否升级

	//装备进阶
	bool ItemPromote(stItemInfo* stItem, AccountRef pAccount, int SocketHandle);//装备进阶
	bool CanPromote(stItemInfo* stItem);				//能否进阶

	//获取装备可镶嵌宝石孔数和强化等级
	void getEquipStrenthenAndGemHole(stItemInfo* stItem, U32& nMaxStrengthen, U32& nMaxGemHole);

	//装备强化 lv强化的等级
	bool ItemStrengthen(stItemInfo* stItem, AccountRef pAccount, int SocketHandle,U8 lv=1, enShortcutType slotType =SHORTCUTTYPE_EQUIP);//装备强化
	bool CanStrengthen(stItemInfo* stItem);

	//装备合成
	bool ItemCompose(stItemInfo* stItem, AccountRef pAccount, int SocketHandle, std::string UID[MAX_COMPOSE_SIZE], enShortcutType slotType = SHORTCUTTYPE_EQUIP);
	bool CanCompose(stItemInfo* stItem);
	bool TransferEmbedSlot(stItemInfo* stItem, stItemInfo* stSubItem);

	//创建物品
	stItemInfo* CreateItem(U32 nItemId, U32 nQuantity = 1);
	//添加物品
	bool AddItem(U32 nItemId,  AccountRef pAccount, int SocketHandle, U32 nQuantity = 1);
	//为邮件发送一些特殊物品
	bool AddItemEx(stItemInfo* pItem, AccountRef pAccount);
	//使用物品
	bool UseItem(stItemInfo* stItem,AccountRef pAccount, int SocketHandle);
	//---------------------------仓库处理-----------------------------
	bool StoreItemToBank(UIDVEC& uidVec, AccountRef pAccount);
	bool GetItemFromBank(UIDVEC& uidVec, AccountRef pAccount);
	//---------------------------消息处理函数--------------------------
	void HandleClientMountEquipRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);//穿上装备
	void HandleClientUnmountEquipRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);//卸下装备
	void HandleClientEquipAddExpRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);//装备添加经验
	void HandleClientStrengthenEquipRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);//玩家强化装备
	void HandleClientComposeEquipRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet); //玩家合成装备
	void HandleClientUseItemRequset(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet); //使用物品
	void HandleClientAddItemRequset(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet); //添加物品
	void HandlePlayerBankStoreItemRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);//存仓库请求
	void HandlePlayerBankGetItemRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);//从仓库取物品请求
	void HandleClientDropItemRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet); //掉落物品
	void HandleClientPickItemRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet); //掉落物品
};
#endif