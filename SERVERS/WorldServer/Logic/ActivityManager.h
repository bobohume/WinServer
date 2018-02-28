#ifndef __ACTIVITY_MANAGER_H
#define __ACTIVITY_MANAGER_H

#include "WINTCP/IPacket.h"
#include "Common/PlayerStruct.h"
#include "Common/Common.h"
#include "BaseMgr.h"
#include "ActivityBase.h"
#include <set>
#include <unordered_map>

/************************************************************************/
/* 战斗管理对象
/************************************************************************/
class CActivityManager : public CBaseManager<ActivityRef>
{
	typedef std::unordered_multimap<S32, ActivityRef> ACTIVITYMULTIMAP;
public:
	CActivityManager();
	~CActivityManager();

	static CActivityManager* Instance();
	
	void CreateActivity(S32 Type, S32 BeginTime, S32 EndTime, S32 CreateTime, std::string Name, std::string Contend, S32 nItemId[MAX_ACTIVITY_NUM], S32 nItemNum[MAX_ACTIVITY_NUM], S32 nExtend[MAX_ACTIVITY_NUM]);
	void TimeProcess(U32 bExit);
	void AddActivity(ActivityRef pData);
	void DelActivity(ActivityRef pData);
	ActivityRef GetActivity(S32 nType);
	std::vector<ActivityRef> GetActivityList(S32 nType);

	bool CanActivity(int type);
	ACTIVITYMULTIMAP& GetActivityMap() { return m_ActivityMap; }
	//---------DB Operator---------//
	void LoadDB();
	void UpdateDB(ActivityRef pData);
	void InsertDB(ActivityRef pData);
	void DeleteDB(S32 ID);
	void SendInitToClient(U32 nPlayerID);
	void NotifyActivity(ActivityRef pData);
	
	bool onNotify(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool onNotify1(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleActivityRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleTanGuanRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);//贪官活动

	void DeleteByID(S32 nID);
	static void Export(struct lua_State* L);
private:
	ACTIVITYMULTIMAP m_ActivityMap;
};

#define ACTIVITYMGR CActivityManager::Instance()
#endif//__ACTIVITY_MANAGER_H
