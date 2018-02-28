#include "DinnerMgr.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "../WorldServer.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "../Script/lua_tinker.h"
#include "Common/MemGuard.h"
#include "Common/mRandom.h"
#include "../PlayerMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "CardMgr.h"
#include "PlayerInfoMgr.h"
#include "SkillData.h"
#include "BuffData.h"
#include "CopyData.h"
#include "TimeSetMgr.h"
#include "ItemMgr.h"
#include "TaskMgr.h"
#include "Commlib/ToSql.h"
#include "BASE/base64.h"
#include "PrincessMgr.h"
#include "DBLayer/Data/TBLExecution.h"
#include "../Script/lua_tinker.h"
#include "platform.h"
#include "../MailManager.h"
#include "PrincessData.h"
#include "RandomNameData.h"

const static S32 s_CostGold[2] ={
	500, 100
};

const static S32 s_FixAward[2][3] = {
	{500,	600096,	 10},
	{50,	600096,	 2},
};

const static S32 s_Award[MAX_DINNER_PLAYER] = {
	100,	80,	 60, 40, 20 
};

#define FREE_JOIN_DINNER_ID   600101
#define FREE_CREATE_DINNER_ID 600102


DECLARE_SQL_UNIT(stDinner);
CDinnerManager::CDinnerManager() 
{
	REGISTER_EVENT_METHOD("CW_OPEN_DINNER_REQUEST", this, &CDinnerManager::HandleOpenDinnerRequest);	
	REGISTER_EVENT_METHOD("CW_JOIN_DINNER_REQUEST", this, &CDinnerManager::HandleJoinDinnerRequest);
	
	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CDinnerManager::TimeProcess, 2 * 1000);

	GET_SQL_UNIT(stDinner).SetName("Tbl_Dinner");
	REGISTER_SQL_UNITKEY(stDinner, Id);
	REGISTER_SQL_UNIT(stDinner, DinnerState);
	REGISTER_SQL_UNIT(stDinner, DinnerNum);
	REGISTER_SQL_UNIT(stDinner, DinnerType);
	REGISTER_SQL_UNIT(stDinner, CreateTime);
	REGISTER_SQL_UNIT(stDinner, PlayerId);
	REGISTER_SQL_UNIT(stDinner, PrincessID);
	REGISTER_SQL_UNIT(stDinner, PlayerName);
	REGISTER_SQL_UNIT(stDinner, JoinType);
	REGISTER_SQL_UNIT(stDinner, PrincessSkill[0]);
	REGISTER_SQL_UNIT(stDinner, PrincessSkill[1]);
	REGISTER_SQL_UNIT(stDinner, PrincessSkill[2]);
	REGISTER_SQL_UNIT(stDinner, PrincessSkill[3]);
	REGISTER_SQL_UNIT(stDinner, PrincessSkill[4]);

	REGISTER_SQL_UNIT(stDinner, ScoreData[0]);
	REGISTER_SQL_UNIT(stDinner, ScoreData[1]);
	REGISTER_SQL_UNIT(stDinner, ScoreData[2]);
	REGISTER_SQL_UNIT(stDinner, ScoreData[3]);
	REGISTER_SQL_UNIT(stDinner, ScoreData[4]);

	REGISTER_SQL_UNIT(stDinner, InfoData[0]);
	REGISTER_SQL_UNIT(stDinner, InfoData[1]);
	REGISTER_SQL_UNIT(stDinner, InfoData[2]);
	REGISTER_SQL_UNIT(stDinner, InfoData[3]);
	REGISTER_SQL_UNIT(stDinner, InfoData[4]);
	g_PrincessRobotDataMgr->read();
	m_MatchTimer.Start();
}

CDinnerManager::~CDinnerManager() 
{
	g_PrincessRobotDataMgr->close();
	m_MatchTimer.Stop();
}

CDinnerManager* CDinnerManager::Instance()
{
	static CDinnerManager s_Mgr;
	return &s_Mgr;
}

void CDinnerManager::_AddDinner(DinnerRef pData)
{
	m_DinnerMap[pData->Id] = pData;
	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		if(pData->PlayerId[i])
			m_PlayerMap[pData->PlayerId[i]] = pData;
	}
}

void CDinnerManager::DelDinner(U32 nID) 
{

}

auto GetEmptyDinnerSlot = [](auto pData) {
	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		if (pData->PlayerId[i] == 0) {
			return i;
		}
	}

	return MAX_DINNER_PLAYER;
};

auto GetDinnerSlot = [](auto pData, S32 nPlayerID) {
	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		if (pData->PlayerId[i] == nPlayerID) {
			return i;
		}
	}

	return MAX_DINNER_PLAYER;
};

auto IsJoinFreeTime = [](S32 nPlayerID, S32 nType) {
	if (nType == 0) {
		if (ITEMMGR->CanReduceItem(nPlayerID, FREE_CREATE_DINNER_ID, 1))
			return 3;
	}
	else {
		TimeSetRef pTimer = TIMESETMGR->GetData(nPlayerID, DINNER_FREE_TIME);
		if (!pTimer)
			return 1;

		if (ITEMMGR->CanReduceItem(nPlayerID, FREE_JOIN_DINNER_ID, 1))
			return 2;
	}
	
	return 0;
};

auto CanJoinBattle = [](S32 nPlayerID, S32 nType) {
	if (nPlayerID < 10000)
		return true;

	if (IsJoinFreeTime(nPlayerID, nType))
		return true;

	if (SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -s_CostGold[nType]))
		return true;

	return false;
};


auto SetScoreInfo = [](auto pData) {
	char strStr[MAX_DINNER_PLAYER][MAX_SCOREDATA_NUM] = { "" };
	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		memcpy(strStr[i], pData->Score[i], sizeof(pData->Score[i]));
		base64_encode((const unsigned char *)strStr[i], pData->ScoreData[i], sizeof(pData->Score[i]), MAX_SCOREDATA_NUM);
		memcpy(strStr[i], &pData->Info[i], sizeof(pData->Info[i]));
		base64_encode((const unsigned char *)strStr[i], pData->InfoData[i], sizeof(pData->Info[i]), MAX_INFODATA_NUM);
		memcpy(strStr[i], pData->Skill[i], sizeof(pData->Skill[i]));
		base64_encode((const unsigned char *)strStr[i], pData->PrincessSkill[i], sizeof(pData->Skill[i]), MAX_INFODATA_NUM);
	}
};

auto GetScoreInfo = [](auto pData) {
	char strStr[MAX_DINNER_PLAYER][MAX_SCOREDATA_NUM] = { "" };
	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		base64_decode(pData->ScoreData[i], (unsigned char*)strStr[i], MAX_SCOREDATA_NUM);
		memcpy(pData->Score[i], strStr[i], sizeof(pData->Score[i]));
		base64_decode(pData->InfoData[i], (unsigned char*)strStr[i], MAX_INFODATA_NUM);
		memcpy(&pData->Info[i], strStr[i], sizeof(pData->Info[i]));
		base64_decode(pData->PrincessSkill[i], (unsigned char*)strStr[i], MAX_INFODATA_NUM);
		memcpy(pData->Skill[i], strStr[i], sizeof(pData->Skill[i]));
	}
};

auto NofiyPlayer = [](DinnerRef pData, auto nPlayerId) {
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if (pAccount)
	{
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_DINNERINFO_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
};

auto DinnerBegin = [](auto pData) {
	bool bAllFit = true;
	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i)
	{
		//钱不够提出
		if (!CanJoinBattle(pData->PlayerId[i], pData->JoinType[i])) {
			bAllFit = false;
		}
	}

	if (bAllFit) 
	{
		pData->StartTime = time(NULL);
		pData->DinnerState = DINNER_STATE_BEGIN;
		pData->ExecTime = time(NULL);
		for (auto itr = pData->PlayerSet.begin(); itr != pData->PlayerSet.end(); ++itr) {
			NofiyPlayer(pData, *itr);
		}
		pData->PlayerSet.clear();
		pData->DinnerType = gRandGen.randI(0, 2);
		for (auto i = 0; i < MAX_DINNER_PLAYER; ++i)
		{
			for (int j = DINNER_PRINCESS_TIME1; j < DINNER_PRINCESS_TIME1 + 80; ++j)
			{
				TimeSetRef pTime = TIMESETMGR->GetData(pData->PlayerId[i], j);
				if (!pTime) {
					TIMESETMGR->AddTimeSet(pData->PlayerId[i], j, Platform::getNextTime(Platform::INTERVAL_DAY) - _time32(NULL), "", pData->PrincessID[i]);
					break;
				}
			}

			auto nFree = IsJoinFreeTime(pData->PlayerId[i], pData->JoinType[i]);
			if (nFree == 1){
				TIMESETMGR->AddTimeSet(pData->PlayerId[i], DINNER_FREE_TIME, Platform::getNextTime(Platform::INTERVAL_DAY) - time(NULL), "", 0);
			}
			else if (nFree == 2) {
				ITEMMGR->AddItem(pData->PlayerId[i], FREE_JOIN_DINNER_ID, -1);
			}
			else if (nFree == 3) {
				ITEMMGR->AddItem(pData->PlayerId[i], FREE_CREATE_DINNER_ID, -1);
			}
			else {
				SERVER->GetPlayerManager()->AddGold(pData->PlayerId[i], -s_CostGold[i], LOG_GOLD_DINNER);
			}
		}
	}
	else {//解散
		DINNERMGR->DisbandDinner(pData->Id);
	}
};


enOpError CDinnerManager::JoinDinner(S32 nPlayerID, std::string UID, S32 nType)
{
	if (m_PlayerMap.find(nPlayerID) != m_PlayerMap.end())
		return OPERATOR_DINNER_ALREADY;

	if (m_DinnerReadyMap.find(nPlayerID) != m_DinnerReadyMap.end())
		return OPERATOR_DINNER_NO;

	PrincessRef pPrincessData = PRINCESSMGR->GetData(nPlayerID, UID);
	if (!pPrincessData) {
		return OPERATOR_PRINCESS_NO;
	}

	if (!CanJoinBattle(nPlayerID, nType))
		return OPERATOR_HASNO_GOLD;

	for (int i = DINNER_PRINCESS_TIME1; i < DINNER_PRINCESS_TIME1 + 80; ++i)
	{
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, i);
		if (pTime && pTime->Flag1 == pPrincessData->PrincessID) {
			return OPERATOR_PRINCESS_NO;
		}
	}

	DinnerReadyRef pData = DinnerReadyRef(new stDinnerReadyInfo);
	pData->PrincessID	= pPrincessData->PrincessID;
	pData->PlayerId		= nPlayerID;
	pData->Info			= pPrincessData;
	pData->JoinType		= nType;
	PlayerSimpleDataRef pPlayer = SERVER->GetPlayerManager()->GetPlayerData(nPlayerID);
	if (pPlayer) {
		pData->PlayerName = pPlayer->PlayerName;
	}
	m_DinnerReadyMap.insert(DINNER_READY_MAP::value_type(nPlayerID, pData));
	return OPERATOR_NONE_ERROR;
}

void CDinnerManager::DisbandDinner(S32 nID) {
	auto itr = m_DinnerMap.find(nID);
	if (itr == m_DinnerMap.end())
		return;

	DinnerRef pData = itr->second;
	if (!pData || pData->DinnerState != DINNER_STATE_NONE)
		return;

	pData->DinnerState = DINNER_STATE_END;
	UpdateDB(pData);
	SendInitToClient(pData);
	return;
}

enOpError CDinnerManager::LeaveDinner(S32 nPlayerID)
{
	m_DinnerReadyMap.erase(nPlayerID);
	return OPERATOR_NONE_ERROR;
}

void CDinnerManager::CreateDinner(DinnerReadyRef Ready[MAX_DINNER_PLAYER])
{
	{
		TBLExecution tHandle(SERVER->GetActorDB());
		tHandle.SetSql(fmt::sprintf("EXECUTE Sp_CreateDinner %d,%d,%d,%d,%d,\
							'%s','%s','%s','%s','%s',\
							%d,%d,%d,%d,%d,%d", \
			Ready[0]->PlayerId, Ready[1]->PlayerId, Ready[2]->PlayerId, Ready[3]->PlayerId, Ready[4]->PlayerId, \
			Ready[0]->PlayerName.c_str(), Ready[1]->PlayerName.c_str(), Ready[2]->PlayerName.c_str(), Ready[3]->PlayerName.c_str(), Ready[4]->PlayerName.c_str(), \
			Ready[0]->PrincessID, Ready[1]->PrincessID, Ready[2]->PrincessID, Ready[3]->PrincessID, Ready[4]->PrincessID, (int)time(NULL)));
		tHandle.SetId(0);
		tHandle.RegisterFunction([&](int id, int error, void* pSqlHandle)
		{
			CDBConn* pHandle = (CDBConn*)(pSqlHandle);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					S32 nID = pHandle->GetInt();
					S32 nCreateTime = pHandle->GetInt();
					if (nID == -1) {

					}
					else
					{
						DinnerRef pData = DinnerRef(new stDinner);
						pData->Id = nID;
						pData->CreateTime = nCreateTime;
						for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
							pData->PlayerId[i]	 = Ready[i]->PlayerId;
							pData->PlayerName[i] = Ready[i]->PlayerName;
							pData->PrincessID[i] = Ready[i]->PrincessID;
							pData->Info[i]		 = Ready[i]->Info;
							pData->JoinType[i]	 = Ready[i]->JoinType;
						}
						SetScoreInfo(pData);
						DINNERMGR->_AddDinner(pData);
						DinnerBegin(pData);
						UpdateDB(pData);
						SendInitToClient(pData);
					}
				}
			}
		});
		tHandle.Commint();
	}
}

void CDinnerManager::MatchDinner() 
{
	auto RandomPrincessRobot = [&]() {
		auto pData = g_PrincessRobotDataMgr->RandRobot();
		DinnerReadyRef Ready = DinnerReadyRef(new stDinnerReadyInfo);
		Ready->PlayerId = pData->m_Id;
		Ready->PlayerName = RandName();
		Ready->PrincessID = pData->m_PrincessId;
		Ready->Info = pData;
		return Ready;
	};

	DINNER_DEQ DinnerDeq;
	for (auto itr = m_DinnerReadyMap.begin(); itr != m_DinnerReadyMap.end(); ++itr) {
		DinnerDeq.push_back(itr->second);
	}
	m_DinnerReadyMap.clear();

	while (!DinnerDeq.empty())
	{
		if (DinnerDeq.size() >= MAX_DINNER_PLAYER) {
			DinnerReadyRef Ready[MAX_DINNER_PLAYER];
			for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
				Ready[i] = DinnerDeq.back();
				DinnerDeq.pop_back();
			}
			CreateDinner(Ready);
		}
		else//不足补充机器人 
		{
			S32 nCurSize = 0;
			DinnerReadyRef Ready[MAX_DINNER_PLAYER];
			for (auto i = 0; i < DinnerDeq.size(); ++i) {
				Ready[i] = DinnerDeq.back();
				DinnerDeq.pop_back();
				nCurSize++;
			}

			for (auto i = nCurSize; i < MAX_DINNER_PLAYER; ++i) {
				Ready[i] = RandomPrincessRobot();
			}
			CreateDinner(Ready);
		}
	}
}

auto GetPrincessScore = [](auto Info, auto nType) {
	F32 fRandVal = gRandGen.randF(0.90f, 1.10f);
	if (nType == DINNER_RITE)
		return (S32)(Info.Rite * 0.01 * fRandVal) + 1;
	else if (nType ==  DINNER_CULTURE)
		return (S32)(Info.Culture * 0.01 * fRandVal) + 1;
	else
		return (S32)(Info.Battle * 0.01 * fRandVal) + 1;
};

auto CaculateScore = [](auto pData) {
	if (!pData)
		return;

	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		auto nScore = GetPrincessScore(pData->Info[i], pData->DinnerType);
		pData->Score[i][pData->DinnerNum] = nScore;
		pData->Score[i][MAX_DINNER_NUM] += nScore;
	}
};

auto CaculateSkill = [](auto pData) {
	if (!pData)
		return;

	S32 nRate[MAX_DINNER_PLAYER] = { 0,0,0,0,0};
	S32 nRateSum = 0;
	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		nRateSum += pData->Info[i].Charm;
		nRate[i] = nRateSum;
	}

	nRateSum *= 2;
	auto nCurRate = gRandGen.randI(0, nRateSum);
	auto nSlot = MAX_DINNER_PLAYER;
	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		if (nCurRate < nRate[i]) {
			nSlot = i;
			break;
		}
	}
	//发动技能
	if (nSlot != MAX_DINNER_PLAYER) {
		auto nSkillType = gRandGen.randI(0, DINNER_SKILL_MAX);
		pData->Skill[nSlot][pData->DinnerNum] = nSkillType << 4 & 0xF0;
		switch (nSkillType)
		{
			case DINNER_SKILL_HUATI:
			{
				std::vector<int> Vec = { pData->Info[nSlot].Rite, pData->Info[nSlot].Culture, pData->Info[nSlot].Battle };
				std::stable_sort(Vec.begin(), Vec.end(), [](int a, int b) {
					return a > b;
				});
			
				if (Vec[0] == pData->Info[nSlot].Rite) {
					pData->DinnerType = DINNER_RITE;
				}
				else if (Vec[0] == pData->Info[nSlot].Culture) {
					pData->DinnerType = DINNER_CULTURE;
				}
				else {
					pData->DinnerType = DINNER_BATTLE;
				}
			}
			break;
			case DINNER_SKILL_QUANJIE: 
			{
				std::vector<std::pair<int, int>> Vec = { {0,0},{ 1,0 },{ 2,0 },{ 3,0 },{ 4,0 } };
				std::vector<std::pair<int, int>> Vec1;
				for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
					Vec[i].second = pData->Score[i][MAX_DINNER_NUM];
				}

				std::stable_sort(Vec.begin(), Vec.end(), [](std::pair<int, int> a, std::pair<int, int> b) {
					return a.second > b.second;
				});
		
				for (auto itr = Vec.begin(); itr != Vec.end(); ++itr) {
					if (itr->first != nSlot) {
						Vec1.push_back(*itr);
					}
					else {
						break;
					}
				}

				if (!Vec1.empty()) {
					auto nRate = gRandGen.randI(0, Vec1.size() - 1);
					auto nTargetSlot = Vec1[nRate].first;
					S32 nScroe = 0;
					S32 nAddScroe = abs(pData->Score[nSlot][MAX_DINNER_NUM] - pData->Score[nTargetSlot][MAX_DINNER_NUM]);
					pData->Score[nSlot][pData->DinnerNum] += nAddScroe;
					pData->Score[nTargetSlot][pData->DinnerNum] -= nAddScroe;
					
					nScroe = pData->Score[nSlot][MAX_DINNER_NUM];
					pData->Score[nSlot][MAX_DINNER_NUM] = pData->Score[nTargetSlot][MAX_DINNER_NUM];
					pData->Score[nTargetSlot][MAX_DINNER_NUM] = nScroe;
					pData->Skill[nSlot][pData->DinnerNum] |= nTargetSlot & 0x0F;
				}
			}
			break;
			case DINNER_SKILL_QISHI:
			{
				pData->Score[nSlot][pData->DinnerNum] += pData->Score[nSlot][MAX_DINNER_NUM];
				pData->Score[nSlot][MAX_DINNER_NUM] *= 2;
			}
			break;
			case DINNER_SKILL_SHOULI:
			{
				pData->Info[nSlot].Rite *= 2;
			}
			break;
			case DINNER_SKILL_NEILIAN:
			{
				pData->Info[nSlot].Culture *= 2;
			}
			break;
			case DINNER_SKILL_XINJI:
			{
				pData->Info[nSlot].Battle *= 2;
			}
			break;
			case DINNER_SKILL_CHENGZHUO:
			{
				pData->Info[nSlot].Rite *= 2;
				pData->Info[nSlot].Culture *= 2;
				pData->Info[nSlot].Battle *= 2;
			}
			break;
			case DINNER_SKILL_MEIHUO:
			{
				std::vector<int> Vec = { 0, 1, 2, 3, 4};
				for (auto itr = Vec.begin(); itr != Vec.end(); ++itr) {
					if (*itr == nSlot) {
						Vec.erase(itr);
						break;
					}
				}

				if (!Vec.empty()) {
					auto nRate = gRandGen.randI(0, Vec.size() - 1);
					auto nTargetSlot = Vec[nRate];
					S32 nScroe = 0;
					{
						nScroe = pData->Score[nTargetSlot][MAX_DINNER_NUM] / 2;
						pData->Score[nSlot][pData->DinnerNum] += nScroe;
						pData->Score[nTargetSlot][pData->DinnerNum] -= nScroe;

						pData->Score[nSlot][MAX_DINNER_NUM] += nScroe;
						pData->Score[nTargetSlot][MAX_DINNER_NUM] -= nScroe;
					}
					pData->Skill[nSlot][pData->DinnerNum] |= nTargetSlot & 0x0F;
				}
			}
			break;
			default:
				break;
		}
	
	}
};

auto DinnerCaculate = [](auto pData){
	S32 nScore[MAX_DINNER_PLAYER] = { 0,	0,	0,	0,	0 };
	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		nScore[i] = pData->Score[i][MAX_DINNER_NUM];
	}

	std::vector<std::pair<int, int>> Vec = { { 0,nScore[0] },{ 1,nScore[1] },{ 2,nScore[2] },{ 3,nScore[3] },{ 4,nScore[4] } };
	std::stable_sort(Vec.begin(), Vec.end(), [](std::pair<int, int> a, std::pair<int, int> b) {
		return a.second > b.second;
	});

	auto nSlot = 0;
	for (auto itr = Vec.begin(); itr != Vec.end(); ++itr) {
		auto nPlayerID = pData->PlayerId[itr->first];
		//排名奖励
		if (nPlayerID > 10000) {
			PLAYERINFOMGR->AddCashGift(nPlayerID, s_Award[nSlot]);
			auto nType = pData->JoinType[itr->first];
			//固定奖励
			if (PLAYERINFOMGR->GetData(nPlayerID)) {
				ITEMMGR->AddItem(nPlayerID, s_FixAward[nType][1], s_FixAward[nType][2]);
			}
			else {
				int itemId[4] = { s_FixAward[nType][1], 0, 0, 0 };
				int itemNum[4] = { s_FixAward[nType][2], 0, 0, 0 };
				MAILMGR->sendMail(0, nPlayerID, 0, itemId, itemNum, "宴会奖励", "宴会奖励,请查收", true);
			}
			PLAYERINFOMGR->AddCashGift(nPlayerID, s_FixAward[nType][0]);
		}
		nSlot++;
	}
};

void CDinnerManager::ExecDinner(DinnerRef pData, S32 nTime) 
{
	if (!pData)
		return;
	
	if (nTime - pData->ExecTime < DINNER_EXEC_TIME)
		return;

	CaculateScore(pData);
	pData->ExecTime = nTime;
	CaculateSkill(pData);
	pData->DinnerNum = mClamp(pData->DinnerNum + 1, 0, MAX_DINNER_NUM);
	//结束
	if (pData->DinnerNum >= MAX_DINNER_NUM) {
		DinnerCaculate(pData);
		pData->DinnerState = DINNER_STATE_END;
	}

	UpdateDB(pData);
	SendInitToClient(pData);
}

void CDinnerManager::LoadDB(){
	TBLExecution tbl_handle(SERVER->GetActorDB());
	tbl_handle.SetSql(GET_SQL_UNIT(stDinner).LoadSql());
	tbl_handle.RegisterFunction([this](int id, int error, void* pSqlHandle)
	{
		CDBConn* pHandle = (CDBConn*)(pSqlHandle);
		if (pHandle && error == NONE_ERROR)
		{
			while (pHandle->More())
			{
				DinnerRef pData = DinnerRef(new stDinner);
				pData->Id = pHandle->GetInt();
				pData->DinnerState = pHandle->GetInt();
				pData->DinnerNum = pHandle->GetInt();
				pData->DinnerType = pHandle->GetInt();
				pData->CreateTime = pHandle->GetInt();
				for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
					pData->PlayerId[i] = pHandle->GetInt();
				}
				for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
					pData->PrincessID[i] = pHandle->GetInt();
				}
				for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
					pData->PlayerName[i] = pHandle->GetString();
				}
				for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
					pData->JoinType[i]   = pHandle->GetInt();
				}
				for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
					dMemcpy(pData->PrincessSkill[i], pHandle->GetString(), MAX_INFODATA_NUM);
				}
				for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
					dMemcpy(pData->ScoreData[i], pHandle->GetString(), MAX_SCOREDATA_NUM);
				}
				for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
					dMemcpy(pData->InfoData[i], pHandle->GetString(), MAX_INFODATA_NUM);
				}
				GetScoreInfo(pData);
				DINNERMGR->_AddDinner(pData);
			}
		}
	}
	);
	tbl_handle.Commint();
}

bool CDinnerManager::UpdateDB(DinnerRef pData)
{
	SetScoreInfo(pData);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(GET_SQL_UNIT(stDinner).UpdateSql(pData));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CDinnerManager::DeleteDB(DinnerRef pData)
{
	if (!pData)
		return false;

	//正常宴会完整记录数据库
	if(pData->DinnerNum == MAX_DINNER_NUM)
	{
		for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
			//SUBMIT(pData->PlayerId[i], 56, 1);
			SUBMITACTIVITY(pData->PlayerId[i], ACTIVITY_DINNER, 1);
		}
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("INSERT INTO Tbl_Dinner_Delete SELECT*, getDate() FROM Tbl_Dinner WHERE ID = %d", pData->Id));
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	}

	{
		DB_Execution* pHandle = new DB_Execution();
		pHandle->SetSql(fmt::sprintf("DELETE FROM Tbl_Dinner WHERE ID = %d", pData->Id));
		SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	}

	return true;
}

void CDinnerManager::TimeProcess(U32 bExit)
{
	//匹配玩家
	if (m_MatchTimer.CheckTimer()) 
	{
		MatchDinner();
	}

	S32 nTime = (S32)time(NULL);
	for (auto itr = m_DinnerMap.begin(); itr != m_DinnerMap.end();) {
		DinnerRef pData = itr->second;
		if (pData) 
		{
			if (pData->DinnerState == DINNER_STATE_NONE) {
			
			}
			else if (pData->DinnerState == DINNER_STATE_BEGIN) {
				ExecDinner(pData, nTime);
			}

			if (pData->DinnerState == DINNER_STATE_END) {
				DeleteDB(pData);
				for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
					m_PlayerMap.erase(pData->PlayerId[i]);
				}
				itr = m_DinnerMap.erase(itr);
			}
			else 
			{
				++itr;
			}
		}
		else 
		{
			++itr;
		}
	}
}

void CDinnerManager::SendInitToClient(DinnerRef pData)
{
	if (!pData)
		return;

	if (pData->DinnerState == DINNER_STATE_NONE) {
		for (auto itr = pData->PlayerSet.begin(); itr != pData->PlayerSet.end(); ++itr) {
			NofiyPlayer(pData, *itr);
		}
	}

	for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
		NofiyPlayer(pData, pData->PlayerId[i]);
	}
}

void CDinnerManager::SendInitToClient(U32 nPlayerID) {
	auto itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end()) {
		if (itr->second) {
			AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
			if (pAccount)
			{
				CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
				Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
				stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_DINNERINFO_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
				itr->second->WriteData(&sendPacket);
				pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
				SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
			}
		}
	}	
}

bool CDinnerManager::HandleOpenDinnerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		SendInitToClient(pAccount->GetPlayerId());
	}
	return true;
}

bool CDinnerManager::HandleJoinDinnerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) 
{
	U32 nAccountId = pHead->Id;
	S32 nType = Packet->readInt(Base::Bit32);
	std::string strUID = Packet->readString(MAX_UUID_LENGTH);
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = JoinDinner(pAccount->GetPlayerId(), strUID, nType);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_JOIN_DINNER_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

void test111() {

}


void CDinnerManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "test1111", &test111);
}