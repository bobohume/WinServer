#include "OfficerMgr.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "Common/MemGuard.h"
#include "../WorldServer.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "../PlayerMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "CardMgr.h"
#include "BuffMgr.h"
#include "PlayerInfoMgr.h"
#include "OfficerData.h"
#include "SkillData.h"
#include "BuffData.h"
#include "BASE/tVector.h"
#include "TaskMgr.h"

const unsigned int MAX_OFFICER_SLOT = 50;

//DECLARE_REDIS_UNIT(stOfficer);
COfficerManager::COfficerManager()
{
	REGISTER_EVENT_METHOD("CW_DESIGNATE_OFFICER_REQUEST", this, &COfficerManager::HandleDesignateOfficerRequest);
	REGISTER_EVENT_METHOD("CW_REVOKE_OFFICER_REQUEST", this, &COfficerManager::HandleRevokeOfficerRequest);
	g_OfficerDataMgr->read();

	//GET_REDIS_UNIT(stOfficer).SetName("Tbl_Officer");
	//REGISTER_REDIS_UNITKEY(stOfficer, PlayerID);
	//REGISTER_REDIS_UNITKEY(stOfficer, OfficeID);
	//REGISTER_REDIS_UNIT(stOfficer, Card);
	//REGISTER_REDIS_UNIT(stOfficer, Business);
	//REGISTER_REDIS_UNIT(stOfficer, Military);
	//REGISTER_REDIS_UNIT(stOfficer, Technology);
}

COfficerManager::~COfficerManager()
{
	g_OfficerDataMgr->clear();
}

COfficerManager* COfficerManager::Instance()
{
	static COfficerManager s_Mgr;
	return &s_Mgr;
}

OfficerRef COfficerManager::CreateOfficer(U32 ID, U32 nPayerID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		COfficerData* pData = g_OfficerDataMgr->getData(ID);
		if (!pData)
			return NULL;

		OfficerRef pOfficer = OfficerRef(new stOfficer);
		pOfficer->OfficeID = ID;
		pOfficer->PlayerID = nPayerID;
		return pOfficer;
	}
	DBECATCH()

	return NULL;
}

enOpError COfficerManager::CanAddOfficer(U32 nPlayerID, U32 nID, std::string UID)
{
	PLAYER_ITR  itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		DATA_MAP& OfficerMap = itr->second;
		for (DATA_ITR iter = OfficerMap.begin(); iter != OfficerMap.end(); ++iter)
		{
			if (iter->second && (iter->second->Card == UID))
				return OPERATOR_OFFICER_CARD_EXIST;
		}
	}

	if (GetData(nPlayerID, nID))
		return OPERATOR_OFFICER_EXIST;

	COfficerData* pData = g_OfficerDataMgr->getData(nID);
	if (!pData)
		return OPERATOR_HASNO_OFFICER;

	CardRef pCard = CARDMGR->GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	U32 pVal = 0;
	(pData->m_OfficerType == 1) ? (pVal = (pCard->Agi)) : ((pData->m_OfficerType == 2) ? (pVal = (pCard->Int)) : (pVal = (pCard->Str)));
	if (pVal < pData->m_OfficerVal)
		return OPERATOR_OFFICER_CARD_NOFIT;

	return OPERATOR_NONE_ERROR;
}

enOpError COfficerManager::Designate(U32 nPlayerID, U32 nID, std::string UID)
{
	U32 nError = 0;
	if ((nError = CanAddOfficer(nPlayerID, nID, UID)) != OPERATOR_NONE_ERROR)
		return (enOpError)nError;

	return AddOfficer(nPlayerID, nID, UID);
}

enOpError COfficerManager::AddOfficer(U32 nPlayerID, U32 nID, std::string UID)
{
	CardRef pCard = CARDMGR->GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_HASNO_CARD;

	COfficerData* pData = g_OfficerDataMgr->getData(nID);
	if (!pData)
		return OPERATOR_HASNO_OFFICER;

	OfficerRef pOfficer = CreateOfficer(nID, nPlayerID);
	if (!pOfficer)
		return OPERATOR_OFFICER_DESIGNATE_FAIL;
	
	pOfficer->Card = UID;
	S32 iAddVal = 0;
	stBuff buff;
	g_Stats.Clear();
	if(pData->m_SkillID)
	{
		CSkillData* pSkillData = g_SkillDataMgr->getData(pData->m_SkillID);
		if (pSkillData)
		{
			pSkillData->getStats(g_Stats);
		}
	}

	//PLAYERINFOMGR->AddHp(nPlayerID, g_Stats.SolHp);
	//PLAYERINFOMGR->AddAttack(nPlayerID, g_Stats.SolAtk);

	pOfficer->Business	 = g_Stats.Bus;
	pOfficer->Military	 = g_Stats.Mil;
	pOfficer->Technology = g_Stats.Tec;
	CARDMGR->CaculateCardStats(nPlayerID, UID, g_Stats);
	pOfficer->Business = pOfficer->Business *(1 + g_Stats.OffBus_gPc / 100.0f);
	pOfficer->Military = pOfficer->Military *(1 + g_Stats.OffMil_gPc / 100.0f);
	pOfficer->Technology = pOfficer->Technology *(1 + g_Stats.OffTec_gPc / 100.0f);
	buff += pOfficer;

	AddData(nPlayerID, nID, pOfficer);
	InsertDB(nPlayerID, pOfficer);
	CaculateStats(nPlayerID, pOfficer, g_Stats);
	UpdateToClient(nPlayerID, pOfficer);
	PLAYERINFOMGR->AddBuff(nPlayerID, 3, buff);
	SUBMIT(nPlayerID, 35, 1);
	SUBMIT(nPlayerID, 68, nID);
	return OPERATOR_NONE_ERROR;
}

enOpError COfficerManager::RemoveOfficer(U32 nPlayerID, U32 nID)
{
	COfficerData* pData = g_OfficerDataMgr->getData(nID);
	if (!pData)
		return OPERATOR_HASNO_OFFICER;

	OfficerRef pOfficer = GetData(nPlayerID, nID);
	if (!pOfficer)
		return OPERATOR_OFFICER_RDESIGNATE_FAIL;

	g_Stats.Clear();
	if (pData->m_SkillID)
	{
		CSkillData* pSkillData = g_SkillDataMgr->getData(pData->m_SkillID);
		if (pSkillData)
		{
			pSkillData->getStats(g_Stats);
		}
	}
	//PLAYERINFOMGR->AddHp(nPlayerID, -g_Stats.SolHp);
	//PLAYERINFOMGR->AddAttack(nPlayerID, -g_Stats.SolAtk);

	stBuff buff;
	if (pOfficer)
	{
		buff -= pOfficer;
	}
	CaculateStats(nPlayerID, pOfficer, g_Stats, true);
	PLAYERINFOMGR->AddBuff(nPlayerID, 3, buff);
	DeleteData(nPlayerID, nID);
	DeleteDB(nPlayerID, nID);
	return OPERATOR_NONE_ERROR;
}

enOpError COfficerManager::OneKeyDesignate(U32 nPlayerID)
{
	std::vector<stSimCard> CardVec;
	//CARDMGR->GetCardUID(nPlayerID, CardVec);
	std::stable_sort(CardVec.begin(), CardVec.end(), [](const stSimCard& Left, const stSimCard& Right) {
		return (Left.Agi + Left.Int + Left.Str) > (Right.Agi + Right.Int + Right.Str);
	});

	for (auto i = 1; i <= 24; ++i)
	{
		COfficerData* pData = g_OfficerDataMgr->getData(i);
		if (pData)
		{
			for (auto itr = CardVec.begin(); itr != CardVec.end(); ++itr)
			{
				U32 nVal = 0;
				(pData->m_OfficerType == 1) ? (nVal = (*itr).Agi) : ((pData->m_OfficerType == 2) ? (nVal = (*itr).Int) : (nVal = (*itr).Str));
				if (pData->m_OfficerVal <= nVal)
				{
					if (CanAddOfficer(nPlayerID, i, (*itr).UID) != OPERATOR_NONE_ERROR  && AddOfficer(nPlayerID, i, (*itr).UID) == OPERATOR_NONE_ERROR)
					{
						itr = CardVec.erase(itr);
						break;
					}
				}
				else if (pData->m_OfficerVal > ((*itr).Agi + (*itr).Int + (*itr).Str))
				{
					break;
				}
			}
		}
	}

	return OPERATOR_NONE_ERROR;
}

void COfficerManager::CaculateStats(U32 nPlayerID, OfficerRef pData, Stats& stats, bool bRemove)
{
	stats.Clear();
	COfficerData* pOffData = g_OfficerDataMgr->getData(pData->OfficeID);
	if (pOffData)
	{
		CSkillData* pSkillData = g_SkillDataMgr->getData(pOffData->m_SkillID);
		if (pSkillData)
		{
			pSkillData->getStats(stats);
		}
	}
	if (bRemove)
	{
		Stats tempStast;
		tempStast -= stats;
		stats = tempStast;
	}

	BUFFMGR->AddStatus(nPlayerID, stats);
}

void COfficerManager::UpdateToClient(U32 nPlayerID, OfficerRef pData)
{
	/*AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_OFFICER_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pOfficer->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}*/
}

void COfficerManager::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		DATA_MAP& OfficerMap = m_PlayerMap[nPlayerID];
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_OFFICER_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(OfficerMap.size(), Base::Bit16);
		for (DATA_ITR itr = OfficerMap.begin(); itr != OfficerMap.end(); ++itr)
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

DB_Execution* COfficerManager::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	/*std::string str;
	{

		CRedisDB conn(SERVER->GetRedisPool());
		HashCmd<stOfficer> cmdRedis(&conn);
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
			HashCmd<stOfficer> cmdRedis(&conn);
			cmdRedis.hvals(id, VecStr);
			if (!VecStr.empty())
			{
				stBuff buff;
				for (auto itr : VecStr)
				{
					OfficerRef pOfficer = OfficerRef(new stOfficer);
					cmdRedis.GetVal(pOfficer, itr);
					AddData(id, pOfficer->OfficeID, pOfficer);
					buff += pOfficer;
					CaculateStats(id, pOfficer, g_Stats);
				}
				PLAYERINFOMGR->AddBuff(id, 3, buff, true);
			}
			return true;
		});
		return pHandle;
	}*/

	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(fmt::sprintf("SELECT TOP %d OfficerID, PlayerID, Card, Business, Technology, Military  FROM Tbl_Officer \
		WHERE PlayerID=%d", MAX_OFFICER_SLOT, nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void* pH)
	{
		DB_Execution* pHandle = (DB_Execution*)(pH);
		if (pHandle && error == NONE_ERROR)
		{
			stBuff buff;
			while (pHandle->More())
			{
				OfficerRef pOfficer = OfficerRef(new stOfficer);
				pOfficer->OfficeID = pHandle->GetInt();
				pOfficer->PlayerID = pHandle->GetInt();
				pOfficer->Card = pHandle->GetString();
				pOfficer->Business = pHandle->GetInt();
				pOfficer->Technology = pHandle->GetInt();
				pOfficer->Military = pHandle->GetInt();
				buff += pOfficer;
				
				CaculateStats(id, pOfficer, g_Stats);
				/*CRedisDB conn(SERVER->GetRedisPool());
				HashCmd<stOfficer> cmdRedis(&conn);
				cmdRedis.hset(pOfficer, id);*/
				AddData(id, pOfficer->OfficeID, pOfficer);
			}

			PLAYERINFOMGR->AddBuff(id, 3, buff, true);
		}
	}
	);
	return pHandle;
}

bool COfficerManager::UpdateDB(OfficerRef pData)
{
	//CRedisDB conn(SERVER->GetRedisPool());
	//HashCmd<stOfficer> cmdRedis(&conn);
	//cmdRedis.hset(pData, pData->PlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Officer SET Card='%s', Business=%d, Technology=%d, Military=%d\
			 WHERE OfficerID=%d AND PlayerID=%d", \
			pData->Card.c_str(), pData->Business, pData->Technology, pData->Military,\
			pData->OfficeID, pData->PlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool COfficerManager::InsertDB(U32 nPlayerID, OfficerRef pData)
{
	//CRedisDB conn(SERVER->GetRedisPool());
	//HashCmd<stOfficer> cmdRedis(&conn);
	//cmdRedis.hset(pData, nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("INSERT INTO  Tbl_Officer \
		(OfficerID,		PlayerID,		Card,			Business,		Technology,		Military) \
		VALUES(%d,		%d,				'%s',			%d,				%d,				%d)", \
		pData->OfficeID, pData->PlayerID, pData->Card.c_str(), pData->Business, pData->Technology, pData->Military));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool COfficerManager::DeleteDB(U32 nPlayerID, U32 nID)
{
	//OfficerRef pData = OfficerRef(new stOfficer);
	//pData->PlayerID = nPlayerID;
	//pData->OfficeID = nID;
	//CRedisDB conn(SERVER->GetRedisPool());
	//HashCmd<stOfficer> cmdRedis(&conn);
	//cmdRedis.hdel(pData, nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE Tbl_Officer  WHERE OfficerID=%d AND PlayerID=%d", \
		nID, nPlayerID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool COfficerManager::onPlayerEnter(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	LoadDB(pHead->Id);
	return true;
}

bool COfficerManager::onPlayerLeave(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	RemovePlayer(pHead->Id);
	return true;
}

bool COfficerManager::HandleDesignateOfficerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 ID = Packet->readInt(Base::Bit8);
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = Designate(pAccount->GetPlayerId(), ID, strUID);
		CMemGuard Buffer(512 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 512);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_DESIGNATE_OFFICER_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(ID, Base::Bit8);
		sendPacket.writeString(strUID, MAX_UUID_LENGTH);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool COfficerManager::HandleRevokeOfficerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 ID = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = RemoveOfficer(pAccount->GetPlayerId(), ID);
		CMemGuard Buffer(512 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 512);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_REVOKE_OFFICER_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(ID, Base::Bit8);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

void ___Designate(int id, int nId, std::string UID)
{
	OFFICERMGR->Designate(id, nId, UID);
}

void ___LonginOfficer(int id)
{
	OFFICERMGR->LoadDB(id);
}

void COfficerManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "Designate", ___Designate);
	lua_tinker::def(L, "LonginOfficer", ___LonginOfficer);
}
