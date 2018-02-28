#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include <hash_map>
#include <set>
#include <deque>
#include "WINTCP/IPacket.h"
#include "TaskBase.h"
#include "BaseMgr.h"
class DB_Execution;

/************************************************************************/
/* 任务
/************************************************************************/
class CTaskManager : public CBaseManagerEx<TaskInfoRef, S32>
{
	typedef CBaseManagerEx<TaskInfoRef, S32>   Parent;
public:
	CTaskManager();
	~CTaskManager();

	static CTaskManager* Instance();
	
	void SubitTaskByType(S32 nPlayerID, S32 nType, S32 nVal);
	void SubitTask(S32 nPlayerId,S32 nTaskId, S32 nVal);
	enOpError FininshTask(S32 nPlayerId, S32 nTaskId);
	void RemoveDailyTask(S32 nPlayerId);
	TaskInfoRef AddTask(S32 nPlayerID, S32 nTaskId);
	void AddPlayerTask(S32 nPlayerID);
	//----------通知客户端-----------//
	void UpdateToClient(U32 nPlayerID, TaskInfoRef pData);
	void SendInitToClient(U32 nPlayerID);

	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);

	bool UpdateDB(U32 nPlayerID, TaskInfoRef pData);
	bool InsertDB(U32 nPlayerID, TaskInfoRef pData);
	bool DeleteDB(U32 nPlayerID, S32 ID);

	bool HandleFinishTaskRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	static void Export(struct lua_State* L);

};

#define TASKMGR CTaskManager::Instance()
#define SUBMIT(nPlayerId, nType, nVal) TASKMGR->SubitTaskByType(nPlayerId, nType, nVal)
#endif//TIMESET_MANAGER_H
