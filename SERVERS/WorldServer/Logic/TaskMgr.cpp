#include "TaskMgr.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "PlayerInfoMgr.h"
#include "../WorldServer.h"
#include "../PlayerMgr.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "Common/MemGuard.h"
#include "WINTCP/dtServerSocket.h"
#include "TaskData.h"
#include "ItemMgr.h"
#include "platform.h"
#include "CommLib/redis.h"
#include "TimeSetMgr.h"
#include "CardMgr.h"
#include "PrinceMgr.h"
#include "PrincessMgr.h"
#include "OfficerMgr.h"
#include "SkillMgr.h"
//DECLARE_REDIS_UNIT(stTaskInfo);
const unsigned int MAX_TIMESET_SLOT = 500;
CTaskManager::CTaskManager()
{
	/*GET_REDIS_UNIT(stTaskInfo).SetName("Tbl_Task");
	REGISTER_REDIS_UNITKEY(stTaskInfo, PlayerID);
	REGISTER_REDIS_UNITKEY(stTaskInfo, TaskId);
	REGISTER_REDIS_UNIT(stTaskInfo, TaskVal);
	REGISTER_REDIS_UNIT(stTaskInfo, FinishFlag);*/
	REGISTER_EVENT_METHOD("CW_FINISHTIME_REQUEST", this, &CTaskManager::HandleFinishTaskRequest);
	g_TaskDataMgr->read();
}

CTaskManager::~CTaskManager()
{
	g_TaskDataMgr->close();
}

CTaskManager* CTaskManager::Instance()
{
	static CTaskManager s_Mgr;
	return &s_Mgr;
}

auto GetTaskTimerId = [](auto nType, auto nTaskId) {
	return nTaskId;
};

TaskInfoRef CTaskManager::AddTask(S32 nPlayerID, S32 nTaskId) 
{
	if (!TASKMGR->GetData(nPlayerID, nTaskId))
	{
		TaskInfoRef pData = TaskInfoRef(new stTaskInfo(nTaskId, 0, 0, nPlayerID));
		if (TASKMGR->AddData(nPlayerID, nTaskId, pData))
		{
			TASKMGR->InsertDB(nPlayerID, pData);
		}
		return pData;
	}

	return TaskInfoRef();
}

void CTaskManager::AddPlayerTask(S32 nPlayerID) 
{
	AddTask(nPlayerID, 11001);
	for (auto i = 12002; i <= 12012; ++i)
	{
		AddTask(nPlayerID, i);
	}
	for (auto i = 13011; i <= 13161; i = i + 10)
	{
		AddTask(nPlayerID, i);
	}
}

DB_Execution* CTaskManager::LoadDB(U32 nPlayerID)
{
	RemovePlayer(nPlayerID);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(nPlayerID);
	pHandle->SetSql(fmt::sprintf("SELECT TaskId, PlayerID, TaskVal, FinishFlag FROM Tbl_Task WHERE PlayerID=%d", nPlayerID));
	pHandle->RegisterFunction([this](int id, int error, void* pSqlHandle)
	{
		DB_Execution* pHandle = (DB_Execution*)(pSqlHandle);
		if (pHandle && error == NONE_ERROR)
		{
			while (pHandle->More())
			{
				TaskInfoRef pData = TaskInfoRef(new stTaskInfo);
				pData->TaskId = pHandle->GetInt();
				pData->PlayerID = pHandle->GetInt();
				pData->TaskVal = pHandle->GetInt();
				pData->FinishFlag = pHandle->GetInt();
				Parent::AddData(id, pData->TaskId, pData);
			}

		}

		AddPlayerTask(id);
	}
	);
	return pHandle;
}

bool CTaskManager::UpdateDB(U32 nPlayerID, TaskInfoRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_Task SET\
	TaskVal=%d,					FinishFlag=%d					WHERE PlayerID=%d AND			TaskId=%d",\
	pData->TaskVal,				pData->FinishFlag,				pData->PlayerID,				pData->TaskId));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CTaskManager::InsertDB(U32 nPlayerID, TaskInfoRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("INSERT INTO Tbl_Task\
		(TaskVal,					FinishFlag,					PlayerID,				TaskId)\
		VALUES(%d,					%d,							%d,						%d)",\
		pData->TaskVal,				pData->FinishFlag,			pData->PlayerID,		pData->TaskId));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CTaskManager::DeleteDB(U32 nPlayerID, S32 ID)
{
	TaskInfoRef pData = TaskInfoRef(new stTaskInfo);
	pData->PlayerID = nPlayerID;
	pData->TaskId = ID;
	pData->TaskVal = -1;
	UpdateToClient(nPlayerID, pData);

	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE FROM Tbl_Task WHERE PlayerID=%d AND TaskId=%d", nPlayerID, ID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

void CTaskManager::SubitTask(S32 nPlayerId, S32 nTaskId, S32 nVal)
{
	CTaskData* pTaskData = g_TaskDataMgr->getData(nTaskId);
	if (!pTaskData)
		return;

	S32 nTimeId = GetTaskTimerId(pTaskData->m_Type, pTaskData->m_TaskId);
	TaskInfoRef pData = GetData(nPlayerId, nTimeId);
	if (!pData)
		return;

	//主线任务
	if (pTaskData->m_Type == 1 && nTaskId != pData->TaskId)
		return;

	auto SumbitTimeSet = [&]() {
		if (!pData)
			return;

		if (pData->TaskId != nTaskId || pData->FinishFlag != 0)
			return;

		if ((pTaskData->m_Finish >= 14 && pTaskData->m_Finish <= 23) ||\
			(pTaskData->m_Finish >= 26 && pTaskData->m_Finish <= 33) ||\
			(pTaskData->m_Finish >= 36 && pTaskData->m_Finish <= 39) ||\
			(pTaskData->m_Finish >= 41 && pTaskData->m_Finish <= 47) ||\
			(pTaskData->m_Finish >= 48 && pTaskData->m_Finish <= 52) ||\
			(pTaskData->m_Finish >= 56 && pTaskData->m_Finish <= 57) ||\
			pTaskData->m_Finish == 63)
		{
			if (pData->TaskVal < nVal) {
				pData->TaskVal = mClamp((S32)nVal, (S32)0, (S32)S32_MAX);
			}
		}
		else if (pTaskData->m_Finish == 24) {
			pData->TaskVal = mClamp((S32)CARDMGR->GetCardNum(nPlayerId), (S32)0, (S32)S32_MAX);
		}
		else if (pTaskData->m_Finish == 25) {
			pData->TaskVal = mClamp((S32)PRINCESSMGR->GetPrincessNum(nPlayerId), (S32)0, (S32)S32_MAX);
		}
		else if (pTaskData->m_Finish >= 53 && pTaskData->m_Finish <= 55) {
			S32 nLevel = ((pTaskData->m_Finish == 53) ? 50 : ((pTaskData->m_Finish == 54) ? 100 : 150));
			pData->TaskVal = mClamp((S32)CARDMGR->GetCardNum(nPlayerId, nLevel), (S32)0, (S32)S32_MAX);
		}
		else if (pTaskData->m_Finish >= 58 && pTaskData->m_Finish <= 60) {
			S32 nCardId = ((pTaskData->m_Finish == 58) ? 110007 : ((pTaskData->m_Finish == 59) ? 110008 : 110009));
			pData->TaskVal = mClamp((S32)CARDMGR->GetCardLevel(nPlayerId, nCardId), (S32)0, (S32)S32_MAX);
		}
		else if (pTaskData->m_Finish == 61 || pTaskData->m_Finish == 62 || pTaskData->m_Finish == 65 || pTaskData->m_Finish == 67) {
			S32 nSeriesId = ((pTaskData->m_Finish == 61) ? 403003 : ((pTaskData->m_Finish == 62) ? 403002 : ((pTaskData->m_Finish == 65) ? 402003 : 403005)));
			S32 nLevel = 0;
			auto pSkill = SKILLMGR->GetData(nPlayerId, nSeriesId);
			if (pSkill) {
				nLevel = pSkill->Level;
			}
			pData->TaskVal = mClamp((S32)nLevel, (S32)0, (S32)S32_MAX);
		}
		else if (pTaskData->m_Finish == 68) {
			if (OFFICERMGR->GetData(nPlayerId, pTaskData->m_FinishVal)){
				pData->TaskVal = mClamp((S32)pTaskData->m_FinishVal, (S32)0, (S32)S32_MAX);
			}
		}
		else {
			pData->TaskVal = mClamp((S32)(pData->TaskVal + nVal), (S32)0, (S32)S32_MAX);
		}
		UpdateDB(nPlayerId, pData);
		UpdateToClient(nPlayerId, pData);
	};
	SumbitTimeSet();
}

enOpError CTaskManager::FininshTask(S32 nPlayerId, S32 nTaskId)
{
	CTaskData* pTaskData = g_TaskDataMgr->getData(nTaskId);
	if (!pTaskData)
		return OPERATOR_PARAMETER_ERROR;

	S32 nTimeId = GetTaskTimerId(pTaskData->m_Type, pTaskData->m_TaskId);
	TaskInfoRef pData = GetData(nPlayerId, nTimeId);
	if (!pData)
		return OPERATOR_PARAMETER_ERROR;

	auto FininshTimer = [&]() {
		if (!pData)
			return OPERATOR_PARAMETER_ERROR;

		if (pData->TaskId != nTaskId || pData->FinishFlag != 0)
			return OPERATOR_PARAMETER_ERROR;

		if (pTaskData->m_Finish == 68) {
			if (pData->TaskVal != pTaskData->m_FinishVal)
				return OPERATOR_PARAMETER_ERROR;
		}
		else {
			if (pData->TaskVal < pTaskData->m_FinishVal)
				return OPERATOR_PARAMETER_ERROR;
		}

		pData->FinishFlag = 1;
		if (pTaskData->m_NextTaskId)
		{
			CTaskData* pNextTaskData = g_TaskDataMgr->getData(pTaskData->m_NextTaskId);
			if (!pNextTaskData)
				return OPERATOR_PARAMETER_ERROR;

			TaskInfoRef pNextData = AddTask(nPlayerId, pTaskData->m_NextTaskId);
			if (pNextData)
			{
				UpdateToClient(nPlayerId, pNextData);
				SUBMIT(nPlayerId, pNextTaskData->m_Finish, 0);
			}
		}
		
		UpdateDB(nPlayerId, pData);
		UpdateToClient(nPlayerId, pData);
		return OPERATOR_NONE_ERROR;
	};

	auto GetAward = [&]() {
		if (!pTaskData)
			return OPERATOR_PARAMETER_ERROR;

		for (auto i = 0; i < 3; ++i)
		{
			if (pTaskData->m_AwardType[i] == 1)
			{
				SERVER->GetPlayerManager()->AddGold(nPlayerId, pTaskData->m_AwardVal[i]);
			}
			else if (pTaskData->m_AwardType[i] == 2)
			{
				PLAYERINFOMGR->AddMoney(nPlayerId, pTaskData->m_AwardVal[i]);
			}
			else if (pTaskData->m_AwardType[i] == 3)
			{
				PLAYERINFOMGR->AddOre(nPlayerId, pTaskData->m_AwardVal[i]);
			}
			else if (pTaskData->m_AwardType[i] == 4)
			{
				PLAYERINFOMGR->AddTroops(nPlayerId, pTaskData->m_AwardVal[i]);
			}
			else
			{
				ITEMMGR->AddItem(nPlayerId, pTaskData->m_AwardType[i], pTaskData->m_AwardVal[i]);
			}
		}
		return OPERATOR_NONE_ERROR;
	};

	enOpError error = FininshTimer();
	if (error == OPERATOR_NONE_ERROR)
	{
		return GetAward();
	}

	return error;
}

void CTaskManager::RemoveDailyTask(S32 nPlayerID)
{
	PLAYER_ITR  itr = m_PlayerMap.find(nPlayerID);
	if (itr != m_PlayerMap.end())
	{
		auto TaskMap = itr->second;
		for (auto iter = TaskMap.begin(); iter != TaskMap.end(); ++iter)
		{
			auto pData = iter->second;
			if (pData && (pData->TaskId > 12000 && pData->TaskId < 13000))
			{
				S32 nTaskId = pData->TaskId;
				pData->FinishFlag = 0;
				pData->TaskVal = 0;
				UpdateDB(nPlayerID, pData);
				UpdateToClient(nPlayerID, pData);
			}
		}
	}
}

void CTaskManager::UpdateToClient(U32 nPlayerID, TaskInfoRef pData)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(4096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 4096);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPDATE_TASK_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		pData->WriteData(&sendPacket);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CTaskManager::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		DATA_MAP& TaskMap = m_PlayerMap[nPlayerID];
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_TASK_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(TaskMap.size(), Base::Bit16);
		for (DATA_ITR itr = TaskMap.begin(); itr != TaskMap.end(); ++itr)
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

bool CTaskManager::HandleFinishTaskRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	S32 nTaskId = Packet->readInt(Base::Bit16);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		U32 nError = FininshTask(pAccount->GetPlayerId(), nTaskId);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_FINISHTIME_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

//贼了这任务，fuck，拷贝类似
void CTaskManager::SubitTaskByType(S32 nPlayerID, S32 nType, S32 nVal)
{
	if (nType > 71 || nType <= 0)
		return;

	nVal = abs(nVal);
	if (nType == 14) {
		PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
		nVal = (pPlayer ? pPlayer->BattleVal : nVal);
	}
	else if (nType == 15) {
		PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
		nVal = (pPlayer ? (pPlayer->Business + pPlayer->getBuff(0).Business + pPlayer->getBuff(1).Business + pPlayer->getBuff(2).Business + \
			pPlayer->getBuff(3).Business) : nVal);
	}
	else if (nType == 16) {
		PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
		nVal = (pPlayer ? (pPlayer->Technology + pPlayer->getBuff(0).Technology + pPlayer->getBuff(1).Technology + pPlayer->getBuff(2).Technology + \
			pPlayer->getBuff(3).Technology) : nVal);
	}
	else if (nType == 17) {
		PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
		nVal = (pPlayer ? (pPlayer->Military + pPlayer->getBuff(0).Military + pPlayer->getBuff(1).Military + pPlayer->getBuff(2).Military + \
			pPlayer->getBuff(3).Military) : nVal);
	}
	else if (nType == 18) {
		PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
		nVal = (pPlayer ? pPlayer->Land : nVal);
	}
	else if (nType == 19) {
		PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
		nVal = (pPlayer ? pPlayer->QueenVal : nVal);
	}
	else if (nType == 20) {
		SetTimesInfo(nPlayerID, COUNT_TIMER1, 0, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER1, 0);
	}
	else if (nType == 21) {
		SetTimesInfo(nPlayerID, COUNT_TIMER1, 1, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER1, 1);
	}
	else if (nType == 22) {
		SetTimesInfo(nPlayerID, COUNT_TIMER1, 2, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER1, 2);
	}
	else if (nType == 23) {
		SetTimesInfo(nPlayerID, COUNT_TIMER1, 3, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER1, 3);
	}
	else if (nType == 26) {
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER2, 0);
	}
	else if (nType == 27) {
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER2, 1);
	}
	else if (nType == 28) {
		TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, GATE_IMTER);
		nVal = (pTime) ? (pTime->Flag1) : nVal;
	}
	else if (nType == 29) {
		SetTimesInfo(nPlayerID, COUNT_TIMER2, 2, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER2, 2);
	}
	else if (nType == 30) {
		SetTimesInfo(nPlayerID, COUNT_TIMER2, 3, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER2, 3);
	}
	else if (nType == 31) {
		SetTimesInfo(nPlayerID, COUNT_TIMER2, 4, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER2, 4);
	}
	else if (nType == 32) {
		SetTimesInfo(nPlayerID, COUNT_TIMER2, 5, nVal, false);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER2, 5);
	}
	else if (nType == 33) {
		SetTimesInfo(nPlayerID, COUNT_TIMER3, 0, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER3, 0);
	}
	else if (nType == 36) {
		SetTimesInfo(nPlayerID, COUNT_TIMER3, 1, nVal, false);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER3, 1);
	}
	else if (nType == 37) {
		SetTimesInfo(nPlayerID, COUNT_TIMER3, 2, nVal, false);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER3, 2);
	}
	else if (nType == 38) {
		SetTimesInfo(nPlayerID, COUNT_TIMER3, 3, nVal, false);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER3, 3);
	}
	else if (nType == 39) {
		SetTimesInfo(nPlayerID, COUNT_TIMER3, 4, nVal, false);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER3, 4);
	}
	else if (nType == 41) {
		SetTimesInfo(nPlayerID, COUNT_TIMER3, 5, nVal, false);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER3, 5);
	}
	else if (nType == 42) {
		SetTimesInfo(nPlayerID, COUNT_TIMER5, 0, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER5, 0);
	}
	else if (nType == 43) {
		SetTimesInfo(nPlayerID, COUNT_TIMER5, 1, nVal, false);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER5, 1);
	}
	else if (nType == 44) {
		SetTimesInfo(nPlayerID, COUNT_TIMER5, 2, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER5, 2);
	}
	else if (nType == 45) {
		SetTimesInfo(nPlayerID, COUNT_TIMER5, 3, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER5, 3);
	}
	else if (nType == 46) {
		SetTimesInfo(nPlayerID, COUNT_TIMER5, 4, nVal, false);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER5, 4);
	}
	else if (nType == 47) {
		SetTimesInfo(nPlayerID, COUNT_TIMER5, 5, nVal, false);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER5, 5);
	}
	else if (nType == 48) {
		nVal = GetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 0);
	}
	else if (nType == 49) {
		nVal = GetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 1);
	}
	else if (nType == 50) {
		nVal = GetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 2);
	}
	else if (nType == 51) {
		nVal = GetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 3);
	}
	else if (nType == 52) {
		SetTimesInfo(nPlayerID, COUNT_TIMER7, 0, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER7, 0);
	}
	else if (nType == 56) {
		SetTimesInfo(nPlayerID, COUNT_TIMER7, 4, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER7, 4);
	}
	else if (nType == 57) {
		nVal = GetTimesInfo(nPlayerID, CARD_HIRE_TIMER1, 4);
	}
	else if (nType == 63) {
		SetTimesInfo(nPlayerID, COUNT_TIMER8, 0, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER8, 0);
	}
	else if (nType == 64) {
		SetTimesInfo(nPlayerID, COUNT_TIMER8, 1, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER8, 1);
	}
	else if (nType == 66) {
		SetTimesInfo(nPlayerID, COUNT_TIMER8, 2, nVal);
		nVal = GetTimesInfo(nPlayerID, COUNT_TIMER8, 2);
	}

	for (auto itr = g_TaskDataMgr->m_TypeTaskMap.lower_bound(nType); itr != g_TaskDataMgr->m_TypeTaskMap.upper_bound(nType); ++itr)
	{
		SubitTask(nPlayerID, itr->second, nVal);
	}
}

void ___Submin(S32 nPlayerID, S32 nType, S32 nVal) {
	SUBMIT(nPlayerID, nType, nVal);
}

void ___FixTimesInfo(int nPlayerID) {
	S32 nVal = 0;
	for (auto type = 20; type <= 23; ++type) {
		std::set<S32> tempSet = g_TaskDataMgr->GetTaskIdByType(type);
		for (auto itr = tempSet.rbegin(); itr != tempSet.rend(); ++itr) {
			auto pTask = TASKMGR->GetData(nPlayerID, *itr);
			if (pTask) {
				nVal = pTask->TaskVal;
				if(nVal != GetTimesInfo(nPlayerID, COUNT_TIMER1, type-20)) {
					SetTimesInfo(nPlayerID, COUNT_TIMER1, type - 20, nVal, false);
					SUBMIT(nPlayerID, type, 0);
				}
				break;
			}
		}
	}
}

void CTaskManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "submit", ___Submin);
	lua_tinker::def(L, "FixTimesInfo", &___FixTimesInfo);
}