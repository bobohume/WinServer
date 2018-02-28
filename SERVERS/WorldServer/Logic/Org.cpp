#include "Org.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "Common/MemGuard.h"
#include "../WorldServer.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "../PlayerMgr.h"
#include "Common/mRandom.h"
#include "PlayerInfoMgr.h"
#include "platform.h"
#include "WINTCP/dtServerSocket.h"
#include "WINTCP/AsyncSocket.h"
#include "BASE/tVector.h"
#include "BASE/mMathFn.h"
#include "DBLayer/Common/DBUtility.h"
#include "CommLib/ToSql.h"
#include "Common/FilterOpt.h"
#include "CommLib/format.h"
#include "TimeSetMgr.h"
#include "ItemMgr.h"
#include "OrgData.h"
#include "BuffMgr.h"
#include "SkillData.h"
#include "ItemData.h"
#include "CardMgr.h"
#include "../MailManager.h"
#include "../ChatHelper.h"

//注册sql
DECLARE_SQL_UNIT(stOrg);
DECLARE_SQL_UNIT(stOrgMember);
DECLARE_SQL_UNIT(stOrgSkill);
DECLARE_SQL_UNIT(stOrgApply);
DECLARE_SQL_UNIT(stOrgLog);
DECLARE_SQL_UNIT(stOrgGift);
#define ORGDB			GET_SQL_UNIT(stOrg)
#define ORGMEMBERDB		GET_SQL_UNIT(stOrgMember)
#define ORGSKILLDB		GET_SQL_UNIT(stOrgSkill)
#define ORGAPPLYDB		GET_SQL_UNIT(stOrgApply)
#define ORGLOGDB		GET_SQL_UNIT(stOrgLog)
#define ORGGIFTDB		GET_SQL_UNIT(stOrgGift)
#define ORGLOGTABLE		"Tbl_OrgLog"
COrgMgr::COrgMgr()
{
	//注册org表
	GET_SQL_UNIT(stOrg).SetName("Tbl_Org");
	REGISTER_SQL_UNITKEY(stOrg, Id);
	REGISTER_SQL_UNIT(stOrg, Level);
	REGISTER_SQL_UNIT(stOrg, Exp);
	REGISTER_SQL_UNIT(stOrg, SkillPoint);
	REGISTER_SQL_UNIT(stOrg, Name);
	REGISTER_SQL_UNIT(stOrg, Qq);
	REGISTER_SQL_UNIT(stOrg, Msg);
	REGISTER_SQL_UNIT(stOrg, Memo);
	REGISTER_SQL_UNIT(stOrg, MasterId);
	REGISTER_SQL_DATETIME(stOrg, TransferTime);
	REGISTER_SQL_UNIT(stOrg, BattleVal);
	REGISTER_SQL_UNIT(stOrg, PlayerNum);
	REGISTER_SQL_UNIT(stOrg, MaxPlayerNum);
	REGISTER_SQL_UNIT(stOrg, MaxEliteNum);
	REGISTER_SQL_UNIT(stOrg, AutoAcceptJoin);
	REGISTER_SQL_UNIT(stOrg, OrgBossTimes);
	REGISTER_SQL_DATETIME(stOrg, OrgBossTime);

	//注册orgmember表
	GET_SQL_UNIT(stOrgMember).SetName("Tbl_OrgMember");
	REGISTER_SQL_UNITKEY(stOrgMember, PlayerId);
	REGISTER_SQL_UNIT(stOrgMember, OrgId);
	REGISTER_SQL_UNIT(stOrgMember, PlayerName);
	REGISTER_SQL_UNIT(stOrgMember, Contribute);
	REGISTER_SQL_UNIT(stOrgMember, GiftExp);
	REGISTER_SQL_UNIT(stOrgMember, MemberLevel);
	REGISTER_SQL_UNIT(stOrgMember, MemberStatus);
	REGISTER_SQL_UNIT(stOrgMember, BattleVal);
	REGISTER_SQL_UNIT(stOrgMember, Vip);
	REGISTER_SQL_UNIT(stOrgMember, PlayerFace);
	REGISTER_SQL_DATETIME(stOrgMember, LeaveTime);

	//注册orgskill表
	GET_SQL_UNIT(stOrgSkill).SetName("Tbl_OrgSkill");
	REGISTER_SQL_UNITKEY(stOrgSkill, OrgId);
	REGISTER_SQL_UNITKEY(stOrgSkill, SkillId);
	REGISTER_SQL_UNIT(stOrgSkill, Level);

	//注册orgapply表
	GET_SQL_UNIT(stOrgApply).SetName("Tbl_OrgApply");
	REGISTER_SQL_UNITKEY(stOrgApply, PlayerId);
	REGISTER_SQL_UNITKEY(stOrgApply, OrgId);
	REGISTER_SQL_UNIT(stOrgApply, PlayerName);
	REGISTER_SQL_UNIT(stOrgApply, BattleVal);
	REGISTER_SQL_DATETIME(stOrgApply, ApplyTime);
	REGISTER_SQL_UNIT(stOrgApply, Vip);
	REGISTER_SQL_UNIT(stOrgApply, PlayerFace);
	REGISTER_SQL_UNIT(stOrgApply, OnlineFlag);

	//注册orglog表
	GET_SQL_UNIT(stOrgLog).SetName("Tbl_OrgLog");
	REGISTER_SQL_UNITKEY(stOrgLog, OrgId);
	REGISTER_SQL_UNITKEY(stOrgLog, CreateTime);
	REGISTER_SQL_UNITKEY(stOrgLog, SourceName);
	REGISTER_SQL_UNIT(stOrgLog, TargetName);
	REGISTER_SQL_UNIT(stOrgLog, OptType);
	REGISTER_SQL_UNIT(stOrgLog, Val0);
	REGISTER_SQL_UNIT(stOrgLog, Val1);

	//注册orggift
	GET_SQL_UNIT(stOrgGift).SetName("Tbl_OrgGift");
	REGISTER_SQL_UNITKEY(stOrgGift, OrgId);
	REGISTER_SQL_UNITKEY(stOrgGift, PlayerId);
	REGISTER_SQL_UNITKEY(stOrgGift, UID);
	REGISTER_SQL_UNIT(stOrgGift, GiftId);
	REGISTER_SQL_UNIT(stOrgGift, PlayerName);

	REGISTER_EVENT_METHOD("RW_CreateOrg",					this, &COrgMgr::HandleRCreateOrgResponse);
	REGISTER_EVENT_METHOD("CW_CreateOrg_Request",			this, &COrgMgr::HandleCreateOrgResponse);
	REGISTER_EVENT_METHOD("CW_ChangeOrg_Request",			this, &COrgMgr::HandleChangeOrgResponse);
	REGISTER_EVENT_METHOD("CW_ApplyOrg_Request",			this, &COrgMgr::HandleApplyOrgResponse);
	REGISTER_EVENT_METHOD("CW_JoinOrg_Request",				this, &COrgMgr::HandleJoinOrgResponse);
	REGISTER_EVENT_METHOD("CW_LeaveOrg_Request",			this, &COrgMgr::HandleLeaveOrgResponse);
	REGISTER_EVENT_METHOD("CW_FireMemberOrg_Request",		this, &COrgMgr::HandleFireMemberOrgResponse);
	REGISTER_EVENT_METHOD("CW_DonateOrg_Request",			this, &COrgMgr::HandleDonateOrgResponse);
	REGISTER_EVENT_METHOD("CW_SetOrgMasterLevel_Request",	this, &COrgMgr::HandleSetOrgMasterLevelResponse);
	REGISTER_EVENT_METHOD("CW_TransferOrgMaster_Request",	this, &COrgMgr::HandleTransferOrgMasterResponse);
	REGISTER_EVENT_METHOD("CW_LeaveUpSkill_Request",		this, &COrgMgr::HandleLeaveUpOrgSkillResponse);
	REGISTER_EVENT_METHOD("CW_DisbandOrgl_Request",			this, &COrgMgr::HandleDisbandOrgResponse);
	REGISTER_EVENT_METHOD("CW_SearchTopOrg_Request",		this, &COrgMgr::HandleSearchTopOrgResponse);
	REGISTER_EVENT_METHOD("CW_SearchOrg_Request",			this, &COrgMgr::HandleSearchOrgResponse);
	REGISTER_EVENT_METHOD("CW_BuyOrgItem_Request",			this, &COrgMgr::HandleBuyOrgItemResponse);
	REGISTER_EVENT_METHOD("CW_BuyOrgGift_Request",			this, &COrgMgr::HandleBuyOrgGiftResponse);
	REGISTER_EVENT_METHOD("CW_GetOrgGift_Request",			this, &COrgMgr::HandleGetOrgGiftResponse);
	REGISTER_EVENT_METHOD("CW_SearchOrgSimple_Request",		this, &COrgMgr::HandleSearchOrgSimpleResponse);
	
	REGISTER_EVENT_METHOD("WW_ORGGIFT_Notify",				this, &COrgMgr::onNotify);
	
	g_OrgExpDataMgr->read();
	m_TopSearchTimer.Start();
	m_SearchTimer.Start();
}

COrgMgr::~COrgMgr()
{
	g_OrgExpDataMgr->clear();
}

static auto SendOrgResponse = [](const char* msg, S32 nError, AccountRef pAccount) {
	CMemGuard Buffer(128 MEM_GUARD_PARAM);
	Base::BitStream sendPacket(Buffer.get(), 128);
	stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, msg, pAccount->GetAccountId(), SERVICE_CLIENT);
	sendPacket.writeInt(nError, Base::Bit16);
	pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
};

const static S32 s_DonateType[COrgMgr::DONATE_MAX][6] = {
	//TYPE			//NUM		//EXP		//POINT		//CONT		//GIFT
	{ 0,			10,			10,			10,			100,			2 },
	{ 0,			50,			50,			50,			500,			10 },
	{ 0,			200,		200,		200,		2000,		40 },
	{ 1,			600112,		400,		400,		4000,		80 },
	{ 1,			600113,		1000,		1000,		10000,		200 },
};

auto OrgGetPlayerName = [](S32 nPlayerID)
{
	PlayerSimpleDataRef pPlayer = SERVER->GetPlayerManager()->GetPlayerData(nPlayerID);
	if (pPlayer)
	{
		return (const char*)pPlayer->PlayerName;
	}

	return "";
};

auto SaveOrgDB = [](std::string sql)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(sql);
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
};

stSimpleOrg::stSimpleOrg(stOrg* other) : Id(other->Id), Level(other->Level), Name(other->Name),
MasterId(other->MasterId), BattleVal(other->BattleVal), PlayerNum(other->PlayerNum),
MaxPlayerNum(other->MaxPlayerNum), AutoAcceptJoin(other->AutoAcceptJoin), Memo(other->Memo)
{
	MasterName = OrgGetPlayerName(MasterId);
}

Org::Org() : m_channel(-1) {
}

Org::~Org() {
	if (m_channel != -1)
		CHAT_HELPER->unregisterChannel(m_channel);

	m_OrgMemberMap.clear();
	m_OrgSkillMap.clear();
	m_OrgApplyMap.clear();
	m_OrgLogMap.clear();
	m_OrgGiftMap.clear();
}

void Org::RegisterOrgChannelCallback(int type, int channelId, void* param)
{
	int orgId = (int)param;
	auto org = ORGMGR->GetOrg(orgId);
	if (org)
	{
		org->SetChannel(channelId);
		for (auto iter = org->m_OrgMemberMap.begin(); iter != org->m_OrgMemberMap.end(); ++iter)
			CHAT_HELPER->addPlayer(channelId, iter->first);
	}
}

S32	Org::GetPlayerNum(enMemLevel level) {
	auto nCurLen = 0;
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr) {
		if (itr->second && itr->second->MemberLevel == level) {
			nCurLen++;
		}
	}
	return nCurLen;
}

bool Org::IsMasterId(S32 nPlayerID) {
	return m_Org.MasterId == nPlayerID;
}

bool Org::IsSubMasterId(S32 nPlayerID) {
	auto pMember = GetMember(nPlayerID);
	if (pMember && pMember->MemberLevel == SUBMASTER) {
		return true;
	}
	return false;
}

void Org::AddMember(OrgMemberRef pMember) {
	m_OrgMemberMap.insert(std::make_pair(pMember->PlayerId, pMember));
}

void Org::RemoveMember(S32 nPlayerId, bool bActive/*=false*/)
{
	auto pMember = GetMember(nPlayerId);
	if (pMember) {
		pMember->OrgId = 0;
		if (bActive) {//主动离开
			pMember->Contribute = pMember->Contribute / 2;
			pMember->LeaveTime = time(NULL);
		}
		SaveOrgDB(ORGMEMBERDB.UpdateSqlEx(pMember, "OrgId", "Contribute", "LeaveTime"));
		CHAT_HELPER->removePlayer(m_channel, pMember->PlayerId);
		m_OrgMemberMap.erase(nPlayerId);

		m_Org.PlayerNum--;
		SaveOrgDB(ORGDB.UpdateSqlEx(&m_Org, "PlayerNum"));

		PLAYERINFOMGR->SetPlayerOrg(nPlayerId, 0);
		PLAYERINFOMGR->UpdateToClient(nPlayerId);
		RemovePlayerSkillEffect(nPlayerId);
	}
}

OrgMemberRef Org::GetMember(S32 nPlayerId) {
	auto itr = m_OrgMemberMap.find(nPlayerId);
	if (itr != m_OrgMemberMap.end())
	{
		return itr->second;
	}
	return OrgMemberRef();
}

void Org::AddGiftExp(S32 nVal)
{
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr) {
		if (itr->second) {
			itr->second->GiftExp = mClamp(itr->second->GiftExp + nVal, 0, S32_MAX);
			SaveOrgDB(ORGMEMBERDB.UpdateSqlEx(itr->second, "GiftExp"));
		}
	}
}

void Org::Disband() {
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr) {
		if (itr->second) {
			itr->second->OrgId = 0;
			SaveOrgDB(ORGMEMBERDB.UpdateSqlEx(itr->second, "OrgId"));
			CHAT_HELPER->removePlayer(m_channel, itr->second->PlayerId);
			RemovePlayerSkillEffect(itr->second->PlayerId);
			MAILMGR->sendMail(0, itr->second->PlayerId, "联盟解散", "联盟已经解散！！！", true);
		}
	}

	SaveOrgDB(fmt::sprintf("Delete from Tbl_OrgSkill WHERE OrgId = %d", m_Org.Id));
	SaveOrgDB(fmt::sprintf("Delete from Tbl_OrgApply WHERE OrgId = %d", m_Org.Id));
}

void Org::AddApply(OrgApplyRef pApply) {
	m_OrgApplyMap.insert(std::make_pair(pApply->PlayerId, pApply));
}

void Org::RemoveApply(S32 nPlayerId) {
	auto pApply = GetApply(nPlayerId);
	if (pApply) {
		SaveOrgDB(ORGAPPLYDB.DelSql(pApply));
		m_OrgApplyMap.erase(nPlayerId);
	}
}

void Org::AddGift(OrgGiftRef pGift) {
	m_OrgGiftMap.insert(std::make_pair(pGift->PlayerId, pGift));
}


void Org::AddGift(S32 GiftId, std::string Name) {
	std::string UID = uuid();
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr) {
		if (itr->second) {
			OrgGiftRef pGift = OrgGiftRef(new stOrgGift);
			pGift->OrgId = GetOrgId();
			pGift->UID = UID;
			pGift->PlayerId = itr->second->PlayerId;
			pGift->GiftId = GiftId;
			pGift->PlayerName = Name;
			AddGift(pGift);
			ANSYDB(0, ORGGIFTDB.InsertSql(pGift));
		}
	}
}

void Org::RemoveGift(S32 nPlayerId, std::string UID) {
	for (auto itr = m_OrgGiftMap.lower_bound(nPlayerId); itr != m_OrgGiftMap.upper_bound(nPlayerId); ++itr) {
		if (itr->second && itr->second->UID == UID) {
			SaveOrgDB(ORGGIFTDB.DelSql(itr->second));
			m_OrgGiftMap.erase(itr);
			break;
		}
	}		
}

OrgGiftRef Org::GetGift(S32 nPlayerId, std::string UID) {
	for (auto itr = m_OrgGiftMap.lower_bound(nPlayerId); itr != m_OrgGiftMap.upper_bound(nPlayerId); ++itr) {
		if (itr->second && itr->second->UID == UID) {
			return itr->second;
		}
	}

	return OrgGiftRef();
}


bool Org::CanOpenBoss() {
	S32 nTime = time(NULL);
	if (nTime >= GetOrg()->OrgBossTime) {
		return true;
	}
	else if(GetOrg()->OrgBossTimes < 3) {
		return true;
	}
	return false;
}

void Org::SetBossTimes() 
{
	S32 nTime = time(NULL);
	if (nTime >= GetOrg()->OrgBossTime) {
		GetOrg()->OrgBossTimes = 1;
		GetOrg()->OrgBossTime = Platform::getNextTime(Platform::INTERVAL_DAY);
	}
	else {
		GetOrg()->OrgBossTimes++;
	}
	SaveOrgDB(ORGDB.UpdateSqlEx(&m_Org, "OrgBossTimes", "OrgBossTime"));
}

OrgApplyRef Org::GetApply(S32 nPlayerId) {
	auto itr = m_OrgApplyMap.find(nPlayerId);
	if (itr != m_OrgApplyMap.end())
	{
		return itr->second;
	}
	return OrgApplyRef();
}

void Org::SetPlayerMaxNum(S32 nVal)
{
	m_Org.MaxPlayerNum = nVal;
	SaveOrgDB(ORGDB.UpdateSqlEx(&m_Org, "MaxPlayerNum"));
}

void Org::SetElitePlayerMaxNum(S32 nVal)
{
	m_Org.MaxEliteNum = nVal;
	SaveOrgDB(ORGDB.UpdateSqlEx(&m_Org, "MaxEliteNum"));
}


void Org::AddSkillEffect(S32 nPreTechId, S32 nAftTechId)
{
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr) {
		if (itr->second)
		{
			Stats preStats;
			Stats aftStats;
			CTechnologyData* preTechData = g_TechnologyDataMgr->getData(nPreTechId);
			if (preTechData)
			{
				preTechData->getStats(preStats);
			}

			CTechnologyData* aftTechData = g_TechnologyDataMgr->getData(nAftTechId);
			if (aftTechData)
			{
				aftTechData->getStats(aftStats);
			}

			aftStats -= preStats;
			if (aftStats.Agi_gPc || aftStats.Str_gPc || aftStats.Int_gPc) {
				CARDMGR->CaculateStats(itr->second->PlayerId);
			}
			BUFFMGR->AddStatus(itr->second->PlayerId, aftStats);
		}
	}
}

void Org::AddPlayerSkillEffect(S32 nPlayerId)
{
	if (!GetMember(nPlayerId))
		return;

	for (auto itr = m_OrgSkillMap.begin(); itr != m_OrgSkillMap.end(); ++itr) {
		if (itr->second) {
			Stats preStats;
			CTechnologyData* preTechData = g_TechnologyDataMgr->getData(MACRO_SKILL_ID(itr->second->SkillId, itr->second->Level));
			if (preTechData)
			{
				preTechData->getStats(preStats);
			}
			BUFFMGR->AddStatus(nPlayerId, preStats);
		}
	}
}

void Org::RemovePlayerSkillEffect(S32 nPlayerId) {
	for (auto itr = m_OrgSkillMap.begin(); itr != m_OrgSkillMap.end(); ++itr) {
		if (itr->second) {
			Stats preStats;
			CTechnologyData* preTechData = g_TechnologyDataMgr->getData(MACRO_SKILL_ID(itr->second->SkillId, itr->second->Level));
			if (preTechData)
			{
				preTechData->getStats(preStats);
			}
			preStats -= preStats;
			BUFFMGR->AddStatus(nPlayerId, preStats);
		}
	}
}

void Org::AddSkill(OrgSkillRef pSkill) {
	m_OrgSkillMap.insert(std::make_pair(pSkill->SkillId, pSkill));
}

OrgSkillRef Org::GetSkill(S32 nSkillI) {
	auto itr = m_OrgSkillMap.find(nSkillI);
	if (itr != m_OrgSkillMap.end()) {
		return itr->second;
	}
	return OrgSkillRef();
}

void Org::SetQq(std::string Name) {
	//检查名字合法性
	if (!StrSafeCheck(Name.c_str(), 256)) {
		return;
	}

	m_Org.Qq = Name;
	SaveOrgDB(ORGDB.UpdateSqlEx(&m_Org, "Qq"));
}

void Org::SetMsg(std::string Name) {
	//检查名字合法性
	if (!StrSafeCheck(Name.c_str(), 256)) {
		return;
	}

	m_Org.Msg = Name;
	SaveOrgDB(ORGDB.UpdateSqlEx(&m_Org, "Msg"));
}

void Org::SetMemo(std::string Name) {
	//检查名字合法性
	if (!StrSafeCheck(Name.c_str(), 256)) {
		return;
	}

	m_Org.Memo = Name;
	SaveOrgDB(ORGDB.UpdateSqlEx(&m_Org, "Memo"));
}

void Org::AddExp(S32 nExp, bool bSaveDB/*=true*/) {
	S32 nAddExp = m_Org.Exp + nExp;
	S32 nLevel = m_Org.Level;
	if (nLevel == MAX_ORG_LEVEL) {
		return;
	}


	do
	{
		auto pData = g_OrgExpDataMgr->getData(nLevel);
		if (!pData)
			return;

		if (nAddExp >= pData->m_Exp) {
			nAddExp -= pData->m_Exp;
			nLevel = mClamp(nLevel + 1, 0, MAX_ORG_LEVEL);
		}
		else {
			break;
		}

		if (nLevel >= MAX_ORG_LEVEL) {
			nAddExp = 0;
			break;
		}
	} while (nAddExp);

	m_Org.Exp = nAddExp;
	m_Org.Level = nLevel;

	if (bSaveDB)
		SaveOrgDB(ORGDB.UpdateSqlEx(&m_Org, "Exp", "Level"));
}

void Org::AddSkillPoint(S32 nVal, bool bSaveDB/*=true*/) {
	m_Org.SkillPoint = mClamp(m_Org.SkillPoint + nVal, 0, S32_MAX);
	if (bSaveDB)
		SaveOrgDB(ORGDB.UpdateSqlEx(&m_Org, "SkillPoint"));
}

void Org::AddContribute(S32 nPlayerId, S32 nVal, bool bSaveDB/*=true*/) {
	auto pMember = GetMember(nPlayerId);
	if (pMember) {
		pMember->Contribute = mClamp(S32(pMember->Contribute + nVal), S32(0), S32(S32_MAX));

		if (bSaveDB)
			SaveOrgDB(ORGMEMBERDB.UpdateSqlEx(pMember, "Contribute"));
	}
}

void Org::LeaveUpSkill(S32 nSkillId) {

}

void Org::SetMemberLevel(S32 nPlayerId, enMemLevel level) {
	auto pMember = GetMember(nPlayerId);
	if (pMember) {
		pMember->MemberLevel = level;
		SaveOrgDB(ORGMEMBERDB.UpdateSqlEx(pMember, "MemberLevel"));
	}
}

void Org::CaculteBattle() {
	S32 nBattleVal = 0;
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr) {
		if (itr->second)
		{
			nBattleVal += itr->second->BattleVal;
		}
	}

	if (GetOrg()->BattleVal < nBattleVal) {
		GetOrg()->BattleVal = nBattleVal;
		SaveOrgDB(ORGDB.UpdateSqlEx(GetOrg(), "BattleVal"));
	}
}

void Org::WriteOrgLog(S32 OrgId, S32 SourceID, S32 TargetID, S32 OptType, S32 Val0, S32 Val1) {
	OrgLogRef pLog = OrgLogRef(new stOrgLog);
	pLog->OrgId = OrgId;
	if (SourceID > 0)
	{
		PlayerSimpleDataRef pPlayer = SERVER->GetPlayerManager()->GetPlayerData(SourceID);
		if (pPlayer)
		{
			pLog->SourceName = pPlayer->PlayerName;
		}
	}
	if (TargetID > 0)
	{
		PlayerSimpleDataRef pPlayer = SERVER->GetPlayerManager()->GetPlayerData(TargetID);
		if (pPlayer)
		{
			pLog->TargetName = pPlayer->PlayerName;
		}
	}

	pLog->CreateTime = time(NULL);
	pLog->OptType = OptType;
	pLog->Val0 = Val0;
	pLog->Val1 = Val1;
	SaveOrgDB(ORGLOGDB.InsertSql(pLog));
	SendUpdateLogToClient(pLog);
	AddLog(pLog);
}

void Org::AddLog(OrgLogRef pLog) {
	m_OrgLogMap.push_back(pLog);
	if (m_OrgLogMap.size() > MAX_ORG_LOG_NUM) {
		OrgLogRef pEndLog = m_OrgLogMap.front();
		if (pEndLog)
		{
			SaveOrgDB(ORGLOGDB.DelSql(pEndLog));
			m_OrgLogMap.pop_front();
		}
	}
}

void Org::SendInitToClient(U32 nPlayerID) {
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ORG_INIT_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		GetOrg()->WriteData(&sendPacket);
		sendPacket.writeInt(m_OrgMemberMap.size(), Base::Bit16);
		for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr) {
			if (sendPacket.writeFlag(itr->second != NULL))
			{
				itr->second->WriteData(&sendPacket);
			}
		}
		sendPacket.writeInt(m_OrgSkillMap.size(), Base::Bit16);
		for (auto itr = m_OrgSkillMap.begin(); itr != m_OrgSkillMap.end(); ++itr) {
			if (sendPacket.writeFlag(itr->second != NULL))
			{
				itr->second->WriteData(&sendPacket);
			}
		}
		sendPacket.writeInt(m_OrgApplyMap.size(), Base::Bit16);
		for (auto itr = m_OrgApplyMap.begin(); itr != m_OrgApplyMap.end(); ++itr) {
			if (sendPacket.writeFlag(itr->second != NULL))
			{
				itr->second->WriteData(&sendPacket);
			}
		}
		sendPacket.writeInt(m_OrgLogMap.size(), Base::Bit16);
		for (auto itr = m_OrgLogMap.begin(); itr != m_OrgLogMap.end(); ++itr) {
			if (sendPacket.writeFlag(*itr != NULL))
			{
				(*itr)->WriteData(&sendPacket);
			}
		}
		std::vector<OrgGiftRef> OrgVec;
		for (auto itr = m_OrgGiftMap.lower_bound(nPlayerID); itr != m_OrgGiftMap.upper_bound(nPlayerID); ++itr) {
			if (itr->second) {
				OrgVec.push_back(itr->second);
			}
		}
	
		sendPacket.writeInt(OrgVec.size(), Base::Bit16);
		for (auto itr = OrgVec.begin(); itr != OrgVec.end(); ++itr) {
			if (sendPacket.writeFlag(*itr != NULL))
			{
				(*itr)->WriteData(&sendPacket);
			}
		}

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

auto SendNotifyToClient = [](S32 nPlayerID, std::string msg, auto pData) {
	if (!pData)
		return;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(4096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 4096);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, msg.c_str(), pAccount->GetAccountId(), SERVICE_CLIENT);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
};

auto SendResponseToClient = [](S32 nPlayerID, std::string msg, auto pData) {
	if (!pData)
		return;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(4096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 4096);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, msg.c_str(), pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(0, Base::Bit16);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
};


auto SendDelNotifyToClient = [](S32 nPlayerID, std::string msg, S32 nID) {
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, msg.c_str(), pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nID, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
};

void Org::SendUpdateOrgToClient() {
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr)
	{
		SendNotifyToClient(itr->first, "WC_UPDATE_ORG_NOTIFY", GetOrg());
	}
}

void Org::SendUpdateMemberToClient(OrgMemberRef pData) {
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr)
	{
		SendNotifyToClient(itr->first, "WC_UPDATE_ORG_MEMBER_NOTIFY", pData);
	}
}

void Org::SendUpdateSkilloClient(OrgSkillRef pData) {
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr)
	{
		SendNotifyToClient(itr->first, "WC_UPDATE_ORG_SKILL_NOTIFY", pData);
		AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(itr->first);
	}
}

void Org::SendUpdateApplyToClient(OrgApplyRef pData) {
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr)
	{
		SendNotifyToClient(itr->first, "WC_UPDATE_ORG_APPLY_NOTIFY", pData);
	}
}

void Org::SendUpdateLogToClient(OrgLogRef pData) {
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr)
	{
		SendNotifyToClient(itr->first, "WC_UPDATE_ORG_LOG_NOTIFY", pData);
	}
}

void Org::SendDelApplyToClient(S32 nID)
{
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr)
	{
		SendDelNotifyToClient(itr->first, "WC_DELETE_ORG_APPLY_NOTIFY", nID);
	}
}

void Org::SendDelMemberToClient(S32 nID)
{
	for (auto itr = m_OrgMemberMap.begin(); itr != m_OrgMemberMap.end(); ++itr)
	{
		SendDelNotifyToClient(itr->first, "WC_DELETE_ORG_MEMBER_NOTIFY", nID);
	}
}

COrgMgr* COrgMgr::Instance()
{
	static COrgMgr s_Mgr;
	return &s_Mgr;
}

enOpError COrgMgr::CreateOrg(S32 nPlayerID, std::string OrgName, std::string QQ, std::string Memo, bool AutoAcceptJoin)
{
	if (FilterOpt::isBanName(OrgName.c_str()))
		return OPERATOR_BAN_WORD;

	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pPlayer || pPlayer->OrgId != 0) {
		return OPERATOR_ALREADY_ORG;
	}

	//检查名字合法性
	if (!StrSafeCheck(OrgName.c_str(), 256)) {
		return OPERATOR_BAN_WORD;
	}

	//通知远程服务器创建联盟
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(512 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 512);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WR_CreateOrg", pAccount->GetAccountId());
		sendPacket.writeString(OrgName);
		sendPacket.writeString(QQ);
		sendPacket.writeString(Memo);
		sendPacket.writeFlag(AutoAcceptJoin);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetRemoteAccountSocket()->Send(sendPacket);
	}

	return OPERATOR_NONE_ERROR;
}

auto AddOrgMember = [](S32 nPlayerID, S32 OrgId, OrgRef pOrg, bool bSaveDB = true) {
	auto nContribute = 0;
	auto bFind = false;
	ORGMGR->GetOrgContribute(nPlayerID, bFind, nContribute);
	OrgMemberRef pMember = OrgMemberRef(new stOrgMember);
	pMember->OrgId = OrgId;
	pMember->PlayerId = nPlayerID;
	PlayerSimpleDataRef pSimPlayer = SERVER->GetPlayerManager()->GetPlayerData(nPlayerID);
	if (pSimPlayer) {
		pMember->PlayerName = pSimPlayer->PlayerName;
		pMember->PlayerFace = pSimPlayer->FirstClass;
		pMember->Vip = pSimPlayer->VipLv;
	}
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (pPlayer) {
		pMember->BattleVal = pPlayer->BattleVal;
	}
	pMember->Contribute = nContribute;
	pOrg->AddMember(pMember);
	if (bFind) {
		SaveOrgDB(ORGMEMBERDB.UpdateSql(pMember));
	}
	else {
		SaveOrgDB(ORGMEMBERDB.InsertSql(pMember));
	}

	if (bSaveDB)
	{
		pOrg->GetOrg()->PlayerNum++;
		SaveOrgDB(ORGDB.UpdateSqlEx(pOrg->GetOrg(), "PlayerNum"));
	}

	CHAT_HELPER->addPlayer(pOrg->GetChannel(), pMember->PlayerId);
	pOrg->CaculteBattle();
	pOrg->SendInitToClient(nPlayerID);
	pOrg->SendUpdateMemberToClient(pMember);
	if(bSaveDB)
		pOrg->WriteOrgLog(OrgId, nPlayerID, 0, stOrgLog::ORG_LOG_JOIN);
	PLAYERINFOMGR->SetPlayerOrg(nPlayerID, OrgId);
	PLAYERINFOMGR->UpdateToClient(nPlayerID);
};

enOpError COrgMgr::CreateOrgHandle(S32 nPlayerID, std::string OrgName, S32 OrgId, std::string QQ, std::string Memo, bool AutoAcceptJoin)
{
	OrgRef pOrg = OrgRef(new Org);
	pOrg->GetOrg()->Id = OrgId;
	pOrg->GetOrg()->Name = OrgName;
	pOrg->GetOrg()->MasterId = nPlayerID;
	pOrg->GetOrg()->PlayerNum = 1;
	pOrg->GetOrg()->Qq = QQ;
	pOrg->GetOrg()->Memo = Memo;
	pOrg->GetOrg()->AutoAcceptJoin = AutoAcceptJoin;
	TBLExecution tHandle(SERVER->GetActorDB());
	tHandle.SetSql(ORGDB.InsertSql(pOrg->GetOrg()));
	tHandle.SetId(OrgId);
	tHandle.RegisterFunction([&](int id, int error, void* pH)
	{
		CDBConn* pHandle = (CDBConn*)(pH);
		if (pHandle && error == NONE_ERROR)
		{
			pHandle->Exec();
			CHAT_HELPER->registerChannel(CHAT_MSG_TYPE_ORG, Org::RegisterOrgChannelCallback, (void*)id);
			ORGMGR->AddOrg(pOrg);
			AddOrgMember(nPlayerID, OrgId, pOrg, false);
		}
		else {
			//删除org
			CMemGuard Buffer(128 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 128);
			stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WR_DeleteOrg", id);
			pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER->GetRemoteAccountSocket()->Send(sendPacket);
			ORGMGR->DeleteOrg(id);
		}
	});
	tHandle.Commint();
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::ChangeOrg(S32 nPlayerID, S32 OrgId, std::string QQ, std::string Memo, std::string Msg, bool AutoAcceptJoin) {
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	if (!pOrg->IsMasterId(nPlayerID) && !pOrg->IsSubMasterId(nPlayerID))
		return OPERATOR_ORG_MASTER_NOT;

	pOrg->GetOrg()->Qq = QQ;
	pOrg->GetOrg()->Msg = Msg;
	pOrg->GetOrg()->Memo = Memo;
	pOrg->GetOrg()->AutoAcceptJoin = AutoAcceptJoin;
	SaveOrgDB(ORGDB.UpdateSqlEx(pOrg->GetOrg(), "Qq", "Msg", "Memo", "AutoAcceptJoin"));
	pOrg->SendUpdateOrgToClient();
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::ApplyOrg(S32 nPlayerID, S32 OrgId) {
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	PlayerSimpleDataRef pSimPlayer = SERVER->GetPlayerManager()->GetPlayerData(nPlayerID);
	if (!pPlayer || !pSimPlayer || pPlayer->OrgId != 0) {
		return OPERATOR_ALREADY_ORG;
	}

	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	if (pOrg->GetOrg()->AutoAcceptJoin == 1)
	{
		if (pOrg->GetOrg()->PlayerNum >= pOrg->GetOrg()->MaxPlayerNum)
			return OPERATOR_ORG_MAX_PLAYER;

		AddOrgMember(nPlayerID, OrgId, pOrg);
		return OPERATOR_NONE_ERROR;
	}

	auto pApply = pOrg->GetApply(nPlayerID);
	if (pApply)
		return OPERATOR_ORG_APPLY_ALREADY;

	pApply = OrgApplyRef(new stOrgApply);
	pApply->OrgId = OrgId;
	pApply->PlayerId = nPlayerID;
	pApply->PlayerName = pSimPlayer->PlayerName;
	pApply->PlayerFace = pSimPlayer->FirstClass;
	pApply->Vip = pSimPlayer->VipLv;
	pApply->OnlineFlag = 1;
	pApply->BattleVal = pPlayer->BattleVal;
	ORGAPPLYDB.InsertSql(pApply);
	pOrg->AddApply(pApply);
	pOrg->SendUpdateApplyToClient(pApply);
	pOrg->WriteOrgLog(OrgId, nPlayerID, 0, stOrgLog::ORG_LOG_APPLY);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::JoinOrg(S32 nPlayerID, S32 OrgId, S32 nApplyPlayerID, bool bAgree/*=false*/) {
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	if (!pOrg->IsMasterId(nPlayerID) && !pOrg->IsSubMasterId(nPlayerID))
		return OPERATOR_ORG_MASTER_NOT;

	if (pOrg->GetMember(nApplyPlayerID))
		return OPERATOR_ORG_MEMBER_NOT;

	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetPlayer(nApplyPlayerID);
	if (!pPlayer)
		return OPERATOR_ORG_MEMBER_NOT;

	if (bAgree) {
		if (pPlayer->OrgId != 0)
			return OPERATOR_ALREADY_ORG;

		if (pOrg->GetOrg()->PlayerNum >= pOrg->GetOrg()->MaxPlayerNum)
			return OPERATOR_ORG_MAX_PLAYER;

		AddOrgMember(nApplyPlayerID, OrgId, pOrg);
		pOrg->WriteOrgLog(OrgId, nPlayerID, nApplyPlayerID, stOrgLog::ORG_LOG_AGREEAPPLY);
	}
	else {
		pOrg->WriteOrgLog(OrgId, nPlayerID, nApplyPlayerID, stOrgLog::ORG_LOG_REFUSEAPPLY);
	}

	pOrg->RemoveApply(nApplyPlayerID);
	pOrg->SendDelApplyToClient(nApplyPlayerID);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::LeaveOrg(S32 nPlayerID, S32 OrgId) {
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	if (pOrg->IsMasterId(nPlayerID))
		return OPERATOR_ORG_MASTER;

	pOrg->RemoveMember(nPlayerID, true);
	pOrg->WriteOrgLog(OrgId, nPlayerID, 0, stOrgLog::ORG_LOG_LEAVE);
	pOrg->SendUpdateOrgToClient();
	pOrg->SendDelMemberToClient(nPlayerID);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::FireMmeber(S32 nPlayerID, S32 OrgId, S32 nFirePlayerID) {
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	if (nPlayerID == nFirePlayerID)
		return OPERATOR_PARAMETER_ERROR;

	if (!pOrg->IsMasterId(nPlayerID) && !pOrg->IsSubMasterId(nPlayerID))
		return OPERATOR_ORG_MASTER_NOT;

	if (pOrg->IsMasterId(nFirePlayerID) || pOrg->IsSubMasterId(nFirePlayerID))
		return OPERATOR_ORG_MASTER2;

	pOrg->RemoveMember(nFirePlayerID, false);
	pOrg->WriteOrgLog(OrgId, nPlayerID, nFirePlayerID, stOrgLog::ORG_LOG_KICK);
	pOrg->SendUpdateOrgToClient();
	pOrg->SendDelMemberToClient(nFirePlayerID);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::DonateOrg(S32 nPlayerID, S32 OrgId, enDonateType nType) {
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	auto pMember = pOrg->GetMember(nPlayerID);
	if (!pMember)
		return OPERATOR_ORG_MEMBER_NOT;

	bool bCostGold = s_DonateType[nType][0] == 0;
	if (bCostGold) {
		if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -s_DonateType[nType][1]))
			return OPERATOR_HASNO_GOLD;

		auto pTimer = TIMESETMGR->GetData(nPlayerID, ORG_DONATE_TIME);
		if (pTimer)
			return OPERATOR_ORG_DONATE_MAX;
	}
	else {
		if (!ITEMMGR->CanReduceItem(nPlayerID, s_DonateType[nType][1], 1))
			return OPERATOR_HASNO_ITEM;
	}

	auto pStats = BUFFMGR->GetData(nPlayerID);
	S32 nAddContribute = s_DonateType[nType][4] * (1.0f + ((pStats) ? (pStats->OrgContribute_gPc	/ 100.0f) : 0.0f));
	S32 nAddExp		   = s_DonateType[nType][2] * (1.0f + ((pStats) ? (pStats->OrgContributeExp_gPc / 100.0f) : 0.0f));
	pOrg->AddContribute(nPlayerID, nAddContribute);
	pOrg->AddSkillPoint(s_DonateType[nType][3], false);
	pOrg->AddExp(nAddExp, false);
	pOrg->AddGiftExp(s_DonateType[nType][5]);
	SaveOrgDB(ORGDB.UpdateSqlEx(pOrg->GetOrg(), "SkillPoint", "Exp", "Level"));
	if (bCostGold) {
		SERVER->GetPlayerManager()->AddGold(nPlayerID, -s_DonateType[nType][1], LOG_GOLD_ORG);
		TIMESETMGR->AddTimeSet(nPlayerID, ORG_DONATE_TIME, Platform::getNextTime(Platform::INTERVAL_DAY) - time(NULL), "", nType);
	}
	else {
		ITEMMGR->AddItem(nPlayerID, s_DonateType[nType][1], -1);
	}
	pOrg->WriteOrgLog(OrgId, nPlayerID, 0, stOrgLog::ORG_LOG_DONATE, nType);
	pOrg->SendUpdateOrgToClient();
	pOrg->SendUpdateMemberToClient(pMember);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::SetOrgMasterLevel(S32 nPlayerID, S32 OrgId, S32 nTargetPlayerID, Org::enMemLevel enLevel) {
	if (nPlayerID == nTargetPlayerID)
		return OPERATOR_PARAMETER_ERROR;

	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	if (!pOrg->IsMasterId(nPlayerID))
		return OPERATOR_ORG_MASTER_NOT;

	auto pMember = pOrg->GetMember(nTargetPlayerID);
	if (!pMember)
		return OPERATOR_ORG_MEMBER_NOT;

	if (enLevel == Org::SUBMASTER)
	{
		if (pOrg->GetPlayerNum(enLevel) >= pOrg->GetMaxSubMasterNum())
			return OPERATOR_PARAMETER_ERROR;
	}
	else if (enLevel == Org::ELITE) {
		if (pOrg->GetPlayerNum(enLevel) >= pOrg->GetMaxEliteNum())
			return OPERATOR_PARAMETER_ERROR;
	}


	pOrg->SetMemberLevel(nTargetPlayerID, enLevel);
	pOrg->WriteOrgLog(OrgId, nPlayerID, nTargetPlayerID, stOrgLog::ORG_LOG_MEMBERLEAVE, enLevel);
	pOrg->SendUpdateMemberToClient(pMember);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::TransferMaster(S32 nPlayerID, S32 OrgId, S32 nTransferPlayerID) {
	if (nPlayerID == nTransferPlayerID)
		return OPERATOR_PARAMETER_ERROR;

	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	if (!pOrg->IsMasterId(nPlayerID))
		return OPERATOR_ORG_MASTER_NOT;

	auto pMember = pOrg->GetMember(nTransferPlayerID);
	if (!pMember)
		return OPERATOR_ORG_MEMBER_NOT;

	pOrg->SetMemberLevel(nTransferPlayerID, Org::NORMAL);
	pOrg->GetOrg()->MasterId = nTransferPlayerID;
	pOrg->GetOrg()->TransferTime = time(NULL);
	pOrg->SetMemberLevel(nPlayerID, Org::NORMAL);
	pOrg->SetMemberLevel(nTransferPlayerID, Org::NORMAL);
	SaveOrgDB(ORGDB.UpdateSqlEx(pOrg->GetOrg(), "MasterId", "TransferTime"));
	pOrg->WriteOrgLog(OrgId, nPlayerID, nTransferPlayerID, stOrgLog::ORG_LOG_TRANSFER);
	pOrg->SendUpdateOrgToClient();
	pOrg->SendUpdateMemberToClient(pMember);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::LeaveUpSkill(S32 nPlayerID, S32 OrgId, S32 nTechSeries)
{
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	if (!pOrg->IsMasterId(nPlayerID) && !pOrg->IsSubMasterId(nPlayerID))
		return OPERATOR_ORG_MASTER_NOT;

	auto nTechId = MACRO_SKILL_ID(nTechSeries, 1);
	auto pSkill = pOrg->GetSkill(nTechSeries);
	if (pSkill) {
		if (pSkill->Level >= Org::MAX_SKILL_LEVEL)
			return OPERATOR_PARAMETER_ERROR;

		nTechId = MACRO_SKILL_ID(nTechSeries, pSkill->Level + 1);
	}

	CTechnologyData* pTechData = g_TechnologyDataMgr->getData(nTechId);
	if (!pTechData)
		return OPERATOR_PARAMETER_ERROR;

	if (pOrg->GetOrg()->Level < pTechData->m_Int)
		return OPERATOR_ORG_LEAVE;

	if (pOrg->GetOrg()->SkillPoint < pTechData->m_Ore)
		return OPERATOR_ORG_SKILLPOINT;

	Stats stats;
	pTechData->getStats(stats);
	if (stats.OrgPlayerNum > 0) {
		pOrg->SetPlayerMaxNum(stats.OrgPlayerNum + 10);
	}
	else if (stats.OrgManagerNum > 0)
	{
		pOrg->SetElitePlayerMaxNum(stats.OrgManagerNum);
	}

	if (!pSkill)
	{
		pSkill = OrgSkillRef(new stOrgSkill);
		pSkill->Level = 1;
		pSkill->SkillId = nTechSeries;
		pSkill->OrgId = OrgId;
		pOrg->AddSkill(pSkill);
		pOrg->AddSkillEffect(0, nTechId);
		SaveOrgDB(ORGSKILLDB.InsertSql(pSkill));
	}
	else {
		pOrg->AddSkillEffect(MACRO_SKILL_ID(nTechSeries, pSkill->Level), nTechId);
		pSkill->Level += 1;
		SaveOrgDB(ORGSKILLDB.UpdateSqlEx(pSkill, "Level"));
	}

	pOrg->AddSkillPoint(-pTechData->m_Ore);
	pOrg->WriteOrgLog(OrgId, nPlayerID, 0, stOrgLog::ORG_LOG_SKILL, nTechSeries, pSkill->Level);
	pOrg->SendUpdateOrgToClient();
	pOrg->SendUpdateSkilloClient(pSkill);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::DisbandOrg(S32 nPlayerID, S32 OrgId) {
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	if (!pOrg->IsMasterId(nPlayerID))
		return OPERATOR_ORG_MASTER_NOT;

	pOrg->Disband();
	m_OrgNameMap.erase(pOrg->GetOrgName());
	m_OrgMap.erase(OrgId);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::SearchTopOrg(S32 nPlayerID)
{
	auto LoadTopOrgDB = []() {
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetId(0);
		tHandle.SetSql("SELECT TOP 10 Id, Level, Name, MasterId, BattleVal, PlayerNum, MaxPlayerNum, AutoAcceptJoin  FROM Tbl_Org ORDER BY Level DESC, BattleVal DESC");
		tHandle.RegisterFunction([](int id, int error, void * pEx)
		{
			CDBConn * pHandle = (CDBConn*)(pEx);
			if (pHandle && error == NONE_ERROR)
			{
				ORG_VEC& TopOrgVec = ORGMGR->GetTopOrgVec();
				TopOrgVec.clear();
				while (pHandle->More())
				{
	
					SimpleOrgRef pOrg = SimpleOrgRef(new stSimpleOrg);
					pOrg->Id = pHandle->GetInt();
					pOrg->Level = pHandle->GetInt();
					pOrg->Name = pHandle->GetString();
					pOrg->MasterId = pHandle->GetInt();
					pOrg->BattleVal = pHandle->GetInt();
					pOrg->PlayerNum = pHandle->GetInt();
					pOrg->MaxPlayerNum = pHandle->GetInt();
					pOrg->MasterName = OrgGetPlayerName(pOrg->MasterId);
					pOrg->AutoAcceptJoin = pHandle->GetInt();
					TopOrgVec.push_back(pOrg);
				}
			}
		});
		tHandle.Commint();
	};
	if (m_TopSearchTimer.CheckTimer())
	{
		LoadTopOrgDB();
	}

	//通知客户端
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_SearchOrg_Response", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(0, Base::Bit16);
		sendPacket.writeInt(m_TopOrgVec.size(), Base::Bit16);
		for (auto itr = m_TopOrgVec.begin(); itr != m_TopOrgVec.end(); ++itr) {
			if (sendPacket.writeFlag((*itr) != NULL)) {
				(*itr)->WriteData(&sendPacket);
			}
		}
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::SearchOrg(S32 nPlayerID) {
	auto LoadOrgCache = []() {
		ORG_VEC& OrgVec = ORGMGR->GetOrgVec();
		ORG_MAP& OrgMap = ORGMGR->GetOrgMap();
		OrgVec.clear();
		for (auto itr = OrgMap.begin(); itr != OrgMap.end(); ++itr) {
			auto pDatat = itr->second;
			if (pDatat) {
				SimpleOrgRef pOrg = SimpleOrgRef(new stSimpleOrg(pDatat->GetOrg()));
				OrgVec.push_back(pOrg);
			}
		}
	};

	if (m_SearchTimer.CheckTimer())
	{
		LoadOrgCache();
	}

	//通知客户端
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_SearchOrg_Response", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(0, Base::Bit16);
		sendPacket.writeInt(m_OrgVec.size(), Base::Bit16);
		for (auto itr = m_OrgVec.begin(); itr != m_OrgVec.end(); ++itr) {
			if (sendPacket.writeFlag((*itr) != NULL)) {
				(*itr)->WriteData(&sendPacket);
			}
		}
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::BuyOrgItem(S32 nPlayerID, S32 OrgId, S32 nShopId)
{
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	CRandomShopData* pData = g_RandomShopDataMgr->getData(nShopId);
	if (!pData)
		return OPERATOR_PARAMETER_ERROR;

	if (pData->m_SaleType != 2)
		return OPERATOR_PARAMETER_ERROR;

	if (pOrg->GetOrg()->Level < pData->m_RandVal)
		return OPERATOR_ORG_LEAVE;

	auto pMember = pOrg->GetMember(nPlayerID);
	if (!pMember)
		return OPERATOR_ORG_MEMBER_NOT;

	if (pMember->Contribute < pData->m_Sale)
		return OPERATOR_ORG_CONTRIBUTE;

	pOrg->AddContribute(nPlayerID, -pData->m_Sale);
	ITEMMGR->AddItem(nPlayerID, pData->m_ItemId, pData->m_ItemNum);
	pOrg->WriteOrgLog(OrgId, nPlayerID, 0, stOrgLog::ORG_LOG_BUYITEM, nShopId);
	pOrg->SendUpdateMemberToClient(pMember);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::BuyOrgGift(S32 nPlayerID, S32 OrgId) {
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	auto pMember = pOrg->GetMember(nPlayerID);
	if (!pMember)
		return OPERATOR_ORG_MEMBER_NOT;

	if (pMember->GiftExp < 10000)
		return OPERATOR_ORG_GIFTEXP;

	pMember->GiftExp = mClamp(pMember->GiftExp - 10000, 0, S32_MAX);
	SaveOrgDB(ORGMEMBERDB.UpdateSqlEx(pMember, "GiftExp"));
	SERVER->GetPlayerManager()->AddGold(nPlayerID, 1000);
	pOrg->WriteOrgLog(OrgId, nPlayerID, 0, stOrgLog::ORG_LOG_BUYGIFT);
	pOrg->SendUpdateMemberToClient(pMember);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::AddOrgGift(S32 OrgId, S32 GiftId, std::string Name)
{
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	pOrg->AddGift(GiftId, Name);
	return OPERATOR_NONE_ERROR;
}

enOpError COrgMgr::GetOrgGift(S32 nPlayerID, S32 OrgId, std::string UID, S32& GiftId) {
	auto pOrg = GetOrg(OrgId);
	if (!pOrg)
		return OPERATOR_ORG_NO;

	auto pGift = pOrg->GetGift(nPlayerID, UID);
	if (pGift)
	{
		if (pGift->GiftId >= 680000) {
			ITEMMGR->AddGift(nPlayerID, pGift->GiftId, 1);
		}
		else if (pGift->GiftId >= 300000) {
			ITEMMGR->DropItem(nPlayerID, pGift->GiftId);
		}
		else {
			ITEMMGR->AddItem(nPlayerID, pGift->GiftId, 1);
		}
		GiftId = pGift->GiftId;
		pOrg->RemoveGift(nPlayerID, UID);
	}
	return OPERATOR_NONE_ERROR;
}

OrgRef COrgMgr::GetOrg(S32 OrgId)
{
	auto itr = m_OrgMap.find(OrgId);
	if (itr != m_OrgMap.end())
		return itr->second;

	return OrgRef();
}

void COrgMgr::GetOrgContribute(S32 nPlayerID, bool& bFind, S32& Contribute) {
	Contribute = 0;
	bFind = false;
	TBLExecution tHandle(SERVER->GetActorDB());
	tHandle.SetId(0);
	tHandle.SetSql(fmt::sprintf("SELECT Contribute FROM Tbl_OrgMember WHERE PlayerId = %d", nPlayerID));
	tHandle.RegisterFunction([&Contribute, &bFind](int id, int error, void * pEx)
	{
		CDBConn * pHandle = (CDBConn*)(pEx);
		if (pHandle && error == NONE_ERROR)
		{
			if (pHandle->More())
			{
				Contribute = pHandle->GetInt();
				bFind = true;
			}
		}
	});
	tHandle.Commint();
}

void COrgMgr::AddOrg(OrgRef pData)
{
	m_OrgNameMap.insert(ORG_NAME_MAP::value_type(pData->GetOrgName(), pData->GetOrgId()));
	m_OrgMap.insert(ORG_MAP::value_type(pData->GetOrgId(), pData));
}

void COrgMgr::DeleteOrg(S32 OrgId)
{
	m_OrgMap.erase(OrgId);
}

void COrgMgr::RegisterChannel()
{
	for (auto iter = m_OrgMap.begin(); iter != m_OrgMap.end(); ++iter)
	{
		CHAT_HELPER->registerChannel(CHAT_MSG_TYPE_ORG, Org::RegisterOrgChannelCallback, (void*)iter->second->GetOrgId());
	}
}

void COrgMgr::AddOrgSkill(S32 nPlayerID, S32 OrgId) {
	auto pOrg = ORGMGR->GetOrg(OrgId);
	if (pOrg)
	{
		pOrg->AddPlayerSkillEffect(nPlayerID);
	}
}

void COrgMgr::AddGiftExp(S32 OrgId, S32 Gold) {
	auto pOrg = ORGMGR->GetOrg(OrgId);
	if (pOrg)
	{
		pOrg->AddGiftExp(Gold * 2);
	}
}

void COrgMgr::UpdateToClient(U32 nPlayerID, OrgRef pData)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_CITY_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		//pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void COrgMgr::SendInitToClient(U32 nPlayerID)
{
	auto pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (pPlayer && pPlayer->OrgId) {
		auto pOrg = ORGMGR->GetOrg(pPlayer->OrgId);
		if (pOrg)
		{
			pOrg->SendInitToClient(nPlayerID);
		}
	}
}

void COrgMgr::LoadDB()
{
	//加载联盟
	{
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetId(0);
		tHandle.SetSql(ORGDB.LoadSql());
		tHandle.RegisterFunction([](int id, int error, void * pEx)
		{
			CDBConn * pHandle = (CDBConn*)(pEx);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					OrgRef pOrg = OrgRef(new Org);
					pOrg->GetOrg()->Id = pHandle->GetInt();
					pOrg->GetOrg()->Level = pHandle->GetInt();
					pOrg->GetOrg()->Exp = pHandle->GetInt();
					pOrg->GetOrg()->SkillPoint = pHandle->GetInt();
					pOrg->GetOrg()->Name = pHandle->GetString();
					pOrg->GetOrg()->Qq = pHandle->GetString();
					pOrg->GetOrg()->Msg = pHandle->GetString();
					pOrg->GetOrg()->Memo = pHandle->GetString();
					pOrg->GetOrg()->MasterId = pHandle->GetInt();
					pOrg->GetOrg()->TransferTime = pHandle->GetTime();
					pOrg->GetOrg()->BattleVal = pHandle->GetInt();
					pOrg->GetOrg()->PlayerNum = pHandle->GetInt();
					pOrg->GetOrg()->MaxPlayerNum = pHandle->GetInt();
					pOrg->GetOrg()->MaxEliteNum = pHandle->GetInt();
					pOrg->GetOrg()->AutoAcceptJoin = pHandle->GetInt();
					pOrg->GetOrg()->OrgBossTimes = pHandle->GetInt();
					pOrg->GetOrg()->OrgBossTime  = pHandle->GetTime();
					ORGMGR->AddOrg(pOrg);
				}
			}
		});
		tHandle.Commint();
		RegisterChannel();
	}

	//加载成员
	{
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetId(0);
		tHandle.SetSql(ORGMEMBERDB.LoadSql());
		tHandle.RegisterFunction([](int id, int error, void * pEx)
		{
			CDBConn * pHandle = (CDBConn*)(pEx);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					OrgMemberRef pMember = OrgMemberRef(new stOrgMember);
					pMember->PlayerId = pHandle->GetInt();
					pMember->OrgId = pHandle->GetInt();
					pMember->PlayerName = pHandle->GetString();
					pMember->Contribute = pHandle->GetInt();
					pMember->GiftExp = pHandle->GetInt();
					pMember->MemberLevel = pHandle->GetInt();
					pMember->MemberStatus = pHandle->GetInt();
					pMember->BattleVal = pHandle->GetInt();
					pMember->Vip = pHandle->GetInt();
					pMember->PlayerFace = pHandle->GetInt();
					pMember->LeaveTime = pHandle->GetTime();
					auto pOrg = ORGMGR->GetOrg(pMember->OrgId);
					if(pOrg)
						pOrg->AddMember(pMember);
				}
			}
		});
		tHandle.Commint();
	}

	//加载技能
	{
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetId(0);
		tHandle.SetSql(ORGSKILLDB.LoadSql());
		tHandle.RegisterFunction([](int id, int error, void * pEx)
		{
			CDBConn * pHandle = (CDBConn*)(pEx);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					OrgSkillRef pSkill = OrgSkillRef(new stOrgSkill);
					pSkill->OrgId = pHandle->GetInt();
					pSkill->SkillId = pHandle->GetInt();
					pSkill->Level = pHandle->GetInt();
					auto pOrg = ORGMGR->GetOrg(pSkill->OrgId);
					if (pOrg)
						pOrg->AddSkill(pSkill);
				}
			}
		});
		tHandle.Commint();
	}

	//加载申请
	{
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetId(0);
		tHandle.SetSql(ORGAPPLYDB.LoadSql());
		tHandle.RegisterFunction([](int id, int error, void * pEx)
		{
			CDBConn * pHandle = (CDBConn*)(pEx);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					OrgApplyRef pApply = OrgApplyRef(new stOrgApply);
					pApply->PlayerId = pHandle->GetInt();
					pApply->OrgId = pHandle->GetInt();
					pApply->PlayerName = pHandle->GetString();
					pApply->BattleVal = pHandle->GetInt();
					pApply->ApplyTime = pHandle->GetTime();
					pApply->Vip = pHandle->GetInt();
					pApply->PlayerFace = pHandle->GetInt();
					pApply->OnlineFlag = pHandle->GetInt();
					auto pOrg = ORGMGR->GetOrg(pApply->OrgId);
					if (pOrg)
						pOrg->AddApply(pApply);
				}
			}
		});
		tHandle.Commint();
	}

	//加载Log
	{
		std::string sqlstr = ORGLOGDB.LoadSql();
		sqlstr += " ORDER BY CreateTime DESC";
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetId(0);
		tHandle.SetSql(sqlstr);
		tHandle.RegisterFunction([](int id, int error, void * pEx)
		{
			CDBConn * pHandle = (CDBConn*)(pEx);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					OrgLogRef pLog = OrgLogRef(new stOrgLog);
					pLog->OrgId = pHandle->GetInt();
					pLog->CreateTime = pHandle->GetInt();
					pLog->SourceName = pHandle->GetString();
					pLog->TargetName = pHandle->GetString();
					pLog->OptType = pHandle->GetInt();
					pLog->Val0 = pHandle->GetInt();
					pLog->Val1 = pHandle->GetInt();
					auto pOrg = ORGMGR->GetOrg(pLog->OrgId);
					if (pOrg)
						pOrg->AddLog(pLog);
				}
			}
		});
		tHandle.Commint();
	}

	//加载Gift
	{
		std::string sqlstr = ORGGIFTDB.LoadSql();
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetId(0);
		tHandle.SetSql(sqlstr);
		tHandle.RegisterFunction([](int id, int error, void * pEx)
		{
			CDBConn * pHandle = (CDBConn*)(pEx);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					OrgGiftRef pGift = OrgGiftRef(new stOrgGift);
					pGift->OrgId = pHandle->GetInt();
					pGift->PlayerId = pHandle->GetInt();
					pGift->UID = pHandle->GetString();
					pGift->GiftId = pHandle->GetInt();
					pGift->PlayerName = pHandle->GetString();
					auto pOrg = ORGMGR->GetOrg(pGift->OrgId);
					if (pOrg)
						pOrg->AddGift(pGift);
				}
			}
		});
		tHandle.Commint();
	}


	//联盟初始化
}

bool COrgMgr::HandleCreateOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	auto OrgName = Util::Utf8ToMbcs(Packet->readString().c_str());
	auto QQ = Util::Utf8ToMbcs(Packet->readString().c_str());
	auto Memo = Util::Utf8ToMbcs(Packet->readString().c_str());
	auto AutoAcceptJoin = Packet->readFlag();
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	S32 nError = 0;
	if (pAccount)
	{
		nError = CreateOrg(pAccount->GetPlayerId(), OrgName, QQ, Memo, AutoAcceptJoin);
		if (nError != OPERATOR_NONE_ERROR) {
			SendOrgResponse("WC_CreateOrg_RESPONSE", nError, pAccount);
		}
	}
	return true;
}

bool COrgMgr::HandleRCreateOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = pHead->DestZoneId;
	auto Name = Packet->readString();
	auto QQ = Packet->readString();
	auto Memo = Packet->readString();
	auto AutoAcceptJoin = Packet->readFlag();
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		if (OrgId != 0) {
			ORGMGR->CreateOrgHandle(pAccount->GetPlayerId(), Name, OrgId, QQ, Memo, AutoAcceptJoin);
			SendOrgResponse("WC_CreateOrg_RESPONSE", OPERATOR_NONE_ERROR, pAccount);
		}
		else {
			SendOrgResponse("WC_CreateOrg_RESPONSE", OPERATOR_CREATE_ORG_FAILED, pAccount);
		}
	}

	return true;
}

bool COrgMgr::HandleChangeOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	std::string Qq = Util::Utf8ToMbcs(Packet->readString());
	std::string Memo = Util::Utf8ToMbcs(Packet->readString());
	std::string Msg = Util::Utf8ToMbcs(Packet->readString());
	S8 AutoAcceptJoin = Packet->readInt(8);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = ChangeOrg(pAccount->GetPlayerId(), OrgId, Qq, Memo, Msg, AutoAcceptJoin);
		SendOrgResponse("WC_ChangeOrg_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleApplyOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = ApplyOrg(pAccount->GetPlayerId(), OrgId);
		SendOrgResponse("WC_ApplyOrg_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleJoinOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 nApplyPlayerID = Packet->readInt(Base::Bit32);
	bool bApply = Packet->readFlag();
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = JoinOrg(pAccount->GetPlayerId(), OrgId, nApplyPlayerID, bApply);
		SendOrgResponse("WC_ApplyOrg_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleLeaveOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = LeaveOrg(pAccount->GetPlayerId(), OrgId);
		SendOrgResponse("WC_LeaveOrg_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleFireMemberOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 nFirePlayerID = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = FireMmeber(pAccount->GetPlayerId(), OrgId, nFirePlayerID);
		SendOrgResponse("WC_FireMemberOrg_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleDonateOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	enDonateType type = (enDonateType)Packet->readInt(Base::Bit8);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = DonateOrg(pAccount->GetPlayerId(), OrgId, type);
		SendOrgResponse("WC_DonateOrg_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleSetOrgMasterLevelResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 nTargetPlayerID = Packet->readInt(Base::Bit32);
	Org::enMemLevel level = (Org::enMemLevel)Packet->readInt(Base::Bit8);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = SetOrgMasterLevel(pAccount->GetPlayerId(), OrgId, nTargetPlayerID, level);
		SendOrgResponse("WC_SetOrgMasterLevel_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleTransferOrgMasterResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 nTargetPlayerID = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = TransferMaster(pAccount->GetPlayerId(), OrgId, nTargetPlayerID);
		SendOrgResponse("WC_TransferOrgMaster_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleLeaveUpOrgSkillResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 nSeriesId = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = LeaveUpSkill(pAccount->GetPlayerId(), OrgId, nSeriesId);
		SendOrgResponse("WC_LeaveUpSkill_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleDisbandOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = DisbandOrg(pAccount->GetPlayerId(), OrgId);
		SendOrgResponse("WC_DisbandOrg_Response", nError, pAccount);
	}

	return true;
}

bool COrgMgr::HandleSearchTopOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = SearchTopOrg(pAccount->GetPlayerId());
	}

	return true;
}

bool COrgMgr::HandleSearchOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = SearchOrg(pAccount->GetPlayerId());
	}

	return true;
}

bool COrgMgr::HandleSearchOrgSimpleResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 nError = 0;
	std::string OrgName = Util::Utf8ToMbcs(Packet->readString());
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		S32 OrgId = 0;
		auto itr = m_OrgNameMap.find(OrgName);
		if (itr != m_OrgNameMap.end())
		{
			OrgId = itr->second;
		}else {
			OrgId = atoi(OrgName.c_str());
		}

		auto pOrg = GetOrg(OrgId);
		if (pOrg)
		{
			SimpleOrgRef pData = SimpleOrgRef(new stSimpleOrg(pOrg->GetOrg()));
			SendResponseToClient(pAccount->GetPlayerId(), "WC_SearchOrgSimple_Response", pData);
		}
		else
		{
			SendOrgResponse("WC_SearchOrgSimple_Response", OPERATOR_ORG_NOFOUNT, pAccount);
		}
	}

	return true;
}

bool COrgMgr::HandleBuyOrgItemResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 ShopId = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = BuyOrgItem(pAccount->GetPlayerId(), OrgId, ShopId);
	}

	SendOrgResponse("WC_BuyOrgItem_Response", nError, pAccount);

	return true;
}

bool COrgMgr::HandleBuyOrgGiftResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = BuyOrgGift(pAccount->GetPlayerId(), OrgId);
	}

	SendOrgResponse("WC_BuyOrgGift_Response", nError, pAccount);

	return true;
}

bool COrgMgr::HandleGetOrgGiftResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	std::string UID = Packet->readString();
	S32 nError = 0, nGiftId = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = GetOrgGift(pAccount->GetPlayerId(), OrgId, UID, nGiftId);
	}

	{
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_GetOrgGift_Response", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeString(UID);
		sendPacket.writeInt(nGiftId, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

	return true;
}

bool COrgMgr::onNotify(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	S32 nNotityType = Packet->readInt(Base::Bit8);
	S32 nPlayerID = Packet->readInt(Base::Bit32);
	S32 nBuyType = Packet->readInt(Base::Bit32);
	S32 nPrice = Packet->readInt(Base::Bit32);
	bool bRecharge = Packet->readFlag();
	S32 nItemId = 0;
	if (nNotityType == ACTIVITY_NOTIFY_GOLD) {
		if (nBuyType == 3)
			nItemId = 680069;
		else if (nBuyType == 4)
			nItemId = 680070;
		else if (nBuyType == 5)
			nItemId = 680071;
		else if (nBuyType == 6)
			nItemId = 680072;
		else if (nBuyType == 7)
			nItemId = 680073;
		else if (nBuyType == 8)
			nItemId = 680074;
	}

	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetPlayer(nPlayerID);
	if (nItemId > 0 && pPlayer->OrgId > 0) {
		auto pOrg = GetOrg(pPlayer->OrgId);
		if (pOrg) {
			pOrg->AddGift(nItemId, SERVER->GetPlayerManager()->GetPlayerName(nPlayerID));
		}
	}

	return true;
}

void ___CreateOrg(U32 nPlayerID, std::string OrgName, std::string QQ, std::string Memo, bool AutoAcceptJoin) {
	ORGMGR->CreateOrg(nPlayerID, OrgName, QQ, Memo, AutoAcceptJoin);
}

void ___SetOrgQq(U32 nOrgId, std::string Name) {
	auto pOrg = ORGMGR->GetOrg(nOrgId);
	if (pOrg) {
		pOrg->SetQq(Name);
	}
}

void ___SearchOrg(U32 nPlayerId) {
	ORGMGR->SearchOrg(nPlayerId);
}

void ___SearchTopOrg(U32 nPlayerId) {
	ORGMGR->SearchTopOrg(nPlayerId);
}

void ___GetOrgContribute(U32 nPlayerId) {
	auto bFind = false;
	auto nContribute = 0;
	ORGMGR->GetOrgContribute(nPlayerId, bFind, nContribute);
}

void COrgMgr::Export(struct lua_State* L)
{
	lua_tinker::def(L, "CreateOrg", &___CreateOrg);
	lua_tinker::def(L, "SetOrgQq", &___SetOrgQq);
	lua_tinker::def(L, "SearchOrg", &___SearchOrg);
	lua_tinker::def(L, "SearchTopOrg", &___SearchTopOrg);
	lua_tinker::def(L, "GetOrgContribute", &___GetOrgContribute);
}