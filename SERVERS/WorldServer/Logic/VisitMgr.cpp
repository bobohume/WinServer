#include "VisitlMgr.h"
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
#include "VisitData.h"
#include "platform.h"
#include "PrincessMgr.h"
#include "ItemMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "BASE/tVector.h"
#include "BASE/mMathFn.h"
#include "TaskMgr.h"
#include "PrincessData.h"
#include "NoviceGuide.h"
#include "VIPMgr.h"

const U32 VISIT_ITEM_ID = 600093;
//DECLARE_REDIS_UNIT(stCity);
CVisitMgr::CVisitMgr()
{
	REGISTER_EVENT_METHOD("CW_ROLLDICE_REQUEST",		this, &CVisitMgr::HandleRollDiceRequest);
	REGISTER_EVENT_METHOD("CW_ROLLFIXEDDICE_REQUEST",	this, &CVisitMgr::HandleRollFixedDiceRequest);
	REGISTER_EVENT_METHOD("CW_DEVELOPCITY_REQUEST",		this, &CVisitMgr::HandleDevelopCityRequest);

	/*GET_REDIS_UNIT(stCity).SetName("Tbl_City");
	REGISTER_REDIS_UNITKEY(stCity, PlayerID);
	REGISTER_REDIS_UNITKEY(stCity, CityID);
	REGISTER_REDIS_UNIT(stCity, Level);
	REGISTER_REDIS_UNIT(stCity, Support);
	REGISTER_REDIS_UNIT(stCity, DevelopFlag);*/
	g_VisitDataMgr->read();
}

CVisitMgr::~CVisitMgr()
{
	g_VisitDataMgr->close();
}

CVisitMgr* CVisitMgr::Instance()
{
	static CVisitMgr s_Mgr;
	return &s_Mgr;
}

auto GetCityID = [](auto nPlayerID)
{
	TimeSetRef pTimer = TIMESETMGR->GetData(nPlayerID, CITY_TIMER1);
	if (pTimer)
		return pTimer->Flag1;

	return 8001;
};

auto GetCityRef = [](auto nPlayerID, auto nCityID, auto& nLevel, auto& nSupport, auto& nDevelopFlag)
{
	CityRef pCity = VISITMGR->GetData(nPlayerID, nCityID);
	if (pCity)
	{
		nLevel = pCity->Level;
		nSupport = pCity->Support;
		nDevelopFlag = pCity->DevelopFlag;
	}
	else
	{
		nLevel = 1; 
		nSupport = 0;
		nDevelopFlag = 0;
	}
};

CityRef CVisitMgr::CreateData(U32 nPlayerID, S32 nID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		CityRef pData = CityRef(new stCity);
		pData->PlayerID = nPlayerID;
		pData->CityID = nID;
		pData->Support = 0;
		pData->Level = 1;
		return pData;
	}
	DBECATCH()

	return NULL;
}

bool CVisitMgr::AddData(U32 nPlayerID, S32 nCityID, S32 nLevel, S32 nSupport, S32 nDevelopFlag)
{
	CityRef pData = GetData(nPlayerID, nCityID);
	if(!pData)
	{
		CityRef pData = CreateData(nPlayerID, nCityID);
		if (!pData)
			return false;

		pData->Level = nLevel;
		pData->Support = nSupport;
		pData->DevelopFlag = nDevelopFlag;
		Parent::AddData(nPlayerID, nCityID, pData);
		InsertDB(nPlayerID, pData);
		UpdateToClient(nPlayerID, pData);
	}
	else
	{
		pData->Level = nLevel;
		pData->Support = nSupport;
		pData->DevelopFlag = nDevelopFlag;
		UpdateDB(nPlayerID, pData);
		UpdateToClient(nPlayerID, pData);
	}
	return true;
}

enOpError CVisitMgr::rollDice(U32 nPlayerID, std::pair<S32, S32>& DropInfo)
{
	S32 nRandDice = 4;
	if(!NGUIDE->IsFirstVisitAndHold(nPlayerID))
	{
		nRandDice = gRandGen.randI(1, 6);
	}
	return VisitCity(nPlayerID, nRandDice, DropInfo);
}

enOpError CVisitMgr::VisitCity(U32 nPlayerID, S32 nVal, std::pair<S32, S32>& DropInfo, bool bCostItem/*=false*/)
{
	if (nVal <= 0 || nVal > 6)
		return OPERATOR_PARAMETER_ERROR;

	TimeSetRef pTimer = TIMESETMGR->GetData(nPlayerID, CITY_TIMER2);
	if (bCostItem)
	{
		if(!ITEMMGR->CanReduceItem(nPlayerID, VISIT_ITEM_ID, -1))
			return OPERATOR_HASNO_ITEM;
	}
	else
	{
		if (!pTimer || pTimer->Flag1 <= 0)
			return OPERATOR_HASHNO_STRENGTH;
	}

	S32 nLevel = 1, nSupport = 0, nDevelopFlag = 0;
	S32 nCityID = GetCityID(nPlayerID);
	//重置开发状态
	GetCityRef(nPlayerID, nCityID, nLevel, nSupport, nDevelopFlag);
	AddData(nPlayerID, nCityID, nLevel, nSupport, 0);

	nCityID += nVal;
	if (nCityID > 8021) 
	{
		nCityID++;
	}

	nCityID = 8000 + nCityID % 21 + 1;
	nCityID = (nCityID == 8021) ? (8001) : nCityID;
	GetCityRef(nPlayerID, nCityID, nLevel, nSupport, nDevelopFlag);

	CVisitData* pVisitData = g_VisitDataMgr->getData(nCityID, nLevel);
	if (!pVisitData)
		return OPERATOR_PARAMETER_ERROR;

	//遇到妃子
	S32 nRanVal = 1;
	if (!NGUIDE->IsFirstVisit(nPlayerID))
	{
		nRanVal = gRandGen.randI(1, 100);
	}
	
	if (pVisitData->m_PrincessId)
	{
		if (nRanVal < (S32)pVisitData->m_RandVal)
		{
			++nSupport;
			S32 nVal = 100;
			CPrincessData* pPrincessData = g_PrincessDataMgr->getData(pVisitData->m_PrincessId);
			if (pPrincessData)
				nVal = pPrincessData->m_HireVal;

			SUBMIT(nPlayerID, 40, 1);
			if (!PRINCESSMGR->AddSupport(nPlayerID, pVisitData->m_PrincessId) && nSupport >= nVal && 3 != pPrincessData->m_HireType)
			{
				PRINCESSMGR->AddData(nPlayerID, pVisitData->m_PrincessId, nSupport);
				SUBMIT(nPlayerID, 42, 1);
			}
		}
	}

	AddData(nPlayerID, nCityID, nLevel, nSupport, 0);

	if (!bCostItem && pTimer)
	{
		S32 ExpireTime = -1;
		if (pTimer->Flag1 == 5 + VIPMGR->GetVipStrengthTopLimit(nPlayerID)) {
			ExpireTime = VISITCITYCD * (1.f + VIPMGR->GetVipVSAReducePercent(nPlayerID) / 100.f);
		}
		TIMESETMGR->AddTimeSet(nPlayerID, CITY_TIMER2, ExpireTime, pTimer->Flag, pTimer->Flag1-1);
	}

	TIMESETMGR->AddTimeSet(nPlayerID, CITY_TIMER1, 0, "", nCityID);

	//产出
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	F32 nPerVal = (pVisitData->m_MakeType == 1) ? ((pPlayer ? pPlayer->Business : 0.0f)) : ((pVisitData->m_MakeType == 2) ? ((pPlayer ? pPlayer->Technology : 0.0f)) : ((pPlayer ? pPlayer->Military : 0.0f)));
	S32 nAddVal = pVisitData->m_MakeVal * (nPerVal / 1000000.0f + 1.0f);
	if (pVisitData->m_MakeType == 1)
		PLAYERINFOMGR->AddMoney(nPlayerID, nAddVal);
	else if (pVisitData->m_MakeType == 2)
		PLAYERINFOMGR->AddOre(nPlayerID, nAddVal);
	else
		PLAYERINFOMGR->AddTroops(nPlayerID, nAddVal);

	if (bCostItem)
		ITEMMGR->AddItem(nPlayerID, VISIT_ITEM_ID, -1);
	DropInfo = ITEMMGR->DropItem(nPlayerID, pVisitData->m_DropId);
	SUBMIT(nPlayerID, 4, 1);
	SetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 3, 1);
	SUBMIT(nPlayerID, 51, 1);
	SUBMITACTIVITY(nPlayerID, ACTIVITY_VISITYCITY, 1);
	return OPERATOR_NONE_ERROR;
}

enOpError CVisitMgr::DevelopCity(U32 nPlayerID, S32 nType)
{
	if (nType < 0 || nType > 1)
		return OPERATOR_PARAMETER_ERROR;

	S32 nCityID = GetCityID(nPlayerID);
	CityRef pCity = GetData(nPlayerID, nCityID);
	if (!pCity || pCity->DevelopFlag)
		return OPERATOR_CITY_DEVELOPED;

	if (pCity->Level >= 9)
		return OPERATOR_CITY_MAX_LEVEL;

	CVisitData* pVisitData = g_VisitDataMgr->getData(nCityID, pCity->Level+1);
	if (!pVisitData)
		return OPERATOR_PARAMETER_ERROR;

	if (nType == 0)
	{
		if (!PLAYERINFOMGR->CanAddValue(nPlayerID, Money, -pVisitData->m_Money * 10000))
			return OPERATOR_HASNO_MONEY;
	}
	else
	{
		if (!PLAYERINFOMGR->CanAddValue(nPlayerID, Ore, -pVisitData->m_Ore * 10000))
			return OPERATOR_HASHNO_ORE;
	}


	if (nType == 0)
		PLAYERINFOMGR->AddMoney(nPlayerID, -pVisitData->m_Money * 10000);
	else
		PLAYERINFOMGR->AddOre(nPlayerID, -pVisitData->m_Ore * 10000);
	AddData(nPlayerID, nCityID, pCity->Level+1, pCity->Support, 1);
	SUBMIT(nPlayerID, 39, pCity->Level);
	return OPERATOR_NONE_ERROR;
}

void CVisitMgr::UpdateToClient(U32 nPlayerID, CityRef pData)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_CITY_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CVisitMgr::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		DATA_MAP& DataMap = m_PlayerMap[nPlayerID];
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_CITY_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
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

DB_Execution* CVisitMgr::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	/*std::string str;
	{
		CRedisDB conn(SERVER->GetRedisPool());
		HashCmd<stCity> cmdRedis(&conn);
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
			HashCmd<stCity> cmdRedis(&conn);
			cmdRedis.hvals(id, VecStr);
			if (!VecStr.empty())
			{
				for (auto itr : VecStr)
				{
					CityRef pData = CityRef(new stCity);
					cmdRedis.GetVal(pData, itr);
					Parent::AddData(id, pData->CityID, pData);
				}
			}
			return true;
		});
		return pHandle;
	}*/

	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(fmt::sprintf("SELECT PlayerID, CityID, Level, Support, DevelopFlag FROM Tbl_City WHERE PlayerID=%d", nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void* pSqlHandle)
	{
		DB_Execution* pHandle = (DB_Execution*)(pSqlHandle);
		if (pHandle && error == NONE_ERROR)
		{
			while (pHandle->More())
			{
				CityRef pData = CityRef(new stCity);
				pData->PlayerID = pHandle->GetInt();
				pData->CityID = pHandle->GetInt();
				pData->Level  = pHandle->GetInt();
				pData->Support = pHandle->GetInt();
				pData->DevelopFlag = pHandle->GetInt();
				/*CRedisDB conn(SERVER->GetRedisPool());
				HashCmd<stCity> cmdRedis(&conn);
				cmdRedis.hset(pData, id);*/
				Parent::AddData(id, pData->CityID, pData);
			}
		}
	}
	);
	return pHandle;
}

bool CVisitMgr::UpdateDB(U32 nPlayerID, CityRef pData)
{
	/*CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stCity> cmdRedis(&conn);
	cmdRedis.hset(pData, nPlayerID);*/
	DB_Execution* pHandle = new DB_Execution();
	//pHandle->SetSql(cmdRedis.UpdateSql(pData));
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_City SET \
	Level=%d,					Support=%d,						DevelopFlag=%d\
	WHERE PlayerID=%d AND		CityID=%d",\
	pData->Level,				pData->Support,					pData->DevelopFlag,\
	pData->PlayerID,			pData->CityID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CVisitMgr::InsertDB(U32 nPlayerID, CityRef pData)
{
	/*CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stCity> cmdRedis(&conn);
	cmdRedis.hset(pData, nPlayerID);*/
	DB_Execution* pHandle = new DB_Execution();
	//pHandle->SetSql(cmdRedis.InsertSql(pData));
	pHandle->SetSql(fmt::sprintf("INSERT INTO Tbl_City \
	(Level,						Support,						DevelopFlag,					PlayerID,					CityID)\
	VALUES(%d,					%d,								%d,								%d,							%d)",\
	pData->Level,				pData->Support,					pData->DevelopFlag,				pData->PlayerID,			pData->CityID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CVisitMgr::DeleteDB(U32 nPlayerID, S32 ID)
{
	/*CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stCity> cmdRedis(&conn);
	CityRef pData = CityRef(new stCity);
	pData->PlayerID = nPlayerID;
	pData->CityID = ID;
	cmdRedis.hdel(pData, nPlayerID);*/
	DB_Execution* pHandle = new DB_Execution();
	//pHandle->SetSql(cmdRedis.DeleteSql(pData));
	pHandle->SetSql(fmt::sprintf("DELETE FROM Tbl_City WHERE PlayerID=%d  AND CityID=%d", nPlayerID, ID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CVisitMgr::HandleRollDiceRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		DROPITEM_PAIR DropInfo;
		U32 nError = rollDice(pAccount->GetPlayerId(), DropInfo);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ROLLDICE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(DropInfo.first, Base::Bit32);
		sendPacket.writeInt(DropInfo.second, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CVisitMgr::HandleRollFixedDiceRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	S32 nVal = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		DROPITEM_PAIR DropInfo;
		U32 nError = VisitCity(pAccount->GetPlayerId(), nVal, DropInfo, true);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ROLLFIXEDDICE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(DropInfo.first, Base::Bit32);
		sendPacket.writeInt(DropInfo.second, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CVisitMgr::HandleDevelopCityRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	S32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = DevelopCity(pAccount->GetPlayerId(), nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_DEVELOPCITY_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

void CVisitMgr::Export(struct lua_State* L)
{

}