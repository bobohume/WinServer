#ifndef _PACKETFACTORY_H
#define _PACKETFACTORY_H

#include "PacketBase.h"
#include "CommLib.h"

class PacketFactory 
{
public:	
	virtual ~PacketFactory (void) {}

	virtual PacketBase*	CreatePacket (void)           = 0;
	virtual void        DestoryPacket(PacketBase* pPacket) {SAFE_DELETE(pPacket);}
	virtual int	        GetPacketID (void) const      = 0;
};

#endif /*_PACKETFACTORY_H*/
