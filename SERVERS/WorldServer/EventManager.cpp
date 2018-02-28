/*#include "EventManager.h"
#include "WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "Common/CommonPacket.h"

CEventManager::CEventManager()
{
	memset( mEvents, 0, sizeof( stGlobalEvent ) * stGlobalEvent::MaxEventCount );

	registerEvent( ZONE_WORLD_EventNotify, &CEventManager::HandleZoneEventNotify );
}

void CEventManager::HandleZoneEventNotify( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	int id = Packet->readInt( 8 );

	mEvents[id].id = id;
	mEvents[id].time = Packet->readInt( 32 );
	mEvents[id].type = Packet->readInt( 8 );
}*/