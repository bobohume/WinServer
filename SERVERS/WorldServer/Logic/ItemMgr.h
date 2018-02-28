#ifndef ITEM_MANAGER_H
#define ITEM_MANAGER_H

#include <hash_map>
#include <deque>
#include "WINTCP/IPacket.h"
#include "Common/PlayerStruct.h"
#include "BaseMgr.h"

struct CDropData;
typedef std::pair<S32, S32> DROPITEM_PAIR;
/************************************************************************/
/* 物品管理对象
/************************************************************************/
class CItemManager : public CBaseManagerEx<ItemInfoRef, std::string>
{
	enum Item_Event
	{
		ITEM_NO_SHOW = 0,
		ITEM_ADD	 = 1,
		ITEM_LOSE	 = 2,
	};
public:
	CItemManager();
	~CItemManager();

	static CItemManager* Instance();
	//创建物品
	ItemInfoRef CreateItem(U32 nItemId, U32 nPayerID, U32 nQuantity = 1);
	//添加物品
	bool AddItem(U32 nPlayerID, U32 nItemId, S32 Quantity = 1);
private:
	bool _AddItem(U32 nPlayerID, U32 nItemId, S32 Quantity = 1);
	bool _ReduceItem(U32 nPlayerID, U32 nItemId, S32 Quantity = 1);
public:
	bool CanReduceItem(U32 nPlayerID, U32 nItemId, S32 Quantity = 1);
	//使用物品
	bool UseItem(U32 nPlayerID, U32 nItemId, S32 Quantity = 1);
	bool UseItem(U32 nPlayerID, std::string UID, S32 Quantity = 1, std::string UID1 = "");
	bool ComposeItem(U32 nPlayerID, U32 nComposeID);
	CDropData*  DropItem(U32 nDropID);
	DROPITEM_PAIR DropItem(U32 nPlayerID, U32 nDropID);
	bool BatchDropItem(U32 nPlayerID, U32 nDropID, S32 Quantity = 1);
	bool AddGift(U32 nPlayerID, U32 nGiftID, S32 Quantity = 1);
	enOpError BuyItem(U32 nPlayerID, U32 nShopId, S32 Quantity = 1);
	//使用加速卡
	enOpError UseAccCard(U32 nPlayerID, std::string UID, S32 TimerId, S32 Quantity = 1);
	//使用比武令
	enOpError UseAddPVPTimesCard(U32 nPlayerID, std::string UID, S32 Quantity = 1);
	//强化
	enOpError EquipStrengthen(U32 nPlayerID, std::string EquipUID[101], std::string CardUID);
	//兑换商店
	enOpError BuyRandomShop(U32 nPlayerID, S32 nSlot);
	enOpError OpenRandomShop(U32 nPlayerID);
	enOpError ReflashRandomShop(U32 nPlayerID);
	//抽奖(nType:1连龙魂，2：10连龙魂， 3：1连征服， 4：5连征服)
	enOpError DrawItem(U32 nPlayerID, S8 nType, std::vector<DROPITEM_PAIR>& ItemVec);
	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);

	bool UpdateDB(ItemInfoRef pItem);
	bool InsertDB(U32 nPlayerID, ItemInfoRef pItem);
	bool DeleteDB(U32 nPlayerID, std::string UID);

	//----------通知客户端-----------//
	void UpdateToClient(U32 nPlayerID, ItemInfoRef pItem);
	void SendInitToClient(U32 nPlayerID);

	//----------角色登录通知---------//
	bool onPlayerEnter(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool onPlayerLeave(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	//----------消息处理----------//
	bool HandleClientUseItemRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientComposeItemRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientBuyItemRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientUseAccCardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientUseAddPVPTimesRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	bool HandleClientBuyRandomShopRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientOpenRandomShopRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientReflashRandomShopRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientDrawItemRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientEquipStrengthenRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	static void Export(struct lua_State* L);
};

#define ITEMMGR CItemManager::Instance()
#endif//ITEM_MANAGER_H
