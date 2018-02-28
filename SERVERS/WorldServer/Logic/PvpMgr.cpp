#include "PvpMgr.h"
#include "DBLayer/Common/Define.h"
#include "../WorldServer.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "../Script/lua_tinker.h"
#include "Common/MemGuard.h"
#include "Common/mRandom.h"
#include "CommLib/redis.h"
#include "../PlayerMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "CardMgr.h"
#include "PlayerInfoMgr.h"
#include "PvpData.h"
#include "SkillData.h"
#include "BuffData.h"
#include "TimeSetMgr.h"
#include "ItemMgr.h"
#include "TopMgr.h"
#include "DBLayer/Common/DBUtility.h"
#include "CommLib/SimpleTimer.h"
#include "TaskMgr.h"
#include "platform.h"
#include "ItemData.h"

#define  MIN_PVP_LEVEL 50
#define  PVP_BATTLE_ITEM_ID 600069
#define  PVP_AVENGE_ITEM_ID 600070
//DECLARE_REDIS_UNIT(stPveInfo);
SimpleTimer<24*60*60> g_ClearTimer;//清楚仇人列表
//---------------------------------------------------------------------------------------------------------------------
//	 ---------------------------------------------------------------------------------------------------------------
//														PVE
//   ---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
CPveManager::CPveManager()
{
	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CPveManager::Update, 1000);
	REGISTER_EVENT_METHOD("CW_OPEN_PVE_REQUEST",   this, &CPveManager::HandleOpenPveRequest);
	REGISTER_EVENT_METHOD("CW_PVE_READY_REQUEST",  this, &CPveManager::HandlePveReadyRequest);
	REGISTER_EVENT_METHOD("CW_PVE_BEGIN_REQUEST",  this, &CPveManager::HandlePveBeginRequest);
	REGISTER_EVENT_METHOD("CW_PVE_BATTLE_REQUEST", this, &CPveManager::HandlePveBattleRequest);
	REGISTER_EVENT_METHOD("CW_PVE_AVENGE_REQUEST", this, &CPveManager::HandlePveAvengeRequest);
	REGISTER_EVENT_METHOD("CW_PVE_ADDBUFF_REQUEST", this, &CPveManager::HandlePveAddBuffRequest);
	REGISTER_EVENT_METHOD("CW_PVE_END_REQUEST",		this, &CPveManager::HandlePveEndRequest);
	REGISTER_EVENT_METHOD("CW_PVE_SETJUMPBATTLE_REQUEST", this, &CPveManager::HandlePveSetJumpBattle);
	g_ClearTimer.Start();
	g_PvpDataMgr->read();
	g_PvpBuffDataMgr->read();

	/*GET_REDIS_UNIT(stPveInfo).SetName("Tbl_Pve");
	REGISTER_REDIS_UNITKEY(stPveInfo, PlayerID);
	REGISTER_REDIS_UNITKEY(stPveInfo, TargetID);
	REGISTER_REDIS_UNITKEY(stPveInfo, Type);
	REGISTER_REDIS_UNITKEY(stPveInfo, Unqiue);
	REGISTER_REDIS_UNIT(stPveInfo, CardNum);
	REGISTER_REDIS_UNIT(stPveInfo, Score);
	REGISTER_REDIS_UNIT(stPveInfo, AddScore);
	REGISTER_REDIS_UNIT(stPveInfo, BattleVal);
	REGISTER_REDIS_UNIT(stPveInfo, Time);*/
}

CPveManager::~CPveManager()
{
	g_PvpDataMgr->close();
	g_PvpBuffDataMgr->close();
}

CPveManager* CPveManager::Instance()
{
	static CPveManager s_Mgr;
	return &s_Mgr;
}

auto GetDamageVal = [](S32 nAction, S32 nAction1)
{
	if ((nAction == 0 && nAction1 == 1) || (nAction == 1 && nAction1 == 2) || (nAction == 2 && nAction1 == 0))
		return 2;
	else if ((nAction == 1 && nAction1 == 0) || (nAction == 2 && nAction1 == 1) || (nAction == 0 && nAction1 == 2))
		return 1;

	return 0;
};

auto CaculateDamage = [](S32 nAction, S32 nDam, auto pData)
{
	S32 nBaseVal = (nAction == 0) ? pData->AgiAtk : ((nAction == 1) ? (pData->IntAtk) : ((nAction == 2) ? (pData->StrAtk) : pData->SpeAtk));
	if (nAction == 3 && pData->Peerless != 100)
	{
		nBaseVal = pData->AgiAtk;
	}

	F32 fReduceVal = (nDam == 2) ? 2.0f : ((nDam == 1) ? (0.5f) : 1.0f);
	S32 nPeerless = (nDam == 2) ? 35 : ((nDam == 1) ? (15) : 25);
	pData->Atk = nBaseVal * gRandGen.randF(0.90f, 1.10f) * fReduceVal;

	if (nAction != 3)
		pData->Peerless = mClamp(pData->Peerless + nPeerless, 0, 100);
	else
		pData->Peerless = 0;
};

auto BattleEnd = [](auto& PlayerDeq, auto pData)
{
	if (pData->Hp <= 0)
	{
		PlayerDeq.pop_back();
	}

	return PlayerDeq.empty();
};

auto ResetTable = [](auto pTable)
{
	memset(pTable->nAction, -1, sizeof(pTable->nAction));
	memset(pTable->nBuff, 0, sizeof(pTable->nBuff));
	memset(pTable->nBuffId, 0, sizeof(pTable->nBuffId));
	pTable->nTime = _time32(NULL) + 23;
};

auto AddScore = [](auto nPlayerID, S32 nScore, auto bWin)
{
	S32 nValue = 1000;
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetPlayer(nPlayerID);
	if (pPlayer)
		nValue = pPlayer->PveScore;

	if (bWin)
		nValue = mClamp(nValue + nScore, 0, S32_MAX);
	else
		nValue = mClamp(nValue - nScore, 0, S32_MAX);

	TOPMGR->NewInData(eTopType_PVE, nPlayerID, nValue);
	if (bWin)
		PLAYERINFOMGR->AddPveScore(nPlayerID, nScore);
	else
		PLAYERINFOMGR->AddPveScore(nPlayerID, -nScore);
};

auto GetScrore = [](auto nPlayerID)
{
	S32 nValue = 1000;
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetPlayer(nPlayerID);
	if (pPlayer)
		nValue = pPlayer->PveScore;

	return nValue;
};

auto GetBattleVal = [](auto nPlayerID)
{
	S32 nValue = 100;
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetPlayer(nPlayerID);
	if (pPlayer)
		nValue = pPlayer->BattleVal;

	return nValue;
};

auto GetPveData = [](auto nPlayerID, auto& PveDeq, int level, auto bCdFlag)
{
	//机器人
	CCardManager::DATA_MAP CardMap;
	if (nPlayerID < 10000)
	{
		std::vector<CPvpData*> Vec = g_PvpDataMgr->GetPvpData(nPlayerID);
		for (auto itr : Vec)
		{
			CPvpData* pPvpData = itr;
			CardRef pCard = CARDMGR->CreateCard(nPlayerID, pPvpData->m_CardID);
			pCard->Agi = pPvpData->m_Agi;
			pCard->Int = pPvpData->m_Int;
			pCard->Str = pPvpData->m_Str;
			pCard->Level = 50;
			pCard->BattleVal = mClamp((S32)(pCard->Agi + pCard->Int + pCard->Str), (S32)0, (S32)S32_MAX);
			CardMap.insert(CCardManager::DATA_MAP::value_type(pCard->UID, pCard));
		}
	}
	else
	{
		CardMap = CARDMGR->GetDataMap(nPlayerID);
		if (CardMap.empty())
		{
			CardMap = CARDMGR->SyncLoadDB(nPlayerID);
		}

		//竞技场CD
		if (!CardMap.empty() && bCdFlag)
		{
			for (S32 i = PVP_CARD_TIMER1; i <= PVP_CARD_TIMER80; ++i)
			{
				TimeSetRef pTimer = TIMESETMGR->GetData(nPlayerID, i);
				if (pTimer)
				{
					if (CardMap.find(pTimer->Flag) != CardMap.end())
					{
						CardMap.erase(pTimer->Flag);
					}
				}
			}
		}
	}

	for (auto itr = CardMap.begin(); itr != CardMap.end(); ++itr)
	{
		auto pCard = itr->second;
		if (pCard && ((int)(pCard->Level) >= level))
		{
			PvpInfoRef pData = PvpInfoRef(new stPvpInfo);
			*pData.get() = pCard;
			g_Stats.Clear();
			for (auto j = 0; j < MAX_SKILL_SLOT; ++j)
			{
				if (pCard->Skill[j] > 0)
				{
					CSkillData* pSkillData = g_SkillDataMgr->getData(pCard->Skill[j]);
					if (pSkillData)
					{
						pSkillData->getStats(g_Stats);
					}
				}
			}

			pData->Hp = mCeil(pData->Hp * (1.0 + (g_Stats.PvpHP_gPc / 100.0f)));
			pData->AgiAtk = mCeil(pData->AgiAtk * (1.0 + (g_Stats.PvpAtk_gPc / 100.0f)));
			pData->IntAtk = mCeil(pData->IntAtk * (1.0 + (g_Stats.PvpAtk_gPc / 100.0f)));
			pData->StrAtk = mCeil(pData->StrAtk * (1.0 + (g_Stats.PvpAtk_gPc / 100.0f)));
			pData->SpeAtk = mCeil(pData->SpeAtk * (1.0 + (g_Stats.PvpAtk_gPc / 100.0f)));
			PveDeq.push_back(pData);
		}
	}

};

auto GetCardSize = [](auto nPlayerID)
{
	CCardManager::DATA_MAP& CardMap = CARDMGR->GetDataMap(nPlayerID);
	return CardMap.size();
};

auto Make64 = [](S32 nPlayerID, S32 nPlayerID1)
{
	S64 nVal = (((S64)nPlayerID) << 32) | nPlayerID1;
	return nVal;
};

auto GetPlayerID = [](S64 nPlayerID)
{
	S32 nVal = nPlayerID & 0xFFFFFFFF;
	return nVal;
};

auto GetPlayerName = [](S64 nID)
{
	S32 nPlayerID = GetPlayerID(nID);
	if (nPlayerID < 10000)
	{
		CPvpData* pData = g_PvpDataMgr->getData(nPlayerID);
		if (pData)
		{
			return (const char*)pData->m_sName.c_str();
		}
	}
	else
	{
		PlayerSimpleDataRef pPlayer = SERVER->GetPlayerManager()->GetPlayerData(nPlayerID);
		if (pPlayer)
		{
			return (const char*)pPlayer->PlayerName;
		}
	}
	return "";
};

void CPveManager::AddPveInfo(S32 nPlayerID, S8 nType, S32 nTargetID, S32 nCardNum, S32 nScore, S32 nAddScore, S32 nBattleVal, S32 Unquie/*=0*/)
{
	PveInfoRef pData = GetData(nPlayerID, nType, nTargetID, Unquie);
	if (pData)
	{
		pData->CardNum = nCardNum;
		pData->Score = nScore;
		pData->AddScore = nAddScore;
		pData->BattleVal = nBattleVal;
		pData->Time = _time32(NULL);
		pData->Unqiue = Unquie;
		UpdateDB(nPlayerID, pData);
		UpdateToClient(nPlayerID, pData);
	}
	else
	{
		pData = PveInfoRef(new stPveInfo);
		pData->PlayerID = nPlayerID;
		pData->TargetID = nTargetID;
		pData->TargetName = GetPlayerName(nTargetID);
		pData->Type = nType;
		pData->CardNum = nCardNum;
		pData->Score = nScore;
		pData->AddScore = nAddScore;
		pData->BattleVal = nBattleVal;
		pData->Time = _time32(NULL);
		pData->Unqiue = Unquie;
		AddData(nPlayerID, nType, pData);
		InsertDB(nPlayerID, pData);
		UpdateToClient(nPlayerID, pData);
	}
}

void CPveManager::AddData(S32 nPlayerID, S8 nType, PveInfoRef pData)
{
	if (pData)
	{
		m_PlayerMap[nType][nPlayerID].insert(DATA_MAP::value_type(std::make_pair(pData->TargetID, pData->Unqiue), pData));
	}
}

void CPveManager::DeleteData(S32 nPlayerID, S8 nType, S32 nTargetID, S32 Unquie/*=0*/)
{
	PLAYER_ITR itr = m_PlayerMap[nType].find(nPlayerID);
	if (itr != m_PlayerMap[nType].end())
	{
		DATA_MAP& DataMap = itr->second;
		DATA_ITR iter = DataMap.find(std::make_pair(nTargetID, Unquie));
		if (iter != DataMap.end())
		{
			DataMap.erase(iter);
		}
	}
}

PveInfoRef CPveManager::GetData(S32 nPlayerID, S8 nType, S32 nTargetID, S32 Unqiue)
{
	PLAYER_ITR itr = m_PlayerMap[nType].find(nPlayerID);
	if (itr != m_PlayerMap[nType].end())
	{
		DATA_MAP& DataMap = itr->second;
		DATA_ITR iter = DataMap.find(std::make_pair(nTargetID, Unqiue));
		if (iter != DataMap.end())
		{
			return iter->second;
		}
	}

	return NULL;
}

void CPveManager::RemovePlayer(S32 nPlayerID)
{
	for (auto i = 0; i < 2; ++i)
	{
		PLAYER_ITR  itr = m_PlayerMap[i].find(nPlayerID);
		if (itr != m_PlayerMap[i].end())
		{
			DATA_MAP& PlayerMap = itr->second;
			PlayerMap.clear();
			m_PlayerMap[i].erase(itr);
		}
	}
}

void CPveManager::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(2048*10 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 2048 * 10);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_PVEQUE_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);

		sendPacket.writeInt(m_PlayerMap[0][nPlayerID].size() + m_PlayerMap[1][nPlayerID].size(), Base::Bit16);
		for (int i = 0; i < 2; ++i)
		{
			DATA_MAP& DataMap = m_PlayerMap[i][nPlayerID];
			for (DATA_ITR itr = DataMap.begin(); itr != DataMap.end(); ++itr)
			{
				if (sendPacket.writeFlag(itr->second != NULL))
				{
					itr->second->WriteData(&sendPacket);
				}
			}
		}

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CPveManager::UpdateToClient(U32 nPlayerID, PveInfoRef pData)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_PVEQUE_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CPveManager::UpdateDelToClient(U32 nPlayerID, S32 nTargetID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 64);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_DELETE_PVEQUE_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nTargetID, Base::Bit32);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

DB_Execution* CPveManager::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	/*std::string str;
	{
		CRedisDB conn(SERVER->GetRedisPool());
		HashCmd<stPveInfo> cmdRedis(&conn);
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
			HashCmd<stPveInfo> cmdRedis(&conn);
			cmdRedis.hvals(id, VecStr);
			if (!VecStr.empty())
			{
				for (auto itr : VecStr)
				{
					PveInfoRef pData = PveInfoRef(new stPveInfo);
					cmdRedis.GetVal(pData, itr);
					AddData(id, pData->Type, pData);
				}
			}
			return true;
		});
		return pHandle;
	}*/

	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(fmt::sprintf("SELECT TOP %d PlayerID, TargetID, Type, Unqiue, CardNum, Score, AddScore, BattleVal, Time FROM Tbl_Pve \
		WHERE PlayerID=%d", 50, nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void* pEx)
	{
		DB_Execution* pHandle = (DB_Execution*)(pEx);
		if (pHandle && error == NONE_ERROR)
		{
			stBuff buff;
			while (pHandle->More())
			{
				PveInfoRef pData = PveInfoRef(new stPveInfo);
				pData->PlayerID = pHandle->GetInt();
				pData->TargetID = pHandle->GetInt();
				pData->Type = pHandle->GetInt();
				pData->Unqiue = pHandle->GetInt();
				pData->CardNum = pHandle->GetInt();
				pData->Score = pHandle->GetInt();
				pData->AddScore = pHandle->GetInt();
				pData->BattleVal = pHandle->GetInt();
				pData->Time = pHandle->GetTime();
				pData->TargetName = GetPlayerName(pData->TargetID);
				//CRedisDB conn(SERVER->GetRedisPool());
				//HashCmd<stPveInfo> cmdRedis(&conn);
				//cmdRedis.hset(pData,id);
				AddData(id, pData->Type, pData);
			}
		}
	}
	);
	return pHandle;
}

bool CPveManager::UpdateDB(U32 nPlayerID, PveInfoRef pData)
{
	char szLoginTime[32] = "";
	GetDBTimeString(pData->Time, szLoginTime);
	/*CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stPveInfo> cmdRedis(&conn);
	cmdRedis.hset(pData, nPlayerID);*/
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Pve \
			SET CardNum=%d,			Score=%d,				AddScore=%d,		BattleVal=%d,		Time='%s' \
			WHERE PlayerID=%d AND TargetID=%d AND Type=%d AND Unqiue=%d",\
			pData->CardNum,			pData->Score,			pData->AddScore,	pData->BattleVal,	szLoginTime, \
			pData->PlayerID, pData->TargetID, pData->Type,  pData->Unqiue));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPveManager::InsertDB(U32 nPlayerID, PveInfoRef pData)
{
	char szLoginTime[32] = "";
	GetDBTimeString(pData->Time, szLoginTime);
	/*CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stPveInfo> cmdRedis(&conn);
	cmdRedis.hset(pData, nPlayerID);*/
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("INSERT INTO Tbl_Pve \
			  (PlayerID,			TargetID,				Type,				Unqiue,			CardNum,					Score,	\
			  AddScore,				BattleVal,				Time) \
		VALUES(%d,					%d,						%d,					%d,				%d,							%d,		\
			 %d,						%d,					'%s')", \
			 pData->PlayerID,		pData->TargetID,		pData->Type,	pData->Unqiue,	pData->CardNum,				pData->Score,  \
			 pData->AddScore,		pData->BattleVal,		szLoginTime));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPveManager::DeleteDB(U32 nPlayerID, S8 nType, S32 nTargetID)
{
	/*PveInfoRef pData = PveInfoRef(new stPveInfo);
	pData->PlayerID = nPlayerID;
	pData->TargetID = nTargetID;
	pData->Type = nType;
	CRedisDB conn(SERVER->GetRedisPool());
	HashCmd<stPveInfo> cmdRedis(&conn);
	cmdRedis.hdel(pData, nPlayerID);*/
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE  Tbl_Pve \
		WHERE PlayerID=%d AND TargetID=%d AND Type=%d AND Unqiue=%d",\
		nPlayerID, nTargetID, nType, 0));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPveManager::ClearDB()
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE FROM Tbl_Pve WHERE DATEDIFF(DAY, Time, GETDATE()) > 7"));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CPveManager::CanJoinPve(S64 nPlayerID)
{
	CCardManager::DATA_MAP& DataMap = CARDMGR->GetDataMap(nPlayerID);
	return false;
}

bool CPveManager::CanOpenPve(S64 nPlayerID)
{
	CCardManager::DATA_MAP& DataMap = CARDMGR->GetDataMap(nPlayerID);
	if (DataMap.size() >= 12)
		return true;

	return false;
}

DATA_VEC& CPveManager::GetBattle(S64 nPlayerID, S8 nType)
{
	BATTLE_MAP::iterator itr = m_BattleMap[nType].find(nPlayerID);
	if (itr != m_BattleMap[nType].end())
	{
		return itr->second;
	}

	DATA_VEC Deq;
	return Deq;
}

S8 CPveManager::GetBattleType(S32 nPlayerID)
{
	for (int i = 0; i < 3; ++i)
	{
		TABLE_ITR itr = m_TableMap[i].find(nPlayerID);
		if (itr != m_TableMap[i].end())
		{
			if (itr->second && (itr->second->nGameStatus == PVP_GAME_PLAY || itr->second->nGameStatus == PVP_GAME_BEGIN))
			{
				return i;
			}
		}
	}

	return -1;
}

S8 CPveManager::GetRemainingMaxPVPTimes(U32 nPlayerID)
{
	DATA_VEC PveDeq;
	GetPveData(nPlayerID, PveDeq, MIN_PVP_LEVEL, false);
	return (S8)PveDeq.size();
}

enOpError CPveManager::AddPVPTimes(U32 nPlayerID, U16 times)
{
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PVP_BATTLE_TIMES);
	if (!pTime) {
		TIMESETMGR->AddTimeSet(nPlayerID, PVP_BATTLE_TIMES, Platform::getNextTime(Platform::INTERVAL_DAY) - _time32(NULL), "", 4 + times);
	}
	else {
		TIMESETMGR->AddTimeSet(nPlayerID, PVP_BATTLE_TIMES, Platform::getNextTime(Platform::INTERVAL_DAY) - _time32(NULL), "", pTime->Flag1 + times);
	}
	return OPERATOR_NONE_ERROR;
}

enOpError CPveManager::Battle(S64 nPlayerID, S32 nAction, S8 nType)
{
	if (nAction < 0 || nAction > 3)
		return OPERATOR_PARAMETER_ERROR;

	TABLE_ITR itr = m_TableMap[nType].find(nPlayerID);
	if (itr == m_TableMap[nType].end())
		return OPERATOR_PVE_NO_PLAYER;

	PvpTableRef pTable = itr->second;
	if (!pTable || pTable->nGameStatus != PVP_GAME_PLAY)
		return OPERATOR_PVE_ERROR;

	if (nAction == 3)
	{
		DATA_VEC& PlayerDeq = m_BattleMap[nType][nPlayerID];
		if (PlayerDeq.empty())
			return OPERATOR_PARAMETER_ERROR;

		PvpInfoRef pData = PlayerDeq.back();
		if(pData->Peerless < 100)
			return OPERATOR_PARAMETER_ERROR;
	}

	for (int i = 0; i < GAME_PLAYER; ++i)
	{
		if ((pTable->nPlayerID[i] == nPlayerID) && (pTable->nAction[i] == -1))
		{
			pTable->nAction[i] = nAction;
		}
	}

	if ((nPlayerID == pTable->nPlayerID[0]) && m_AIMap[nType][pTable->nPlayerID[1]])
	{
		m_AIMap[nType][pTable->nPlayerID[1]]->Update();
	}

	if (CanUpdate(pTable))
	{
		Battle(pTable, nType);
	}

	return OPERATOR_NONE_ERROR;
}

enOpError CPveManager::Battle(PvpTableRef pTable, S8 nType)
{
	S32 nAction		= pTable->nAction[0];
	S32 nAction1	= pTable->nAction[1];
	S64 nPlayerID	= pTable->nPlayerID[0];
	S64 nPlayerID1	= pTable->nPlayerID[1];
	//0:Agi,1:Int,2:Str,3:Spe
	nAction = mClamp(nAction, 0, 3);
	nAction1 = mClamp(nAction1, 0, 3);
	S32 nActionArray[2] = { nAction, nAction1 };
	DATA_VEC& PlayerDeq = m_BattleMap[nType][nPlayerID];
	DATA_VEC& PlayerDeq1 = m_BattleMap[nType][nPlayerID1];
	S32 nCurSize[2] = { PlayerDeq.size(), PlayerDeq1.size() };
	if (!PlayerDeq.empty() && !PlayerDeq1.empty())
	{
		PvpInfoRef pData  = PlayerDeq.back();
		PvpInfoRef pData1 = PlayerDeq1.back();
		S32 nDamageVal  = GetDamageVal(nAction, nAction1);
		S32 nDamageVal1 = GetDamageVal(nAction1, nAction);
		
		CaculateDamage(nAction,  nDamageVal,  pData);
		CaculateDamage(nAction1, nDamageVal1, pData1);

		pData->Hp -= pData1->Atk;
		pData1->Hp -= pData->Atk;
		ResetTable(pTable);

		PvpInfoRef pVpData[2] = { pData, pData1 };
		if(!JumpBattle(pTable))
		UpdatePveToClient(GetPlayerID(nPlayerID), pVpData, nActionArray);

		if (BattleEnd(PlayerDeq, pData) || BattleEnd(PlayerDeq1, pData1))
		{
			S32 nAtkScore = GetScrore(GetPlayerID(nPlayerID));
			S32 nWiner = GetPlayerID(PlayerDeq.empty() ? nPlayerID1 : nPlayerID);
			S32 nLoster = GetPlayerID(PlayerDeq1.empty() ? nPlayerID1 : nPlayerID);
			if (nWiner == nPlayerID) {
				++pTable->nCSize[1];
			}
			BattleCleaning(nWiner, nLoster, nPlayerID, pTable, 2, pData->UID);
			m_BattleMap[nType].erase(nPlayerID);
			m_BattleMap[nType].erase(nPlayerID1);
			m_AIMap[nType].erase(nPlayerID);
			m_AIMap[nType].erase(nPlayerID1);
			pTable->nGameStatus = PVP_GAME_END;

			if (nType == 1)
			{
				DeleteData(GetPlayerID(nPlayerID), 1, GetPlayerID(nPlayerID1));
				DeleteDB(GetPlayerID(nPlayerID), 1, GetPlayerID(nPlayerID1));
				UpdateDelToClient(GetPlayerID(nPlayerID), GetPlayerID(nPlayerID1));
			}

			PVPCheckAndFlushBattleCD(nPlayerID);
			PVPOnce(nPlayerID);
		}
		else
		{
			bool bLos[2] = { PlayerDeq.size() != nCurSize[0], PlayerDeq1.size() != nCurSize[1] };
			if (bLos[0] || bLos[1])
			{
				if (bLos[0])
					++pTable->nCSize[0];
				
				if (bLos[1])
					++pTable->nCSize[1];

				S32 nWiner = GetPlayerID(bLos[0] ? nPlayerID1 : nPlayerID);
				S32 nLoster = GetPlayerID(bLos[1] ? nPlayerID1 : nPlayerID);

				pTable->nGameStatus = PVP_GAME_BEGIN;
				BattleCleaning(nWiner, nLoster, nPlayerID, pTable, 0, pData->UID);
			}
		}
	}

	return OPERATOR_NONE_ERROR;
}

enOpError CPveManager::BattleCleaning(S32 nWiner, S32 nLoser, S64 nPlayerID, PvpTableRef pTable, S8 bEnd, std::string UID)
{
	S32 nScore = 2;
	S32 nDefenseScore = 1; //防守变动1分
	S32 nAddScore = 0;
	S32 nAddAch = 100;
	S32 nDefeat = pTable->nCSize[1];
	S32 nCurPlayerID = GetPlayerID(nPlayerID);
	S32 nTargetPlayerID = GetPlayerID(pTable->nPlayerID[1]);
	if (nWiner == nLoser)
	{
		AddScore(nWiner, nScore, false);
	}
	else if(nWiner == nCurPlayerID)
	{
		AddScore(nWiner, nScore, true);
		AddScore(nLoser, nDefenseScore, false);
	}
	else if (nLoser == nCurPlayerID)
	{
		AddScore(nLoser, nScore, false);
		AddScore(nWiner, nDefenseScore, true);
	}

	if (nCurPlayerID == nWiner)
	{
		if (nDefeat >= 20)
			TOPMGR->NewInData(eTopType_PVEWIN, nCurPlayerID, nDefeat, nTargetPlayerID);

		CARDMGR->AddCardAchievement(nCurPlayerID, UID, nAddAch);
		if (nDefeat == 3)
			ITEMMGR->AddItem(nCurPlayerID, 300005, 1);

		PLAYERINFOMGR->AddPveMoney(nPlayerID, 1);
		nAddScore = nScore;
	}
	else
	{
		nAddScore = -nScore;
		pTable->nWinNum[0] = 0;
	}

	{
		S64 nPlayerID1 = pTable->nPlayerID[1];
		S32 nAtkScore = GetScrore(GetPlayerID(nPlayerID));
		S32 nAtker = GetPlayerID(nPlayerID);
		S32 nDefer = GetPlayerID(nPlayerID1);
		if (nAtker > 10000 && nDefer > 10000)
		{
			pTable->nScore += (nAddScore / 2) * -1;

			if (nDefeat > 6 && nWiner == nCurPlayerID)
			{
				AddPveInfo(nDefer, 1, nAtker, pTable->nCSize[1], GetScrore(nAtker), pTable->nScore, GetBattleVal(nAtker));
			}
			else
			{
				AddPveInfo(nDefer, 0, nAtker, pTable->nCSize[1], GetScrore(nAtker), pTable->nScore, GetBattleVal(nAtker), pTable->nPvpBattleTime);
			}
		}
	}

	//三胜宝箱
	if (nCurPlayerID == nWiner)
	{
		++pTable->nWinNum[0];
		if (pTable->nWinNum[0] >= 3)
		{
			CDropData* pDropData = ITEMMGR->DropItem(300005);
			if (pDropData)
			{
				AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
				if (pAccount)
				{
					if (ITEMMGR->AddItem(nPlayerID, pDropData->m_ItemId, pDropData->m_ItemNum))
					{
						CMemGuard Buffer(256 MEM_GUARD_PARAM);
						Base::BitStream sendPacket(Buffer.get(), 256);
						stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PVP_WIN3_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
						sendPacket.writeInt(pDropData->m_ItemId, Base::Bit32);
						sendPacket.writeInt(pDropData->m_ItemNum, Base::Bit32);
						pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
						SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
					}
				}
			}
			pTable->nWinNum[0] = 0;
		}
	}

	SendPveEndToClient(GetPlayerID(nPlayerID), nAddScore, nDefeat, nAddAch, bEnd);

	return OPERATOR_NONE_ERROR;
}

enOpError CPveManager::AddBuff(S64 nPlayerID, S32 nBuffID, S8 nType, PvpInfoRef pBattleData[2])
{
	if (nType < 0 || nType > 3)
		return OPERATOR_PARAMETER_ERROR;

	CPvpBuffData* pBuffData = g_PvpBuffDataMgr->getData(nBuffID);
	if (!pBuffData)
		return OPERATOR_PARAMETER_ERROR;

	TABLE_ITR itr = m_TableMap[nType].find(nPlayerID);
	if (itr == m_TableMap[nType].end())
		return OPERATOR_PVE_NO_PLAYER;

	PvpTableRef pTable = itr->second;
	if (!pTable || pTable->nGameStatus != PVP_GAME_BEGIN)
		return OPERATOR_PVE_ERROR;

	BATTLE_MAP::iterator iter = m_BattleMap[nType].find(nPlayerID);
	if (iter == m_BattleMap[nType].end())
		return OPERATOR_PARAMETER_ERROR;

	S64 nPlayerID1 = pTable->nPlayerID[1];
	BATTLE_MAP::iterator iter1 = m_BattleMap[nType].find(nPlayerID1);
	if (iter1 == m_BattleMap[nType].end())
		return OPERATOR_PARAMETER_ERROR;

	DATA_VEC& PlayerDeq = iter->second;
	if (PlayerDeq.empty())
		return OPERATOR_PARAMETER_ERROR;

	DATA_VEC& PlayerDeq1 = iter1->second;
	if (PlayerDeq1.empty())
		return OPERATOR_PARAMETER_ERROR;

	S32 nBuffType = pBuffData->m_BuffType - 1;
	if (pTable->nBuff[0][nBuffType] != 0)
	{
		return OPERATOR_PVE_BUFF_ERROR;
	}

	PvpInfoRef pData = PlayerDeq.back();
	if (!pData)
		return OPERATOR_PARAMETER_ERROR;

	PvpInfoRef pData1 = PlayerDeq1.back();
	if (!pData1)
		return OPERATOR_PARAMETER_ERROR;

	if ((pBuffData->m_CostType == 1) && !PLAYERINFOMGR->CanAddValue(nPlayerID, PveMoney, -pBuffData->m_CostNum))
		return OPERATOR_PVE_ADDBUFF_ERROR;
	else if ((pBuffData->m_CostType == 2) && !SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -pBuffData->m_CostNum))
		return OPERATOR_HASNO_GOLD;
		
	g_Stats.Clear();
	CBuffData* pBData = g_BuffDataMgr->getData(pBuffData->m_BuffID);
	if (pBData)
	{
		pBData->getStats(g_Stats);
		pData->AddAgi += g_Stats.PvpAgi_gPc;
		pData->AddInt += g_Stats.PvpInt_gPc;
		pData->AddStr += g_Stats.PvpStr_gPc;
		pData->AddHp  = g_Stats.PvpHp_gPc;

		pData->Agi = mCeil(pData->BaseAgi * (1.0f + pData->AddAgi / 100.0f));
		pData->Int = mCeil(pData->BaseInt * (1.0f + pData->AddInt / 100.0f));
		pData->Str = mCeil(pData->BaseStr * (1.0f + pData->AddStr / 100.0f));
		pData->Hp  = mCeil(pData->Hp + pData->MaxHp * (pData->AddHp / 100.0f));
		pData->AgiAtk = pData->Agi;
		pData->IntAtk = pData->Int;
		pData->StrAtk = pData->Str;
		pData->SpeAtk = (pData->Agi + pData->Int + pData->Str);
		pData->Hp = mClamp(pData->Hp, 0, pData->MaxHp);

		pData1->Agi = mCeil(pData1->Agi * (1.0f + g_Stats.PvpStats_gPc / 100.0f));
		pData1->Int = mCeil(pData1->Int * (1.0f + g_Stats.PvpStats_gPc / 100.0f));
		pData1->Str = mCeil(pData1->Str * (1.0f + g_Stats.PvpStats_gPc / 100.0f));
		pData1->Hp = (pData1->Agi + pData1->Int + pData1->Str) * 5;

		pData1->AgiAtk = pData1->Agi;
		pData1->IntAtk = pData1->Int;
		pData1->StrAtk = pData1->Str;
		pData1->SpeAtk = (pData1->Agi + pData1->Int + pData1->Str);

		pData1->BattleVal = mCeil(pData1->BattleVal * (1.0f + g_Stats.PvpStats_gPc / 100.0f));
	}

	if (pBuffData->m_CostType == 1)
		PLAYERINFOMGR->AddPveMoney(nPlayerID, -pBuffData->m_CostNum);
	else if (pBuffData->m_CostType == 2)
		SERVER->GetPlayerManager()->AddGold(nPlayerID, -pBuffData->m_CostNum, LOG_GOLD_PVP);

	pBattleData[0] = pData;
	pBattleData[1] = pData1;
	pTable->nBuff[0][nBuffType] = 1;
	return OPERATOR_NONE_ERROR;
}

bool CPveManager::PVPBattleCD(U32 nPlayerID)
{
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PVP_CD_TIME);
	if (!pTime || pTime->ExpireTime <= _time32(NULL)) {
		return true;
	}
	return false;
}

bool CPveManager::PVPCheckAndFlushBattleCD(U32 nPlayerID)
{
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PVP_CD_TIME);
	if (!pTime || pTime->ExpireTime <= _time32(NULL)) {
		TIMESETMGR->AddTimeSet(nPlayerID, PVP_CD_TIME, 1 * 60 * 60, "", 0);
		return true;
	}
	return false;
}

U32 CPveManager::PVPTimes(U32 nPlayerID)
{
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PVP_BATTLE_TIMES);
	if (!pTime)
		return 4;
	return pTime->Flag1 > 0 ? pTime->Flag1 : 0;
}

bool CPveManager::PVPOnce(U32 nPlayerID)
{
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, PVP_BATTLE_TIMES);
	U32 Times = 0;
	if (!pTime)
		Times = 4 - 1;
	else {
		if (0 == pTime->Flag1) {
			return false;
		}
		else {
			Times = pTime->Flag1 - 1;
		}
	}
	TIMESETMGR->AddTimeSet(nPlayerID, PVP_BATTLE_TIMES, Platform::getNextTime(Platform::INTERVAL_DAY) - _time32(NULL), "", Times);
	return true;
}

void CPveManager::RandDEQData(DATA_VEC &Deq)
{
	for (int i = 0; i < mClamp((S32)Deq.size(), 0, 100); ++i)
	{
		S32 nRandVal = gRandGen.randI(0, Deq.size() - 1);
		PvpInfoRef tmpInfo = Deq[i];
		Deq[i] = Deq[nRandVal];
		Deq[nRandVal] = tmpInfo;
	}
}

bool CPveManager::JumpBattle(const PvpTableRef pTable)
{
	return ((pTable->nPvpType != 3 && pTable->bJumpBattle[0]) || (pTable->nPvpType == 3 && pTable->bJumpBattle[0] && pTable->bJumpBattle[1]));
}

enOpError CPveManager::JoinBattle(S64 nPlayerID, S8 nType)
{
	BATTLE_MAP::iterator itr = m_BattleMap[nType].find(nPlayerID);
	if (itr != m_BattleMap[nType].end())
		return OPERATOR_PVE_BATTLE;

	DATA_VEC PveDeq;
	GetPveData(nPlayerID, PveDeq, MIN_PVP_LEVEL, true);
	if (PveDeq.empty())
		return OPERATOR_PVE_NO_CARD;

	//找一个没有用的timerhandler作为大臣使用记录
	S32 nTimerID = 0;
	for (S32 i = PVP_CARD_TIMER1; i <= PVP_CARD_TIMER80; ++i)
	{
		TimeSetRef pTimer = TIMESETMGR->GetData(nPlayerID, i);
		if (!pTimer)
		{
			nTimerID = i;
			break;
		}
	}

	if (nTimerID == 0)
		return OPERATOR_PVE_NO_CARD;

	S32 nRandVal = gRandGen.randI(0, PveDeq.size()-1);
	PvpInfoRef pInfo = PveDeq[nRandVal];
	DATA_VEC PveData;
	PveData.push_back(pInfo);
	m_BattleMap[nType][nPlayerID] = PveData;
	TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, Platform::getNextTime(Platform::INTERVAL_DAY) - _time32(NULL), pInfo->UID, 0);
	return OPERATOR_NONE_ERROR;
}

enOpError CPveManager::JoinBattle(U32 nPlayerID, S8 nType, U32 nPlayerID1)
{
	S64 nID = Make64(nPlayerID1, nPlayerID);
	BATTLE_MAP::iterator itr = m_BattleMap[nType].find(nID);
	if (itr != m_BattleMap[nType].end())
		return OPERATOR_PVE_BATTLE;

	DATA_VEC PveDeq;
	GetPveData(nPlayerID, PveDeq, 0, false);
	if (PveDeq.empty())
		return OPERATOR_PVE_ERROR;

	RandDEQData(PveDeq);

	m_BattleMap[nType][nID] = PveDeq;
	return OPERATOR_NONE_ERROR;
}

bool CPveManager::CanUpdate(PvpTableRef pTable)
{
	bool bUpdate = (pTable->nAction[0] != -1) && (pTable->nAction[1] != -1);
	return bUpdate;
}

enOpError CPveManager::MatchBattle(S64 nPlayerID, S8 nType)
{
	if (m_BattleMap[nType].find(nPlayerID) == m_BattleMap[nType].end())
		return OPERATOR_PVE_NO_CARD;

	if (m_TableMap[nType].find(nPlayerID) != m_TableMap[nType].end())
		return OPERATOR_PVE_BATTLE;

	if (!PVPBattleCD(nPlayerID))
		return OPERATOR_PVP_COOLDOWN;

	if (PVPTimes(nPlayerID) <= 0)
		return OPERATOR_PVE_MAXTIMES;

	S32 nMatchVal = 1000;
	S64 nPlayerID1 = 0;
	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (!pPlayer)
		return OPERATOR_PVE_NO_PLAYER;

	nMatchVal = pPlayer->PveScore;
	std::ostringstream oStr;
	oStr << nPlayerID;
	std::vector<std::string> UIDVec = TOPMGR->GetRank(eTopType_PVE, oStr.str(), nMatchVal);
	if (!UIDVec.empty())
	{
		S32 nRandVal = gRandGen.randI(0, UIDVec.size()-1);
		nPlayerID1 = atoi(UIDVec[nRandVal].c_str());
		enOpError nError = JoinBattle(nPlayerID1, nType, nPlayerID);
		if (nError != OPERATOR_NONE_ERROR)
			return nError;

		nPlayerID1 = Make64(nPlayerID, nPlayerID1);
	}

	PvpTableRef pTable = PvpTableRef(new stPvpTable);
	pTable->nPlayerID[0] = nPlayerID;
	pTable->nPlayerID[1] = nPlayerID1;
	pTable->bTrustee[0] = false;
	pTable->bTrustee[1] = true;
	pTable->nSize[0] = m_BattleMap[nType][nPlayerID].size();
	pTable->nSize[1] = m_BattleMap[nType][nPlayerID1].size();
	pTable->nCSize[0] = 0;
	pTable->nCSize[1] = 0;
	pTable->nPvpType = nType;
	pTable->nTime = _time32(NULL);
	pTable->nPvpBattleTime = _time32(NULL);
	pTable->nGameStatus = PVP_GAME_READY;
	m_TableMap[nType][nPlayerID] = pTable;
	m_TableMap[nType][nPlayerID1] = pTable;
	m_AIMap[nType][nPlayerID] = PveAIRef(new PveAI(nPlayerID, nType));
	m_AIMap[nType][nPlayerID1] = PveAIRef(new PveAI(nPlayerID1, nType));

	return OPERATOR_NONE_ERROR;
}

enOpError CPveManager::AvengeBattle(S32 nPlayerID, S8 nType, S32 nTargetID, std::string UID)
{
	if (nPlayerID == nTargetID)
		return OPERATOR_PVE_ERROR;

	if (nType == 1)
	{
		if (m_PlayerMap[nType][nPlayerID].find(std::make_pair(nTargetID, 0)) == m_PlayerMap[nType][nPlayerID].end())
			return OPERATOR_PVE_ERROR;
	}

	if (m_TableMap[nType].find(nPlayerID) != m_TableMap[nType].end())
		return OPERATOR_PVE_BATTLE;

	CardRef pCard = CARDMGR->GetData(nPlayerID, UID);
	if (!pCard)
		return OPERATOR_PARAMETER_ERROR;

	if (!ITEMMGR->CanReduceItem(nPlayerID, PVP_AVENGE_ITEM_ID, -1))
		return OPERATOR_HASNO_AVENGE_ITEM;

	PvpInfoRef pInfo = PvpInfoRef(new stPvpInfo);
	(*pInfo.get()) = pCard;
	DATA_VEC PveData;
	PveData.push_back(pInfo);
	m_BattleMap[nType][nPlayerID] = PveData;

	S64 nPlayerID1 = nTargetID;
	enOpError nError = JoinBattle(nPlayerID1, nType, nPlayerID);
	if (nError != OPERATOR_NONE_ERROR)
		return nError;

	nPlayerID1 = Make64(nPlayerID, nPlayerID1);

	PvpTableRef pTable = PvpTableRef(new stPvpTable);
	pTable->nPlayerID[0] = nPlayerID;
	pTable->nPlayerID[1] = nPlayerID1;
	pTable->bTrustee[0] = false;
	pTable->bTrustee[1] = true;
	pTable->nSize[0] = m_BattleMap[nType][nPlayerID].size();
	pTable->nSize[1] = m_BattleMap[nType][nPlayerID1].size();
	pTable->nPvpType = nType;
	pTable->nTime = _time32(NULL);
	pTable->nPvpBattleTime = _time32(NULL);
	pTable->nGameStatus = PVP_GAME_READY;
	m_TableMap[nType][nPlayerID] = pTable;
	m_TableMap[nType][nPlayerID1] = pTable;
	m_AIMap[nType][nPlayerID] = PveAIRef(new PveAI(nPlayerID, nType));
	m_AIMap[nType][nPlayerID1] = PveAIRef(new PveAI(nPlayerID1, nType));
	ITEMMGR->AddItem(nPlayerID, PVP_AVENGE_ITEM_ID, -1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPveManager::BattleReady(S64 nPlayerID, S8 nType)
{
	TABLE_ITR itr = m_TableMap[nType].find(nPlayerID);
	if (itr == m_TableMap[nType].end())
		return OPERATOR_PVE_NO_CARD;

	PvpTableRef pTable = itr->second;
	if (!pTable)
		return OPERATOR_PVE_NO_CARD;

	if (pTable->nGameStatus != PVP_GAME_READY)
		return OPERATOR_PVE_ERROR;

	/*{
		BATTLE_MAP::iterator iter = m_BattleMap[nType].find(nPlayerID);
		if (iter != m_BattleMap[nType].end())
		{
			DATA_VEC& PlayerDeq = iter->second;
			if (!PlayerDeq.empty())
			{
				PvpInfoRef pData = PlayerDeq.back();
				if (pData)
				{
					CardRef	pCard = CARDMGR->GetData(nPlayerID, pData->UID);
					if (pCard)
					{
						*pData.get() = pCard;
					}
				}
			}
		}
	}*/


	pTable->nGameStatus = PVP_GAME_BEGIN;
	SendPveToClient(nPlayerID, nType, "WC_OPEN_PVE_RESPONSE", 0);
	return OPERATOR_NONE_ERROR;
}

enOpError CPveManager::BattleBegin(S64 nPlayerID, S8 nType)
{
	TABLE_ITR itr = m_TableMap[nType].find(nPlayerID);
	if (itr == m_TableMap[nType].end())
		return OPERATOR_PVE_NO_CARD;

	PvpTableRef pTable = itr->second;
	if (!pTable)
		return OPERATOR_PVE_NO_CARD;

	if (pTable->nGameStatus != PVP_GAME_BEGIN)
		return OPERATOR_PVE_ERROR;

	pTable->nGameStatus = PVP_GAME_PLAY;
	pTable->nTime = _time32(NULL) + 15;
 	for (int i = 0; i < GAME_PLAYER; ++i){
		pTable->bJumpBattle[i] = false;
 	}
	
	SUBMIT(nPlayerID, 11, 1);
	SUBMITACTIVITY(nPlayerID, ACTIVITY_PVP, 1);
	return OPERATOR_NONE_ERROR;
}

enOpError CPveManager::BattleEndness(S64 nPlayerID, S8 nType)
{
	TABLE_ITR itr = m_TableMap[nType].find(nPlayerID);
	if (itr == m_TableMap[nType].end())
		return OPERATOR_PVE_NO_PLAYER;

	PvpTableRef pTable = itr->second;
	if (!pTable)
		return OPERATOR_PVE_NO_PLAYER;

	S64 nPlayerID1 = pTable->nPlayerID[1];
	m_BattleMap[nType].erase(nPlayerID);
	m_BattleMap[nType].erase(nPlayerID1);
	m_AIMap[nType].erase(nPlayerID);
	m_AIMap[nType].erase(nPlayerID1);

	pTable->nGameStatus = PVP_GAME_END;
	pTable->nTime = _time32(NULL) + 15;

	BattleCleaning(nPlayerID1, nPlayerID, nPlayerID, pTable, 1, "");

	PVPCheckAndFlushBattleCD(nPlayerID);
	PVPOnce(nPlayerID);

	return OPERATOR_NONE_ERROR;
}

void CPveManager::Update(U32 Time)
{
	S32 nTime = _time32(NULL);
	for (int i = 0; i < 3; ++i)
	{
		for (TABLE_ITR itr = m_TableMap[i].begin(); itr != m_TableMap[i].end();)
		{
			PvpTableRef pData = itr->second;
			if (!pData)
			{
				++itr;
				continue;
			}

			if (pData->nGameStatus == PVP_GAME_PLAY)
			{
				//双方都要求跳过战斗过程
				if(JumpBattle(pData)){
					while (pData->nGameStatus == PVP_GAME_PLAY) {
						if (CanUpdate(pData))
						{
							Battle(pData, i);
						}
						else
						{
							for (S32 j = 0; j < GAME_PLAYER; ++j)
							{
								if ((pData->nAction[j] == -1) && m_AIMap[i][pData->nPlayerID[j]])
									m_AIMap[i][pData->nPlayerID[j]]->Update();
							}
						}
					}
				}
				else {
					BattleUpdate(pData, i);
				}
			}

			if (pData->nGameStatus == PVP_GAME_END)
			{
				S32 nType = pData->nPvpType;
				S64 nPlayerID  = pData->nPlayerID[1];
				S64 nPlayerID1 = pData->nPlayerID[1];
				m_BattleMap[nType].erase(nPlayerID);
				m_BattleMap[nType].erase(nPlayerID1);
				m_AIMap[nType].erase(nPlayerID);
				m_AIMap[nType].erase(nPlayerID1);
				itr = m_TableMap[i].erase(itr);
				continue;
			}
			else 
				++itr;
		}
	}

	if (g_ClearTimer.CheckTimer())
	{
		S32 nTime = _time32(NULL);
		for (int i = 0; i < 2; ++i)
		{
			for (PLAYER_ITR itr = m_PlayerMap[i].begin(); itr != m_PlayerMap[i].end();)
			{
				DATA_MAP& DataMap = itr->second;
				for (DATA_ITR iter = DataMap.begin(); iter != DataMap.end();)
				{
					PveInfoRef pData = iter->second;
					if (abs(pData->Time - nTime) > 7 * 24 * 60 * 60)
					{
						//CRedisDB conn(SERVER->GetRedisPool());
						//HashCmd<stPveInfo> cmdRedis(&conn);
						//cmdRedis.hdel(pData, pData->PlayerID);
						iter = DataMap.erase(iter);
					}
					else
					{
						++iter;
					}
				}
				if (DataMap.empty())
				{
					itr = m_PlayerMap[i].erase(itr);
				}
				else
				{
					++itr;
				}
			}
		}
		ClearDB();
	}
}

void CPveManager::BattleUpdate(PvpTableRef pData, S8 Type)
{
	S32 nTime = _time32(NULL);

	if (CanUpdate(pData))
	{
		Battle(pData, Type);
	}
	else if (pData->nTime <= nTime)
	{
		for (S32 j = 0; j < GAME_PLAYER; ++j)
		{
			if ((pData->nAction[j] == -1) && m_AIMap[Type][pData->nPlayerID[j]])
				m_AIMap[Type][pData->nPlayerID[j]]->Update();
		}
	}
	else
	{
		for (S32 j = 0; j < GAME_PLAYER; ++j)
		{
			if ((pData->bTrustee[j]) && m_AIMap[Type][pData->nPlayerID[j]])
			{
				m_AIMap[Type][pData->nPlayerID[j]]->Update();
			}
		}
	}
}

bool CPveManager::SendPveToClient(U32 nPlayerID, S8 nType, char* Msg, U32 nError)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		TABLE_ITR itr = m_TableMap[nType].find(nPlayerID);
		if (itr != m_TableMap[nType].end())
		{
			PvpTableRef pTable = itr->second;
			if (!pTable)
			{
				CMemGuard Buffer(64 MEM_GUARD_PARAM);
				Base::BitStream sendPacket(Buffer.get(), 64);
				stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, Msg, pAccount->GetAccountId(), SERVICE_CLIENT);
				sendPacket.writeInt(nError, Base::Bit16);
				SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
				return false;
			}

			{
				for (int i = 0; i < GAME_BUFF; ++i)
				{
					if (pTable->nBuffId[0][i] == 0)
					{
						pTable->nBuffId[0][i] = g_PvpBuffDataMgr->randomBuff(i + 1);
 					}
				}
			}

			if (nError != OPERATOR_NONE_ERROR)
				nError = OPERATOR_NONE_ERROR;

			CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
			stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, Msg, pAccount->GetAccountId(), SERVICE_CLIENT);
			sendPacket.writeInt(nError, Base::Bit16);
			sendPacket.writeInt(pTable->nGameStatus, Base::Bit8);
			sendPacket.writeInt(pTable->nPvpType, Base::Bit8);
			sendPacket.writeInt(pTable->nSize[1], Base::Bit8);
			sendPacket.writeInt(pTable->nCSize[1], Base::Bit8);

			sendPacket.writeInt(GAME_BUFF, Base::Bit16);
			for (int i = 0; i < GAME_BUFF; ++i)
			{
				sendPacket.writeFlag(true);
				sendPacket.writeInt(pTable->nBuffId[0][i], Base::Bit8);
			}

			sendPacket.writeInt(GAME_BUFF, Base::Bit16);
			for (int i = 0; i < GAME_BUFF; ++i)
			{
				sendPacket.writeFlag(true);
				sendPacket.writeInt(pTable->nBuff[0][i], Base::Bit8);
			}

			sendPacket.writeInt(2, Base::Bit16);
			{
				for (S32 i = 0; i < 2; ++i)
				{
					BATTLE_MAP::iterator iter = m_BattleMap[nType].find(pTable->nPlayerID[i]);
					if (iter == m_BattleMap[nType].end())
					{
						sendPacket.writeFlag(false);
						continue;
					}

					PvpInfoRef pData;
					DATA_VEC& DataDeq = iter->second;
					if (sendPacket.writeFlag(!DataDeq.empty()))
					{
						std::string Name = GetPlayerName(pTable->nPlayerID[i]);
						sendPacket.writeString(Util::MbcsToUtf8(Name), 50);
						sendPacket.writeInt(GetScrore(GetPlayerID(pTable->nPlayerID[i])), Base::Bit32);
						pData = DataDeq.back();
						pData->WriteData(&sendPacket);
					}
				}
			}
			pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
		}
		else
		{
			CMemGuard Buffer(64 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 64);
			stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, Msg, pAccount->GetAccountId(), SERVICE_CLIENT);
			sendPacket.writeInt(nError, Base::Bit16);
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
		}
	}
	return true;
}

bool CPveManager::SendPveEndToClient(U32 nPlayerID, S32 nScore, S32 nBattleNum, S32 nAddAch, S8 bEnd)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_END_PVE_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nScore, Base::Bit32);
		sendPacket.writeInt(nBattleNum, Base::Bit32);
		sendPacket.writeInt(nAddAch, Base::Bit32);
		sendPacket.writeInt(bEnd, Base::Bit8);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPveManager::UpdatePveToClient(U32 nPlayerID, PvpInfoRef pData[2], S32 nAction[2])
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(1024 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 1024);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_PVE_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(2, Base::Bit16);
		for (S32 i = 0; i < 2; ++i)
		{
			if (sendPacket.writeFlag(pData[i] != NULL))
			{
				sendPacket.writeInt(pData[i]->Hp, Base::Bit32);
				sendPacket.writeInt(pData[i]->Peerless, Base::Bit32);
				sendPacket.writeInt(nAction[i], Base::Bit8);
			}
		}
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPveManager::HandleOpenPveRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	U32 nPlayerID = 0;
	S32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nPlayerID = pAccount->GetPlayerId();
		if (nType == 2)
		{
			nError = JoinBattle(nPlayerID, nType);
			nError = MatchBattle(nPlayerID, nType);
			SendPveToClient(nPlayerID, nType, "WC_OPEN_PVE_RESPONSE", nError);
		}
		else if (nType == 1)
		{
			SendPveToClient(nPlayerID, nType, "WC_OPEN_PVE_RESPONSE", nError);
		}
		else if (nType == 0)
		{
			SendPveToClient(nPlayerID, nType, "WC_OPEN_PVE_RESPONSE", nError);
		}
	}
	return true;
}

bool CPveManager::HandlePveBeginRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	U32 nPlayerID = 0;
	S32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nPlayerID = pAccount->GetPlayerId();
		nError = BattleBegin(nPlayerID, nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PVE_BEGIN_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPveManager::HandlePveReadyRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	U32 nPlayerID = 0;
	S32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nPlayerID = pAccount->GetPlayerId();
		nError = BattleReady(nPlayerID, nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PVE_READY_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPveManager::HandlePveEndRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	U32 nPlayerID = 0;
	S32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nPlayerID = pAccount->GetPlayerId();
		nError = BattleEndness(nPlayerID, nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PVE_END_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPveManager::HandlePveBattleRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	U32 nPlayerID = 0;
	S32 nAction = Packet->readInt(Base::Bit8);
	S32 nType = Packet->readInt(Base::Bit8);
	nType = mClamp(nType, 0, 2);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nPlayerID = pAccount->GetPlayerId();
		nError = Battle(nPlayerID, nAction, nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PVE_BATTLE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPveManager::HandlePveAvengeRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 nPlayerID = 0;
	std::string UID = Packet->readString(MAX_UUID_LENGTH);
	S32 nTargetID = Packet->readInt(Base::Bit32);
	S32 nType = Packet->readInt(Base::Bit8);
	nType = mClamp(nType, 0, 1);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nPlayerID = pAccount->GetPlayerId();
		nError = AvengeBattle(nPlayerID, nType, nTargetID, UID);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PVE_AVENGE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nType, Base::Bit8);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPveManager::HandlePveAddBuffRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 nPlayerID = 0;
	S32 nBuffId = Packet->readInt(Base::Bit8);
	S32 nType = Packet->readInt(Base::Bit8);
	nType = mClamp(nType, 0, 2);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		PvpInfoRef pData[2];
		nPlayerID = pAccount->GetPlayerId();
		nError = AddBuff(nPlayerID, nBuffId, nType, pData);
		CMemGuard Buffer(8096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 8096);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_PVE_ADDBUFF_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(nBuffId, Base::Bit8);
		sendPacket.writeInt(GAME_PLAYER, Base::Bit16);
		for (int i = 0; i < GAME_PLAYER; ++i)
		{
			if (sendPacket.writeFlag(pData[i] != NULL))
			{
				pData[i]->WriteData(&sendPacket);
			}
		}
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CPveManager::HandlePveSetJumpBattle(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 nJumpBattle = Packet->readInt(Base::Bit8);
	S32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		TABLE_ITR itr = m_TableMap[nType].find(pAccount->GetPlayerId());
		if (itr == m_TableMap[nType].end())
			nError = OPERATOR_PVE_NO_PLAYER;

		PvpTableRef pTable = itr->second;
		for (int i = 0; i < GAME_PLAYER; ++i) {
			if (pAccount->GetPlayerId() == pTable->nPlayerID[i]) {
				pTable->bJumpBattle[i] = (0 != nJumpBattle ? true : false);
			}
		}
	}

	CMemGuard Buffer(128 MEM_GUARD_PARAM);
	Base::BitStream sendPacket(Buffer.get(), 128);
	stPacketHead* pPktHead = IPacket::BuildPacketHead(sendPacket, "WC_PVE_SETJUMPBATTLE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
	sendPacket.writeInt(nError, Base::Bit16);
	pPktHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
//	 ---------------------------------------------------------------------------------------------------------------
//														PVE
//   ---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------


void ___JoinBattle(int id, int type)
{
	PVEMGR->JoinBattle(id, type);
}

void ___MatchBattle(int id, int type)
{
	PVEMGR->MatchBattle(id, type);
}

void ___Battle(int id, int action, int type)
{
	PVEMGR->Battle(id, action, type);
}

void ___JumpBattle(int id, int bJumpBattle, int type)
{
	MAKE_WORKQUEUE_PACKET(sendPacket, 128);
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WW_PVE_SETJUMPBATTLE_REQUEST", id);
	sendPacket.writeInt(bJumpBattle, Base::Bit8);
	sendPacket.writeInt(type, Base::Bit8);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SEND_WORKQUEUE_PACKET(sendPacket, WQ_NORMAL, OWQ_EnterGame);
}

void CPveManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "JoinBattle", &___JoinBattle);
	lua_tinker::def(L, "MatchBattle", &___MatchBattle);
	lua_tinker::def(L, "Battle", &___Battle);
	lua_tinker::def(L, "JumpBattle", &___JumpBattle);
}