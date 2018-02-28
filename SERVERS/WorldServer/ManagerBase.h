#ifndef MANAGER_BASE_H
#define MANAGER_BASE_H

#include "Common/PacketType.h"
#include "wintcp/IPacket.h"
#include <memory>

class Player;

typedef std::tr1::shared_ptr<Player> AccountRef;

template< typename _Ty>
class CManagerBase : public MessageCode
{

public:
   CManagerBase()
   {
   }

protected:
	typedef bool (_Ty::*EventFunction)(int ,stPacketHead *,Base::BitStream *);
	void registerEvent( const char* messageName, EventFunction funct )
	{
		Register(messageName, (_Ty *)(this), funct);
	}

public:
	void HandleEvent(AccountRef pAccount,int SocketHandle,stPacketHead *pHead,Base::BitStream& Packet)
	{ 
		//if (0 == pAccount)
		//	return;
		Trigger(SocketHandle, pHead, Packet);
	}
};

#ifndef DECLARE_EVENT_FUNCTION
#define DECLARE_EVENT_FUNCTION(p) bool p(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
#endif

#ifndef EVENT_FUNCTION
#define EVENT_FUNCTION(p) p(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
#endif


#endif