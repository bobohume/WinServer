#ifndef PACKET_POOL_H
#define PACKET_POOL_H

#include <hash_map>
#include "base/bitStream.h"
#include "base/Locker.h"
#include "base/memPool.h"

class CPacketPool
{
public:

	virtual ~CPacketPool();

	unsigned long PostPacket( Base::BitStream& sendPacket, unsigned long UID );

	unsigned long PostPacket(const char* pData,int len,unsigned long UID );

	bool PeekPacket( Base::BitStream& packet, unsigned long UID = 0 );
	void RemovePacket( unsigned long UID );

	void Begin();
	bool Peek(  Base::BitStream& packet );
	void End();

	void Clear();
	CMyCriticalSection m_cs;

private:
	stdext::hash_map< unsigned long, char* > m_packetMap;
	stdext::hash_map< unsigned long, char* >::iterator m_currentIt;
};


#endif