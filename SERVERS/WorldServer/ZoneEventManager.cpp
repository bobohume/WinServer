/*#include "ZoneEventManager.h"
#include "wintcp/dtServerSocket.h"
#include "ServerMgr.h"
#include "WorldServer.h"
#include "../Common/TopType.h"
#include "PlayerMgr.h"

CZoneEventManager::~CZoneEventManager(void)
{
}

CZoneEventManager::CZoneEventManager(void)
{
	//注册网络事件处理
	registerEvent( ZONE_WORLD_SetSeverEvent, &CZoneEventManager::HandleSetSeverEvent );
	registerEvent( CLIENT_WORLD_GetSeverEvent, &CZoneEventManager::HandleClientGetSeverEvent );
}

void CZoneEventManager::HandleSetSeverEvent(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::ZoneEvent);

	int Id = pHead->Id;

	if ( Id < 1 || Id > 255 )
		return;

	m_eventData[Id-1].id = Id;
	m_eventData[Id-1].LineId = Packet->readInt(32);
	Packet->readBits( Base::Bit64, &(m_eventData[Id-1].Time) );
	m_eventData[Id-1].Type = Packet->readInt(32);
}

void CZoneEventManager::HandleClientGetSeverEvent(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::ZoneEvent | Lock::Player  );

	int playerId = pHead->Id;
	int count = 0;

	AccountRef pa = SERVER->GetPlayerManager()->GetOnlinePlayer( playerId );

	if(!pa)
    {
		return;
    }

	char buf[5120];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, CLIENT_WORLD_GetSeverEventResponse, pa->AccountId, SERVICE_CLIENT );

	for ( int i = 0; i < MAX_EVENT; i++)
	{
		if ( m_eventData[i].id == -1 )
			continue;

		count++;
	}
	
	sendPacket.writeInt( count, 32 );

	for ( int i = 0; i < MAX_EVENT; i++)
	{
		if ( m_eventData[i].id == -1 )
			continue;

		sendPacket.writeInt( m_eventData[i].id, 32);
		sendPacket.writeBits( Base::Bit64, &(m_eventData[i].Time) );
		sendPacket.writeInt( m_eventData[i].Type, 32);
	}
	
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
    SERVER->GetServerSocket()->Send( pa->GetGateSocketId(), sendPacket );
}*/

