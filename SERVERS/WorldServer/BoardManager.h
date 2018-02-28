#ifndef BOARD_MANAGER_H
#define BOARD_MANAGER_H
/*
#include "Common/TimerMgr.h"
#include "Common/PacketType.h"
#include "Base/bitStream.h"
#include "Base/Locker.h"
#include "Common/MemGuard.h"
#include "wintcp/IPacket.h"
#include "wintcp/dtServerSocket.h"

#include "PlayerMgr.h"
#include "ServerMgr.h"

#include "LockCommon.h"
#include "ManagerBase.h"
#include "Common/BoardBase.h"

#include <hash_map>
#include <hash_set>

//Õ÷ÓÑ²Ù×÷
class CBoardManager : public CManagerBase< CBoardManager, MSG_BOARD_BEGIN, MSG_BOARD_END >
{
public:
	CBoardManager();
	virtual ~CBoardManager();

	void HandleClientListRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleClientDeleteRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleClientSaveRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleClientLoadRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleClientAddHot1( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleClientAddHot2( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );

private:
#ifdef _NTJ_UNITTEST
public:
#endif
	typedef stdext::hash_map< int , stdext::hash_set< int > > PlayerHotMap;
	typedef stdext::hash_map< int, int > PlayerTimeMap;

	PlayerHotMap mPlayerHotMap;
	PlayerTimeMap mPlayerTimeMap;
};

*/
#endif