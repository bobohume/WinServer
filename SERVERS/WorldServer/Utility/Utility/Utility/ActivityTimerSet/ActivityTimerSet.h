#ifndef _UTILITY_ACTIVITYTIMERSET_H_
#define _UTILITY_ACTIVITYTIMERSET_H_
#include "base/types.h"
#include "Common/PlayerStruct.h"
#include "../../AccountHandler.h"
#include <set>
#include <vector>
#include <string>

class TimerSetMgr
{
public:
	enum enMissionFlag
	{
		MISSION_ACCEPT_FLAG			= BIT(0),
		MISSION_FINISH_FLAG			= BIT(1),
	};
	TimerSetMgr()
	{
	}

	~TimerSetMgr()
	{
	}

	//接收任务
	bool AcceptMission(U32 nMissionId, Player* pPlayer, int SocketHandle);
	bool CanAcceptMisssion(U32 nMissionId, Player* pPlayer);

	bool SubmitMission(U32 nMissionId, Player* pPlayer, int SocketHandle);
	bool CanSubmitMission(U32 nMissionId,Player* pPlayer);

	//改变任务标志
	bool ChangeMission(U32 nMissionId, Player* pPlayer, S32 flag1, S32 flag2, int SocketHandle);

	bool ResetMission(U32 nMissionId, Player* pPlayer, int SocketHandle);

	static TimerSetMgr* getInstance()
	{
		static TimerSetMgr s_cTimerSetMgr;
		return &s_cTimerSetMgr;
	}
};

#define TIMERSETMGR TimerSetMgr::getInstance()
#endif