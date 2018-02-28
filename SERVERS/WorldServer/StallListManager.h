#ifndef __STALLLISTMANAGER_H__
#define __STALLLISTMANAGER_H__
/*
#include <deque>
#include "LockCommon.h"
#include "ManagerBase.h"
#include "Common/StallListSMData.h"

class CStall : public StallSMData
{
public:
	CStall(void);
	CStall(U16 id);
	~CStall(void);

	bool InitSM(U32 stallId);
	bool InitSM(SM_STALL_DATA* pSMData);
    
    //这个暂时不支持多个需要posMask的标识同时使用
	void UpdateSMData(int mask,U32 posMask = -1);

	void SetInfo(stFixedStall* pInfo);
	stFixedStall* GetInfo(void);
	void InitInfo();

	void SetOneSellItem(stFixedStallItem* pItem);
	void SetOneBuyItem(stFixedStallItem* pItem);
	void SetOneSellPet(stFixedStallPet* pPet);
	stFixedStallItem* GetOneSellItem(U32 pos);
	stFixedStallItem* GetOneBuyItem(U32 pos);
	stFixedStallPet* GetOneSellPet(U32 pos);

	U32 GetInfoVer() { return m_Info.DataVerion;}
	void SetInfoVer() { m_Info.DataVerion++;}
	U32 GetSellItemVer(U32 pos);
	void SetSellItemVer(U32 pos);
	U32 GetBuyItemVer(U32 pos);
	void SetBuyItemVer(U32 pos);
	U32 GetSellPetVer(U32 pos);
	void SetSellPetVer(U32 pos);

	U16 GetStallID()        { return m_Info.StallID;}
    void SetStallId(U32 id) {m_Info.StallID = id;}

	void SetStallName(const char* name) { dStrcpy(m_Info.StallName, 32, name);}
	const char* GetStallName() { return m_Info.StallName;}

	void SetStallerID(U32 PlayerID) { m_Info.StallerID = PlayerID;}
	U32 GetStallerID() { return m_Info.StallerID;}

	void SetStallerName(const char* name) { dStrcpy(m_Info.StallerName, 32, name);}
	const char* GetStallerName() { return m_Info.StallerName;}

	void SetPreStallerID(U32 PlayerID) { m_Info.PreStallerID = PlayerID;}

	void SetStatus(U8 state) { m_Info.StallStatus = state;}
	U8 GetStatus() { return m_Info.StallStatus;}

	void SetStallType(U8 type) { m_Info.StallType = type;}
	U8 GetStallType() { return m_Info.StallType;}

	void SetTradeTax(S32 tax) { m_Info.TradeTax = tax;}
	S32 GetTradeTax() { return m_Info.TradeTax;}
	void SetStallTax(S32 tax) { m_Info.StallTax = tax;}
	S32 GetStallTax() { return m_Info.StallTax;}

	void SetPayTaxTime(U32 t) { m_Info.PayTaxTime = t;}
	U32 GetPayTaxTime() { return m_Info.PayTaxTime;}

	void SetIsAttorn(bool t) { m_Info.IsAttorn = t;}
	bool IsAttorn() { return m_Info.IsAttorn;}
	void SetAttornPrice(S32 price) { m_Info.AttornPrice = price;}
	S32 GetAttornPrice() { return m_Info.AttornPrice;}
	void SetAttornStartTime(U32 t) { m_Info.AttornStartTime = t;}
	U32 GetAttornStartTime() { return m_Info.AttornStartTime;}

	void SetFixedMoney(S32 money) { m_Info.FixedMoney = money;}
	S32 GetFixedMoney() { return m_Info.FixedMoney;}

	void SetFlowMoney(S32 money) { m_Info.FlowMoney = money;}
	S32 GetFlowMoney() { return m_Info.FlowMoney;}

	void SetNotice(const char* word) { dStrcpy(m_Info.Notice, 128, word);}
	const char* GetNotice() { return m_Info.Notice;}

	void SetIsShoutAll(bool t) { m_Info.IsShoutAll = t;}
	void SetShoutItemNum(U32 itemnum) { m_Info.ShoutItemNum = itemnum;}
	void SetShoutLastTime(U32 t) { m_Info.ShoutLastTime = t;}
	void SetShoutContent(const char* ad) {dStrcpy(m_Info.ShoutContent, 128, ad);}
	void SetShoutInterval(U16 t) { m_Info.ShoutInterval = t;}
	void SetShoutItemId(U32 t) { m_Info.ShoutItemID = t;}
	const char* GetShoutContent() { return m_Info.ShoutContent;}
	bool GetIsShoutAll() { return m_Info.IsShoutAll;}
	U32 GetShoutItemId() { return m_Info.ShoutItemID;}
	U16 GetShoutItemNum() { return m_Info.ShoutItemNum;}
	U32 GetShoutLastTime() { return m_Info.ShoutLastTime;}
	U32 GetShoutInterval() { return m_Info.ShoutInterval;}

	void SetFlourish(U32 t) { m_Info.Flourish = t;}
	U32 GetFlourish() { return m_Info.Flourish;}

	void SetBuySellItems(U16 t) {m_Info.BuySellItems = t;}
	U16 GetBuySellItems() { return m_Info.BuySellItems;}
	void SetBuySellPets(U8 t) { m_Info.BuySellPets = t;}
	U16 GetBuySellPets() { return m_Info.BuySellPets;}
	void SetBuyTime(U32 t) { m_Info.BuyTime = t;}

	//========帮助函数=========================================================
	bool FindSameItemInBuyItem(U32 itemId);
	S16 FindEmptyPosFromSellItem();
	S16 FindEmptyPosFromSellPet();
	S16 FindEmptyPosFromBuyItem();

	SM_STALL_DATA*				m_pSMData;
	std::deque<stStallChat*>	ChatList;		//商铺玩家留言记录（限定保存最近50条）
	std::deque<stStallRecord*>	RecordList;		//商铺铺主交易记录（限定保存最近50条）

	void AddChatToList(const char* sender, const char* content);
	void ClearChat();
	void AddRecordToList(U32 type, S32 money, S32 tax, const char* target,
		stItemInfo* item, S32 num, stPetInfo* pet);
	void ClearRecord();
};

typedef CStall* StallHandle;

class CStallManager : public CManagerBase< CStallManager, MSG_STALL_BEGIN, MSG_STALL_END >
{
public:
	typedef stdext::hash_map<U32, StallHandle> STALLMAP;
	CStallManager(void);
	~CStallManager();

    bool LoadAllStalls(void);
	bool InitSMData(void);
	SM_STALL_POOL* GetStallSMPool(void) {return &m_stallSMPool;}

	//购买系统商铺价格[注：公式＝300000 * (1 + m_TotalFlourish / 25000) * 2]
	S32 GetBuySystemStall() { return BUYSYSSTALLPRISE(m_TotalFlourish); }
	//计算每日税.(注:公式=5000 * pStall->GetFlourish() / 5 * (1 + m_TotalFlourish/25000)
	S32 GetDayTax(S32 flourish) { return (flourish * (1000 + m_TotalFlourish / 25));}

	size_t GetStallSize() { return m_stallMap.size(); }
	StallHandle GetStall(U32 stallId);
	void SetTotalFlourish(U32 t);
	U32 GetTotalFlourish() { return m_TotalFlourish;}
	void SetMaxStalls(U32 t) { if(t > m_MaxStalls) m_MaxStalls = t;}
	U32 GetMaxStalls() { return m_MaxStalls;}
    
	void UpdateStall();
	void UpdateAllStallTax();

	void HandleQueryStallListInfoRequest(S32, stPacketHead*, Base::BitStream*);
	void HandleQueryStallBaseInfoRequest(S32, stPacketHead*, Base::BitStream*);
	void HandleQueryStallDataRequest(S32, stPacketHead*, Base::BitStream*);
	void SendStallInfoData(S32 SocketHandle, U32 srcZoneId, std::vector<StallHandle>& vecInfo);
	void SendSellItemData(S32 SocketHandle, U32 srcZoneId, std::vector<stFixedStallItem*>& vecItem);
	void SendBuyItemData(S32 SocketHandle, U32 srcZoneId, std::vector<stFixedStallItem*>& vecItem);
	void SendSellPetData(S32 SocketHandle, U32 srcZoneId, std::vector<stFixedStallPet*>& vecPet);

	void HandleStallBuyRequest(S32, stPacketHead*, Base::BitStream*);
	void SendStallBuyRespond(U32, U16, U32, stFixedStall*, S32);

	void HandleSetStallStatusRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSetStallStatusRespond(U32, U16, U8, S32);

	void HandleSetStallNameRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSetStallNameRespond(U32, U16, const char*, S32);

	void HandleSetAttornPriceRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSetAttornPriceRespond(U32, U16, S32, S32);

	void HandleNoticeRequest(S32, stPacketHead*, Base::BitStream*);
	void SendNoticeRespond(U32, U16, const char*, S32);

	void HandleSetFlowMoneyRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSetFlowMoneyRespond(U32, U16, S32, bool, S32);

	void HandleSetFixedMoneyRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSetFixedMoneyRespond(U32, U16, S32, bool, S32);

	void HandleShoutContentRequest(S32, stPacketHead*, Base::BitStream*);
	void SendShoutContentRespond(U32, U16, const char*, S32);

	void HandleStallTypeRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSetStallTypeRespond(U32, U16, U8, S32);

	void HandleIsAttornRequest(S32, stPacketHead*, Base::BitStream*);
	void SendIsAttornRespond(U32, U16, bool, U32, S32);

	void HandleIsShoutAllRequest(S32, stPacketHead*, Base::BitStream*);
	void SendIsShoutAllRespond(U32, U16, bool, S32);

	void HandleShoutIntervalRequest(S32, stPacketHead*, Base::BitStream*);
	void SendShoutIntervalRespond(U32, U16, U16, S32);

	void HandleBuyStallSlotsRequest(S32, stPacketHead*, Base::BitStream*);
	void SendBuyStallSlotsRespond(U32, U16, U8, U8, U8, S32, S32, S32);

	void HandleSellItemRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSellItemRespond(U32, const char*, U16, bool,stItemInfo&, S32, S32, S32, S32);

	void HandleSellPetRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSellPetRespond(U32, U16, bool,stPetInfo&, S32, S32, S32, S32);

	void HandleBuyItemRequest(S32, stPacketHead*, Base::BitStream*);
	void SendBuyItemRespond(U32, const char*, U16, bool,stFixedStallItem&, S32);

	void HandleSetShoutItemRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSetShoutItemRespond(U32, const char*, U16, bool, U32, S32, S32, S32);

	void HandlePlayerItemRequest(S32, stPacketHead*, Base::BitStream*);
	void SendPlayerItemRespond(U32, const char*, U16, U8, S32, S32, S16, S16,
		stItemInfo*, stPetInfo*, S32, S32, S32);

	void HandleSetStallMoneyRequest(S32, stPacketHead*, Base::BitStream*);
	void SendSetStallMoneyRespond(U32, U16, U8, S32, S32, S32);

	void HandleSendChatRequest(S32 SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	void HandleSendClearChatRequest(S32 SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	void HandleQueryChatRequest(S32 SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	void SendChatRespond();

private:
	STALLMAP		m_stallMap;			//商铺数据列表
	SM_STALL_POOL	m_stallSMPool;		//商铺共享数据池
	U32				m_TotalFlourish;	//总繁荣度(商业指数)
	U32				m_MaxStalls;		//当前最大商铺数
};
*/
#endif//__STALLLISTMANAGER_H__
