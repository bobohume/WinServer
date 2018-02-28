#ifndef __ORG_H
#define __ORG_H
#include "OrgBase.h"
#include "BaseMgr.h"
#include "Base/tVector.h"
#include "CommLib/SimpleTimer.h"
#include <unordered_map>
#include <vector>
#include <deque>

class Org
{
public:
	typedef std::unordered_map<S32, OrgMemberRef> ORG_MEMBER_MAP;
	typedef std::unordered_map<S32, OrgSkillRef>  ORG_SKILL_MAP;
	typedef std::unordered_map<S32, OrgApplyRef>  ORG_APPLY_MAP;
	typedef std::unordered_multimap<S32, OrgGiftRef> ORG_GIFT_MAP;
	typedef std::deque<OrgLogRef>				  ORG_LOG_MAP;
	enum enMemLevel
	{
		NORMAL,
		ELITE,
		SUBMASTER,
		MAX_LEVEL,
	};

	enum
	{
		MAX_ORG_LEVEL = 60,
		MAX_SKILL_LEVEL = 20,
		MAX_ORG_LOG_NUM = 500,
	};

	Org();
	~Org();

	static void RegisterOrgChannelCallback(int type, int channelId, void* param);

	inline S32	GetOrgId() { return m_Org.Id; }
	inline std::string	GetOrgName() { return m_Org.Name; }
	inline stOrg* GetOrg() { return &m_Org; }
	inline S32	GetMaxSubMasterNum() { return m_Org.Level / 20 + 1; }
	inline S32	GetMaxEliteNum() { return m_Org.MaxEliteNum;  }
	inline S32  GetChannel() { return m_channel; }
	inline void SetChannel(S32 nId) { m_channel = nId; }
	inline ORG_SKILL_MAP& GetOrgSkillMap() { return m_OrgSkillMap; }
	S32	GetPlayerNum(enMemLevel level);

	bool IsMasterId(S32 nPlayerID);
	bool IsSubMasterId(S32 nPlayerID);

	void AddMember(OrgMemberRef pMember);
	void RemoveMember(S32 nPlayerId, bool bActive = false);
	OrgMemberRef GetMember(S32 nPlayerId);
	void AddGiftExp(S32 nVal);

	void Disband();

	void AddApply(OrgApplyRef pApply);
	void RemoveApply(S32 nPlayerId);
	OrgApplyRef GetApply(S32 nPlayerId);

	void SetPlayerMaxNum(S32 nVal);
	void SetElitePlayerMaxNum(S32 nVal);
	void AddSkillEffect(S32 nPreTechId, S32 nAftTechId);
	void AddPlayerSkillEffect(S32 nPlayerId);
	void RemovePlayerSkillEffect(S32 nPlayerId);

	void AddSkill(OrgSkillRef pSkill);
	OrgSkillRef GetSkill(S32 nSkillId);
	void SetQq(std::string Name);
	void SetMsg(std::string Name);
	void SetMemo(std::string Name);
	void AddExp(S32 nExp, bool bSaveDB = true);
	void AddSkillPoint(S32 nVal, bool bSaveDB = true);
	void AddContribute(S32 nPlayerId, S32 nVal, bool bSaveDB = true);
	void LeaveUpSkill(S32 nSkillId);
	void SetMemberLevel(S32 nPlayerId, enMemLevel level);
	void CaculteBattle();

	void AddGift(OrgGiftRef pGift);
	void AddGift(S32 GiftId, std::string Name);
	void RemoveGift(S32 nPlayerId, std::string UID);
	OrgGiftRef GetGift(S32 nPlayerId, std::string UID);

	bool CanOpenBoss();
	void SetBossTimes();

	void WriteOrgLog(S32 OrgId, S32 SourceID, S32 TargetID, S32 OptType, S32 Val0 = 0, S32 Val1= 0);
	void AddLog(OrgLogRef pLog);
	void SendInitToClient(U32 nPlayerID);
	void SendUpdateOrgToClient();
	void SendUpdateMemberToClient(OrgMemberRef pData);
	void SendUpdateSkilloClient(OrgSkillRef pData);
	void SendUpdateApplyToClient(OrgApplyRef pData);
	void SendUpdateLogToClient(OrgLogRef pData);
	void SendDelApplyToClient(S32 nID);
	void SendDelMemberToClient(S32 nID);

private:
	stOrg m_Org;
	ORG_MEMBER_MAP							m_OrgMemberMap;
	ORG_SKILL_MAP							m_OrgSkillMap;
	ORG_APPLY_MAP							m_OrgApplyMap;
	ORG_LOG_MAP								m_OrgLogMap;
	ORG_GIFT_MAP							m_OrgGiftMap;
	int m_channel;
};

typedef std::tr1::shared_ptr<Org> OrgRef;
typedef std::tr1::shared_ptr<stSimpleOrg> SimpleOrgRef;

class COrgMgr
{
	typedef std::unordered_map<S32, OrgRef> ORG_MAP;
	typedef std::unordered_map<std::string, S32> ORG_NAME_MAP;
	typedef std::vector<SimpleOrgRef>	ORG_VEC;
public:
	enum enDonateType
	{
		PRIMARY_GRADE,
		MEDIUM_GRADE,
		HIGH_GRADE,
		ITEM,
		HIGH_ITEM,
		DONATE_MAX,
	};

	COrgMgr();
	virtual ~COrgMgr();

	static COrgMgr* Instance();

	enOpError CreateOrg(S32 nPlayerID, std::string OrgName, std::string QQ, std::string Memo, bool AutoAcceptJoin);
	enOpError CreateOrgHandle(S32 nPlayerID, std::string OrgName, S32 OrgId, std::string QQ, std::string Memo, bool AutoAcceptJoin);
	enOpError ApplyOrg(S32 nPlayerID, S32 OrgId);//申请加入联盟
	enOpError ChangeOrg(S32 nPlayerID, S32 OrgId, std::string QQ, std::string Memo, std::string Msg, bool AutoAcceptJoin);//改变联盟状态
	enOpError JoinOrg(S32 nPlayerID, S32 OrgId, S32 nApplyPlayerID, bool bAgree = false);
	enOpError LeaveOrg(S32 nPlayerID, S32 OrgId);//自动离开联盟
	enOpError FireMmeber(S32 nPlayerID, S32 OrgId, S32 nFirePlayerID);//开除成员
	enOpError DonateOrg(S32 nPlayerID, S32 OrgId, enDonateType nType);//捐献
	enOpError SetOrgMasterLevel(S32 nPlayerID, S32 OrgId, S32 nTargetPlayerID, Org::enMemLevel enLevel);//设置副盟主
	enOpError TransferMaster(S32 nPlayerID, S32 OrgId, S32 nTransferPlayerID);//转移帮主
	enOpError LeaveUpSkill(S32 nPlayerID, S32 OrgId, S32 nTechSeries);//升级科技
	enOpError DisbandOrg(S32 nPlayerID, S32 OrgId);//删除联盟
	enOpError SearchTopOrg(S32 nPlayerID);//查询联盟排行榜
	enOpError SearchOrg(S32 nPlayerID);//查询联盟
	enOpError BuyOrgItem(S32 nPlayerID, S32 OrgId, S32 nShopId);//购买物品
	enOpError BuyOrgGift(S32 nPlayerID, S32 OrgId);//兑换宝箱
	enOpError AddOrgGift(S32 OrgId, S32 GiftId, std::string Name);//添加联盟礼包
	enOpError GetOrgGift(S32 nPlayerID, S32 OrgId, std::string UID, S32& GiftId);//领取礼包

	OrgRef GetOrg(S32 OrgId);
	void AddOrg(OrgRef pData);
	void DeleteOrg(S32 OrgId);
	void RegisterChannel();
	void AddOrgSkill(S32 nPlayerID, S32 OrgId);
	void AddGiftExp(S32 OrgId, S32 Gold);
	void GetOrgContribute(S32 nPlayerID, bool& bFind, S32& Contribute);
	//---------Notify Operator---------//
	void UpdateToClient(U32 nPlayerID, OrgRef pData);
	void SendInitToClient(U32 nPlayerID);
	//---------DB Operator---------//
	void LoadDB();
	//------------消息---------------//
	bool HandleCreateOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleRCreateOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleChangeOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleApplyOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleJoinOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleLeaveOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleFireMemberOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleDonateOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleSetOrgMasterLevelResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleTransferOrgMasterResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleLeaveUpOrgSkillResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleDisbandOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleSearchTopOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleSearchOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleSearchOrgSimpleResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleBuyOrgItemResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleBuyOrgGiftResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleGetOrgGiftResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool onNotify(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	static void Export(struct lua_State* L);

	inline ORG_VEC& GetTopOrgVec()	{ return m_TopOrgVec; }
	inline ORG_VEC& GetOrgVec()		{ return m_OrgVec; }
	inline ORG_MAP& GetOrgMap()		{ return m_OrgMap; }
private:
	ORG_MAP m_OrgMap;
	ORG_NAME_MAP m_OrgNameMap;
	ORG_VEC m_TopOrgVec;
	ORG_VEC m_OrgVec;
	SimpleTimer<120> m_SearchTimer;
	SimpleTimer<10>	m_TopSearchTimer;
};

#define ORGMGR COrgMgr::Instance()
#endif//__ORG_H