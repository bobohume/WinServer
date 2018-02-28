#ifndef TIMESET_MANAGER_H
#define TIMESET_MANAGER_H

#include <hash_map>
#include <set>
#include <deque>
#include "WINTCP/IPacket.h"
#include "TimeSetBase.h"
#include "BaseMgr.h"
class DB_Execution;
template<class T>
struct CompareTimeSet
{
	bool operator()(const T _Left, const T _Right) const
	{
		if (_Left->ExpireTime < _Right->ExpireTime)
			return true;
		else if (_Left->ExpireTime == _Right->ExpireTime)
			return _Left->ID < _Right->ID;
		else
			return false;
	}
};

/************************************************************************/
/* 任务和CoolDown管理对象
/************************************************************************/
class CTimeSetManager : public CBaseManagerEx<TimeSetRef, U32>
{
	typedef std::set<TimeSetRef, CompareTimeSet<TimeSetRef>>	TIME_SORT_SET;//用于跟新排序
	typedef TIME_SORT_SET::iterator						 TIME_SORT_ITR;

	typedef stdext::hash_map<U32, TIME_SORT_SET>		 PLAYER_SORT_MAP;
	typedef PLAYER_SORT_MAP::iterator					 PLAYER_SORT_ITR;
	typedef std::deque<TimeSetRef>						 TIMESET_STACK;
public:
	CTimeSetManager();
	~CTimeSetManager();

	static CTimeSetManager* Instance();
	TimeSetRef GetData(U32 nPlayerID, U32 ID);
	TimeSetRef CreateTimeSet(U32 ID, U32 nPayerID);
	bool AddPlayerTimeSet(U32 nPayerID);
	bool AddNewPlayerTimeSet(U32 nPayerID);//玩家标志
	const char* NewPlayer(U32 nPlayerID);
	TimeSetRef AddTimeSet(U32 nPlayerID, U32 nID, S32 nExpireTime, std::string strFlag, S32 Flag1 = 0);
	void _AddTimeSet(U32 nPlayerID, TimeSetRef pTime);
	bool RemoveTimeSet(U32 nPlayerID, U32 nID);
	void _RemoveTimeSet(U32 nPlayerID, U32 nID);
	void RemovePlayer(U32 nPlayerID);
	enOpError AdvanceTime(U32 nPlayerID, S32 ID);
	//不处理扣费的加速操作
	enOpError AdvanceTimeUseOther(U32 nPlayerID, S32 ID, S32 AccTime);
	void NotifyTimeDel(U32 nPlayerID, TimeSetRef pTime);//定时器到期回调
	//----------通知客户端-----------//
	void UpdateToClient(U32 nPlayerID, TimeSetRef pData);
	void SendInitToClient(U32 nPlayerID);

	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);
	TimeSetRef SyncLoadDB(U32 nPlayerID, U32 nTimeID);

	bool UpdateDB(TimeSetRef pTime);
	bool InsertDB(U32 nPlayerID, TimeSetRef pTime);
	bool DeleteDB(U32 nPlayerID, U32 nTimeID);

	//----------角色登录通知------//
	bool onPlayerEnter(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool onPlayerLeave(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	bool HandleAdvanceTimeRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	static void Export(struct lua_State* L);

	void TimeProcess(U32 bExit);
public:
	PLAYER_SORT_MAP m_PlayerSortMap;//用于排序
	TIMESET_STACK	m_TimeStack;//删除队列
};

#define TIMESETMGR CTimeSetManager::Instance()

//用于统计不重要的数据
static auto SetTimesInfo = [](auto nPlayerID, auto nTimerID, auto nIndex, auto nVal, bool bAdd = true, S32 nExpireTime = 0) {
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, nTimerID);
	if (!pTime) {
		stTimeSetInfo info("");
		pTime = TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, nExpireTime, info.GetFlag(), 0);
	}

	if (!pTime)
		return;

	nIndex = mClamp(nIndex, 0, 5);
	std::string Flag1;
	stTimeSetInfo info("");
	if (pTime)
	{
		info = stTimeSetInfo(pTime->Flag);
		if (bAdd)
			info.nVal[nIndex] = mClamp((S32)(info.nVal[nIndex] + nVal), (S32)0, (S32)S32_MAX);
		else
		{
			if (info.nVal[nIndex] < nVal)
				info.nVal[nIndex] = mClamp((S32)nVal, (S32)0, (S32)S32_MAX);
			else
				return;
		}
	}

	TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, nExpireTime, info.GetFlag(), 0);
};

static auto GetTimesInfo = [](auto nPlayerID, auto nTimerID, auto nIndex) {
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, nTimerID);
	nIndex = mClamp(nIndex, 0, 5);
	if (pTime)
	{
		stTimeSetInfo info(pTime->Flag);
		return info.nVal[nIndex];
	}

	return 0;
};

static auto SetTimesInfoEx = [](auto nPlayerID, auto nTimerID, stTimeSetInfo info, S32 nExpireTime = 0) {
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, nTimerID);
	if (!pTime) {
		pTime = TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, nExpireTime, info.GetFlag(), 0);
	}

	if (!pTime)
		return;

	TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, nExpireTime, info.GetFlag(), 0);
};

static auto SetTimesInfoEx1 = [](auto nPlayerID, auto nTimerID, auto nIndex, auto nVal, S32 nExpireTime = 0) {
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, nTimerID);
	if (!pTime) {
		stTimeSetInfo info("");
		pTime = TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, nExpireTime, info.GetFlag(), 0);
	}

	if (!pTime)
		return;

	nIndex = mClamp(nIndex, 0, 5);
	std::string Flag1;
	stTimeSetInfo info("");
	if (pTime)
	{
		info = stTimeSetInfo(pTime->Flag);
			info.nVal[nIndex] = nVal;
	}

	TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, nExpireTime, info.GetFlag(), 0);
};
#endif//TIMESET_MANAGER_H
