#include "PacketPool.h"
#include "Common/PacketType.h"
#include "wintcp/IPacket.h"
#include <windows.h>

CPacketPool::~CPacketPool()
{
	Clear();
}
 
void CPacketPool::Clear()
{
	OLD_DO_LOCK( m_cs );
	
	stdext::hash_map<unsigned long, char*>::iterator it;

	for( it = m_packetMap.begin(); it != m_packetMap.end(); it++ )
	{
		char* pBuf = it->second;
		if( pBuf )
		{
			CMemPool::GetInstance()->Free( (MemPoolEntry)pBuf );
		}
	}

	m_packetMap.clear();
}

unsigned long CPacketPool::PostPacket( Base::BitStream& sendPacket, unsigned long UID )
{
	OLD_DO_LOCK( m_cs );
	
	stdext::hash_map<unsigned long, char*>::iterator it;
	
	it = m_packetMap.find( UID );

	if( it != m_packetMap.end() )
	{
		CMemPool::GetInstance()->Free( (MemPoolEntry)(it->second) );
		m_packetMap.erase( it );
	}

	char* pBuf = (char*)CMemPool::GetInstance()->Alloc( MAX_PACKET_SIZE );

	memcpy( pBuf, sendPacket.getBuffer(), sendPacket.getPosition() );

	m_packetMap[UID] = pBuf;

	return UID;
}

unsigned long CPacketPool::PostPacket(const char* pData,int len,unsigned long UID )
{
	OLD_DO_LOCK( m_cs );

	stdext::hash_map<unsigned long, char*>::iterator it;

	it = m_packetMap.find( UID );

	if( it != m_packetMap.end() )
	{
		CMemPool::GetInstance()->Free( (MemPoolEntry)(it->second) );
		m_packetMap.erase( it );
	}

	char* pBuf = (char*)CMemPool::GetInstance()->Alloc( MAX_PACKET_SIZE );

	memcpy( pBuf, pData, len);

	m_packetMap[UID] = pBuf;

	return UID;
}

bool CPacketPool::PeekPacket( Base::BitStream& packet, unsigned long UID )
{
	OLD_DO_LOCK( m_cs );

	stdext::hash_map<unsigned long, char*>::iterator it;

	if( UID == 0 )
		it = m_packetMap.begin();
	else
		it = m_packetMap.find( UID );

	if( it == m_packetMap.end() )
		return false;

	packet.setBuffer( it->second, MAX_PACKET_SIZE );
	packet.setPosition( sizeof( stPacketHead ) );

	return true;
}

void CPacketPool::RemovePacket( unsigned long UID )
{
	OLD_DO_LOCK( m_cs );

	stdext::hash_map<unsigned long, char*>::iterator it;

	it = m_packetMap.find( UID );

	if( it != m_packetMap.end() )
	{
		CMemPool::GetInstance()->Free( (MemPoolEntry)(it->second) );

		m_packetMap.erase( it );
	}
}

void CPacketPool::Begin()
{
	m_cs.Lock();

	m_currentIt = m_packetMap.begin();
}

bool CPacketPool::Peek(  Base::BitStream& packet )
{
	if( m_currentIt == m_packetMap.end() )
		return false;

	if (m_currentIt->second == 0)
		return false;

	packet.setBuffer( m_currentIt->second, MAX_PACKET_SIZE );
	packet.setPosition( sizeof( stPacketHead ) );

	m_currentIt++;

	return true;
}

void CPacketPool::End()
{
	m_currentIt = m_packetMap.end();

	m_cs.Unlock();
}