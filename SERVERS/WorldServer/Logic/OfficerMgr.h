#ifndef OFFICER_MANAGER_H
#define OFFICER_MANAGER_H

#include <hash_map>
#include <set>
#include "WINTCP/IPacket.h"
#include "OfficerBase.h"
#include "BaseMgr.h"

struct Stats;
/************************************************************************/
/* 官职管理对象
/************************************************************************/
class COfficerManager : public CBaseManagerEx<OfficerRef, U32>
{
public:
	COfficerManager();
	virtual ~COfficerManager();

	static COfficerManager* Instance();

	OfficerRef CreateOfficer(U32 ID, U32 nPayerID);

	enOpError CanAddOfficer(U32 nPlayerID, U32 nID, std::string UID);
	enOpError Designate(U32 nPlayerID, U32 nID, std::string UID);
	enOpError AddOfficer(U32 nPlayerID, U32 nID, std::string UID);
	enOpError RemoveOfficer(U32 nPlayerID, U32 nID);
	enOpError OneKeyDesignate(U32 nPlayerID);
	
	void CaculateStats(U32 nPlayerID, OfficerRef pData, Stats& stats, bool bRemove = false);
	void UpdateToClient(U32 nPlayerID, OfficerRef pData);
	void SendInitToClient(U32 nPlayerID);
	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);

	bool UpdateDB(OfficerRef pData);
	bool InsertDB(U32 nPlayerID, OfficerRef pData);
	bool DeleteDB(U32 nPlayerID, U32 nID);

	//----------角色登录通知------//
	bool onPlayerEnter(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool onPlayerLeave(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	bool HandleDesignateOfficerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleRevokeOfficerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	static void Export(struct lua_State* L);
};

#define OFFICERMGR COfficerManager::Instance()

#endif//OFFICER_MANAGER_H
