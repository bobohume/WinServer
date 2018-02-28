#ifndef __PRINCE_MGR_H
#define __PRINCE_MGR_H
#include "PrincessBase.h"
#include "BaseMgr.h"
#include "Base/tVector.h"

class CPrinceMgr : public CBaseManagerEx<PrinceRef, std::string>
{
	typedef CBaseManagerEx<PrinceRef, std::string>   Parent;
public:
	CPrinceMgr();
	virtual ~CPrinceMgr();

	static CPrinceMgr* Instance();

	PrinceRef CreateData(U32 nPlayerID);
	DATA_MAP& GetDataMap(U32 nPlayerID);
	bool	  AddData(U32 nPlayerID, std::string PrincessUID);
	enOpError TrainPrince(U32 nPlayerID, std::string CUID, std::string UID);
	enOpError AwardPeerages(U32 nPlayerID, std::string UID);
	enOpError RemoveData(U32 nPlayerID, std::string UID);
	enOpError OpenHourse(U32 nPlayerID, std::string UID, S32 nType);
	enOpError AwardItem(U32 nPlayerID, std::string UID, std::pair<S32, S32> DropInfo[3]);
	enOpError AddActivity(U32 nPlayerID, std::string UID, S32 nItemID);
	enOpError Rename(U32 nPlayerID, std::string UID, std::string Name);
	enOpError AddMaxSlot(U32 nPlayerID);
	bool AddPrinceSlot(U32 nPlayerID, U32 Number);
	//---------Update Operator---------//
	void AddUpdateMap(U32 nPlayerID, PrinceRef pData);
	void RemoveUpdateMap(U32 nPlayerID, PrinceRef pData);
	void RemoveUpdateMap(U32 nPlayerID);
	//---------Notify Operator---------//
	void UpdateToClient(U32 nPlayerID, PrinceRef pData);
	void SendInitToClient(U32 nPlayerID);
	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);

	bool UpdatePrinceActivity(U32 nPlayerID, PrinceRef pData);
	bool UpdateDB(U32 nPlayerID, PrinceRef pData);
	bool InsertDB(U32 nPlayerID, PrinceRef pData);
	bool DeleteDB(U32 nPlayerID, std::string UID);

	void RemovePlayer(U32 nPlayerID);
	void TimeProcess(U32 bExit);
	void ReduceTimePercent(U32 nPlayerID, F32 OldReducePercent, F32 NewReducePercent);

	bool HandleClientAddActivity(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientTrain(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientRename(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientOpenHourse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientAwardItem(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientDeletePrince(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientAddMaxSlotPrince(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	static void Export(struct lua_State* L);

protected:
	PLAYER_MAP m_UpdateMap;//更新列表
};

#define PRINCEMGR CPrinceMgr::Instance()
#endif//__PRINCE_MGR_H