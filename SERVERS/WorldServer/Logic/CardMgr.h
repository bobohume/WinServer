#ifndef CARD_MANAGER_H
#define CARD_MANAGER_H

#include <unordered_map>
#include "WINTCP/IPacket.h"
#include "CardBase.h"
#include "BaseMgr.h"
#include <vector>

struct Stats;
struct stBuff;
struct stFate 
{
	S32 PlayerID;
	S32 FateID;//卡牌缘分ID
	S32 Level;//卡牌缘分等级
	S32 Val;//卡牌附加属性
	std::string UID[10];

	stFate():PlayerID(0), FateID(0), Level(0), Val(0)
	{
	}
};

typedef std::tr1::shared_ptr<stFate> FateRef;
/************************************************************************/
/* 卡牌管理对象
/************************************************************************/
class CCardManager : public CBaseManagerEx<CardRef, std::string>
{
	static const int s_AddAbilityVal[MAX_ADD_SLOT];
	static const int s_CostGrowItemID[3];
	static const int s_AddAbilityConfig[12][3];
	static const int s_OfficalRankConfig[10][2];
	typedef		std::unordered_map<S32, FateRef>		FATEMAP;
	typedef		std::unordered_map<S32, FATEMAP>		FATEPLAYERMAP;
public:
	typedef		DATA_MAP	DATA_MAP;
	CCardManager();
	~CCardManager();

	static CCardManager* Instance();

	CardRef CreateCard(U32 nPlayerID, U32 nCardID);
	bool CanAddCard(U32 nPlayerID, U32 nCardID);
	bool AddCard(U32 nPlayerID, U32 nCardID, bool bNotify = true);
	bool RemoveCard(U32 nPlayerID, std::string UID);
	void RemovePlayer(U32 nPlayerID);
	enOpError DispatchCard(U32 nPlayerID, std::string UID, U32 nType);//派遣卡牌
	enOpError DispatchCardAward(U32 nPlayerID, S32 nTimeID, S32& nAddVal, bool& bDouble);
	enOpError AddCardLevel(U32 nPlayerID, std::string UID, S32 level);//升级等级
	enOpError AddCardLevelLimit(U32 nPlayerID, std::string UID);//等级突破
	enOpError AddCardOfficalRank(U32 nPlayerID, std::string UID);//升级官品
	enOpError AddCardSkill(U32 nPlayerID, std::string UID, S32 SkillID, bool& bScuess);//升级技能
	enOpError AddCardEquip(U32 nPlayerID, std::string UID, std::string ItemUID);//装备物品
	enOpError RemoveCardEquip(U32 nPlayerID, std::string UID, S32 nPos);//装备物品
	enOpError AddCardAbility(U32 nPlayerID, std::string UID, U32 nType, S32 nTimes);//提升资质
	enOpError AddCardGrow(U32 nPlayerID, std::string UID, U32 nType, S32& nAddVal);//提升成长
	enOpError HireCard(U32 nPlayerID, U32 nCardID);//召唤卡牌
	enOpError UpdateApply(U32 nPlayerID, S32 iCreateNum);//打开奏章
	enOpError ReadApply(U32 nPlayerID, S32 TimeID, bool bApply);//批阅奏章
	bool BanishCard(U32 nPlayerID, std::string UID);
	//------------缘分-----------//
	void RemoveFatePlayer(U32 nPlayerID);
	bool CaculteCardFate(U32 nPlayerID, bool bNotify = true);
	bool LoadCardFateDB(U32 nPlayerID);
	void CaculteFateState(U32 nPlayerID, std::string UID, S32 nBase_gPc[3]);
	//___________________________//
	enOpError _LearnSkill(U32 nPlayerID, std::string UID, S32 SkillID, bool& bScuessful);
	enOpError _LeaveUpSkill(U32 nPlayerID, std::string UID, S32 SkillID, bool& bScuessful);

	DATA_MAP GetDataMap(U32 nPlayerID);
	S32		 GetCardNum(U32 nPlayerID, S32 nLevel = 0);
	S32		 GetCardLevel(U32 nPlayerID, S32 CardID);
	bool CaculateCardStats(U32 nPlayerID, std::string UID, Stats& stats);//计算单个属性
	bool CaculateStats(U32 nPlayerID, std::string UID, bool bNotify = true);//计算属性
	bool CaculateStats(U32 nPlayerID);//计算所有大臣属性
	bool CaculateEquipStats(U32 nPlayerID, std::string UID, U32 nEquipAddPro[MAX_EQUIP_SLOT]);//计算装备属性
	bool AddCardAchievement(U32 nPlayerID, std::string UID, S32 val, bool bAddItem = false, bool bNotify = true);//增加功勋
	bool CanAddCardAchievement(U32 nPlayerID, std::string UID, S32 val);//增加功勋
	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);
	bool DeleteDB(U32 nPlayerID, std::string UID);
	bool InsertDB(U32 nPlayerID, CardRef pCard);
	bool UpdateDB(U32 nPlayerID, CardRef pCard);
	//----------同步获取-----------//
	DATA_MAP SyncLoadDB(U32 nPlayerID);

	//---------通知客户端----------//
	void UpdateToClient(U32 nPlayerID, CardRef pCard);
	void SendInitToClient(U32 nPlayerID);
	static void Export(struct lua_State* L);

	//----------角色登录通知------//
	bool onPlayerEnter(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool onPlayerLeave(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);

	//----------消息处理----------//
	bool HandleAddCardLevelRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleAddCardLevelLimitRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleAddCardSkillRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleAddCardEquipRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleRemoveCardEquipRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleAddCardAbilityRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleAddCardGrowRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleHireCardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleDispatchCardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleReadApplyRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleAddCardOfficalRankRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleDispatchCardAwardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

private:
	//大臣技能
	S32 GetSkillFailedNumBySkillId(U64 SkillsFailedNum, S32 SkillId);
	void AddSkillFailedNumBySkillId(U64& SkillsFailedNum, S32 SkillId);
	void ClearSkillFailedNumBySkillId(U64& SkillsFailedNum, S32 SkillId);
	//大臣资质
	S32 GetGrowFailedNumByType(U32 GrowFialedNum, S32 Type);
	void AddGrowFailedNumByType(U32& GrowFialedNum, S32 Type);
	void ClearGrowFailedNumByType(U32& GrowFialedNum, S32 Type);

private:
	FATEPLAYERMAP m_FatePlayerMap;
};

#define CARDMGR CCardManager::Instance()
#endif//CARD_MANAGER_H
