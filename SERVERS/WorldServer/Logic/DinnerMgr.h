#ifndef DINNER_MANAGER_H
#define DINNER_MANAGER_H

#include <hash_map>
#include "WINTCP/IPacket.h"
#include "CommLib/SimpleTimer.h"
#include "Common/PlayerStruct.h"
#include "BaseMgr.h"
#include "DinnerBase.h"
#include <set>
#include <deque>

/************************************************************************/
/* dinnermgr
/************************************************************************/
class CDinnerManager : public CBaseManager<DinnerRef>
{
	typedef std::unordered_map<U32, DinnerRef>		DINNER_MAP;
	typedef std::unordered_map<S32, DinnerReadyRef>	DINNER_READY_MAP;
	typedef std::deque<DinnerReadyRef>				DINNER_DEQ;
public:
	CDinnerManager();
	~CDinnerManager();

	static CDinnerManager* Instance();

	void _AddDinner(DinnerRef pData);
	void DelDinner(U32 nID);
	void ExecDinner(DinnerRef pData, S32 nTime);

	enOpError JoinDinner(S32 nPlayerID, std::string UID, S32 nType);
	void	  DisbandDinner(S32 nID);
	enOpError LeaveDinner(S32 nPlayerID);
	void	  CreateDinner(DinnerReadyRef Ready[MAX_DINNER_PLAYER]);
	void	  MatchDinner();//匹配宴会

	//---------DB Operator---------//
	void LoadDB();
	bool UpdateDB(DinnerRef pData);
	bool DeleteDB(DinnerRef pData);

	void SendInitToClient(DinnerRef pData);
	void SendInitToClient(U32 nPlayerID);
	//---------消息处理----------//
	bool HandleOpenDinnerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleJoinDinnerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	static void Export(struct lua_State* L);

	void TimeProcess(U32 bExit);

private:
	SimpleTimer<60> m_MatchTimer;
	DINNER_MAP	m_DinnerMap;//宴会队列
	DINNER_READY_MAP m_DinnerReadyMap;//准备队列
};

#define DINNERMGR CDinnerManager::Instance()
#endif//DINNER_MANAGER_H
