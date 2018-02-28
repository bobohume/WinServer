#include "SkillMgr.h"
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
#include "TimeSetMgr.h"
#include "CardMgr.h"
#include "BuffMgr.h"
#include "platform.h"
#include "SkillData.h"
#include "BuffData.h"
#include "WINTCP/dtServerSocket.h"
#include "PrincessData.h"
#include "BASE/tVector.h"
#include "BASE/mMathFn.h"
#include "TaskMgr.h"

static std::vector<S32> s_TechnologyTimer = { \
TECHNOLOGY_TIMER, TECHNOLOGY_TIMER1, TECHNOLOGY_TIMER2\
};
//DECLARE_REDIS_UNIT(stSkill);
CSkillMgr::CSkillMgr()
{
	/*GET_REDIS_UNIT(stSkill).SetName("Tbl_Skill");
	REGISTER_REDIS_UNITKEY(stSkill, PlayerID);
	REGISTER_REDIS_UNITKEY(stSkill, Series);
	REGISTER_REDIS_UNIT(stSkill, Level);*/
	g_TechnologyDataMgr->read();

	REGISTER_EVENT_METHOD("CW_TECHNOLOGY_LEVELUP_REQUEST", this, &CSkillMgr::HandleTechnologyLevelUpRequest);
}

CSkillMgr::~CSkillMgr()
{
	g_TechnologyDataMgr->close();
}

CSkillMgr* CSkillMgr::Instance()
{
	static CSkillMgr s_Mgr;
	return &s_Mgr;
}

auto GetSkillID = [](auto pData){
	return MACRO_SKILL_ID(pData->Series, pData->Level);
};

SkillRef CSkillMgr::CreateData(U32 nPlayerID, S32 nID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		SkillRef pData = SkillRef(new stSkill);
		pData->PlayerID = nPlayerID;
		pData->Series = nID;
		pData->Level = 0;
		return pData;
	}
	DBECATCH()

	return NULL;
}

bool CSkillMgr::AddData(U32 nPlayerID, S32 nID)
{
	if (GetData(nPlayerID, nID))
		return false;

	SkillRef pData = CreateData(nPlayerID, nID);
	if (!pData)
		return false;

	Parent::AddData(nPlayerID, nID, pData);
	InsertDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);
	return true;
}

void CSkillMgr::CaculateStats(U32 nPlayerID, S32 nID)
{
	g_Stats.Clear();
	CTechnologyData* pData = g_TechnologyDataMgr->getData(nID);
	if (pData)
	{
		CBuffData* pBuffData = g_BuffDataMgr->getData(pData->m_Buff);
		if (pBuffData)
		{
			pBuffData->getStats(g_Stats);
		}
		BUFFMGR->AddStatus(nPlayerID, g_Stats);
	}
}

enOpError CSkillMgr::LevelUp(U32 nPlayerID, std::string UID, S32 nID)
{
	CardRef pCard = CARDMGR->GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	SkillRef pData = GetData(nPlayerID, nID);
	if (!pData)
		return OPERATOR_HASHNO_TECHNOLOGY;

	S32 nPreTechnologyID = GetSkillID(pData);
	S32 nTechnologyID = GetSkillID(pData) + 1;
	S32 nCostTime = 0, nCostMoney = 0;
	CTechnologyData* pTechData = g_TechnologyDataMgr->getData(nTechnologyID);
	if (!pTechData)
		return OPERATOR_HASHNO_TECHNOLOGYID;

	Stats buff;
	CARDMGR->CaculateCardStats(nPlayerID, UID, buff);
	StatsRef pStat = BUFFMGR->GetData(nPlayerID);
	nCostMoney = pTechData->m_Ore  * ((pStat) ? (1.0f + ((pStat->TecCost_gPc + buff.TecCost_gPc) / 100.0f)) : 1.0f);
	nCostTime = pTechData->m_Time * 60 * ((pStat) ? (1.0f + ((pStat->TecTime_gPc + buff.TecTime_gPc) / 100.0f)) : 1.0f);
	if (pCard->Int < pTechData->m_Int)
		return OPERATOR_HASHNO_INT;

	if (!PLAYERINFOMGR->CanAddValue(nPlayerID, Ore, -nCostMoney))
		return OPERATOR_HASHNO_ORE;


	S32 nTimerId = 0, nTimes = 0;
	StatsRef pStats = BUFFMGR->GetData(nPlayerID);
	S32 nMaxSlot = 1 + (pStats ? pStats->TechNum : 0);
	for (auto itr = s_TechnologyTimer.begin(); itr != s_TechnologyTimer.end(); ++itr, nTimes++)
	{
		if(nTimes >= nMaxSlot)//上限
			break;

		TimeSetRef pTimer = TIMESETMGR->GetData(nPlayerID, *itr);
		if (!pTimer)
		{
			nTimerId = *itr;
		}
		else
		{
			if (pTimer->Flag1 == nID || pTimer->Flag == UID)
			{
				nTimerId = 0;
				break;
			}
		}
	}

	if (nTimerId == 0)
		return OPERATOR_TECHNOLOGY_COOLDOWN;

	SUBMIT(nPlayerID, 29, 1);
	SUBMIT(nPlayerID, 34, 1);
	TIMESETMGR->AddTimeSet(nPlayerID, nTimerId, nCostTime, UID, pData->Series);
	PLAYERINFOMGR->AddOre(nPlayerID, -nCostMoney);
	return OPERATOR_NONE_ERROR;
}

void CSkillMgr::DoLevelUp(U32 nPlayerID, S32 nID)
{
	SkillRef pData = GetData(nPlayerID, nID);
	if (!pData)
		return;

	S32 nPreTechnologyID = GetSkillID(pData);
	S32 nTechnologyID = GetSkillID(pData) + 1;
	S32 nCostTime = 0, nCostMoney = 0;
	CTechnologyData* pTechData = g_TechnologyDataMgr->getData(nTechnologyID);
	if (!pTechData)
		return;

	if (pTechData->m_NextID)
	{
		AddData(nPlayerID, pTechData->m_NextID);
	}

	pData->Level += 1;
	UpdateDB(nPlayerID, pData);
	UpdateToClient(nPlayerID, pData);

	//计算属性
	{
		Stats preStats;
		Stats aftStats;
		CTechnologyData* preTechData = g_TechnologyDataMgr->getData(nPreTechnologyID);
		if (preTechData)
		{
			preTechData->getStats(preStats);
		}

		{
			pTechData->getStats(aftStats);
		}

		aftStats -= preStats;
		BUFFMGR->AddStatus(nPlayerID, aftStats);
		PLAYERINFOMGR->UpdateToClient(nPlayerID);

		if (aftStats.Impose_gPc)
			PLAYERINFOMGR->AddImposeTimes(nPlayerID, aftStats.Impose_gPc);
		else if (aftStats.Agi_gPc || aftStats.Str_gPc || aftStats.Int_gPc) {
			CARDMGR->CaculateStats(nPlayerID);
		}
	}

	if (nID == 403003) {
		SUBMIT(nPlayerID, 61, MACRO_SKILL_LEVEL(nTechnologyID));
	}
	else if (nID == 403002) {
		SUBMIT(nPlayerID, 62, MACRO_SKILL_LEVEL(nTechnologyID));
	}
	else if (nID == 402003) {
		SUBMIT(nPlayerID, 65, MACRO_SKILL_LEVEL(nTechnologyID));
	}
	else if (nID == 403005) {
		SUBMIT(nPlayerID, 67, MACRO_SKILL_LEVEL(nTechnologyID));
	}
}

void CSkillMgr::UpdateToClient(U32 nPlayerID, SkillRef pData)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_TECHNOLOGYNOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CSkillMgr::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		DATA_MAP& DataMap = m_PlayerMap[nPlayerID];
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_TECHNOLOGY_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(DataMap.size(), Base::Bit16);
		for (DATA_ITR itr = DataMap.begin(); itr != DataMap.end(); ++itr)
		{
			if (sendPacket.writeFlag(itr->second != NULL))
			{
				itr->second->WriteData(&sendPacket);
			}
		}

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

DB_Execution* CSkillMgr::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	/*std::string str;
	{
		CRedisDB conn(SERVER->GetRedisPool());
		HashCmd<stSkill> cmdRedis(&conn);
		str = cmdRedis.hlen(nPlayerID);
	}
	if (str != "0")
	{
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetId(nPlayerID);
		pHandle->SetSql("");
		pHandle->RegisterFunction([this](int id, int error, void * pH)
		{
			std::vector<std::string> VecStr;
			CRedisDB conn(SERVER->GetRedisPool());
			HashCmd<stSkill> cmdRedis(&conn);
			cmdRedis.hvals(id, VecStr);
			if (!VecStr.empty())
			{
				for (auto itr : VecStr)
				{
					SkillRef pData = SkillRef(new stSkill);
					cmdRedis.GetVal(pData, itr);
					Parent::AddData(id, pData->Series, pData);
					CaculateStats(id, GetSkillID(pData));
				}
			}
			return true;
		});
		return pHandle;
	}*/

	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(fmt::sprintf("SELECT PlayerID, Series, Level FROM Tbl_Skill WHERE PlayerID=%d", nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void* pSqlHandle)
	{
		DB_Execution* pHandle = (DB_Execution*)(pSqlHandle);
		if (pHandle && error == NONE_ERROR)
		{
			while (pHandle->More())
			{
				SkillRef pData = SkillRef(new stSkill);
				pData->PlayerID = pHandle->GetInt();
				pData->Series = pHandle->GetInt();
				pData->Level  = pHandle->GetInt();
				//CRedisDB conn(SERVER->GetRedisPool());
				//HashCmd<stSkill> cmdRedis(&conn);
				//cmdRedis.hset(pData.get(), id);
				Parent::AddData(id, pData->Series, pData);
				CaculateStats(id, GetSkillID(pData));
			}

			if (!GetData(id, 401001))
				AddData(id, 401001);
			if (!GetData(id, 401002))
				AddData(id, 401002);
			if (!GetData(id, 401003))
				AddData(id, 401003);
			if (!GetData(id, 402001))
				AddData(id, 402001);
			if (!GetData(id, 402002))
				AddData(id, 402002);
			if (!GetData(id, 402003))
				AddData(id, 402003);
			if (!GetData(id, 403001))
				AddData(id, 403001);
			if (!GetData(id, 403002))
				AddData(id, 403002);
			if (!GetData(id, 403003))
				AddData(id, 403003);
		}
	}
	);
	return pHandle;
}

bool CSkillMgr::UpdateDB(U32 nPlayerID, SkillRef pData)
{
	/*CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stSkill> cmdRedis(&conn);
	cmdRedis.hset(pData.get(), nPlayerID);*/
	DB_Execution* pHandle = new DB_Execution();
	//pHandle->SetSql(cmdRedis.UpdateSql(pData));
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Skill SET Level=%d WHERE PlayerID=%d AND Series=%d", \
		pData->Level, pData->PlayerID, pData->Series));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CSkillMgr::InsertDB(U32 nPlayerID, SkillRef pData)
{
	/*CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stSkill> cmdRedis(&conn);
	cmdRedis.hset(pData.get(), nPlayerID);*/
	DB_Execution* pHandle = new DB_Execution();
	//pHandle->SetSql(cmdRedis.InsertSql(pData));
	pHandle->SetSql(fmt::sprintf("INSERT INTO Tbl_Skill\
	(PlayerID,					Series,						Level)\
	VALUES(%d,					%d,							%d)",\
	pData->PlayerID,			pData->Series,				pData->Level));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CSkillMgr::DeleteDB(U32 nPlayerID, S32 ID)
{
	/*CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stSkill> cmdRedis(&conn);
	SkillRef pData = SkillRef(new stSkill);
	pData->PlayerID = nPlayerID;
	pData->Series = ID;
	cmdRedis.hdel(pData, nPlayerID);*/
	DB_Execution* pHandle = new DB_Execution();
	//pHandle->SetSql(cmdRedis.DeleteSql(pData));
	pHandle->SetSql(fmt::sprintf("DELETE FROM Tbl_Skill WHERE PlayerID=%d AND Series=%d", nPlayerID, ID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CSkillMgr::HandleTechnologyLevelUpRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 ID = Packet->readInt(Base::Bit32);
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = LevelUp(pAccount->GetPlayerId(), UID, ID);
		CMemGuard Buffer(512 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 512);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_TECHNOLOGY_LEVELUP_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeString(UID, MAX_UUID_LENGTH);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}


void CSkillMgr::Export(struct lua_State* L)
{

}