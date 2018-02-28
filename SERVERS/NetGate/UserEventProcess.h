#ifndef USER_EVENT_FN_H
#define USER_EVENT_FN_H

#include "wintcp/IPacket.h"

namespace UserEventFn
{
	void Initialize();
	void SwitchSendToWorld(int sockId, stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType=PT_GUARANTEED);
	void SwitchSendToChat(int sockId, stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType=0);
	void NotifyWorldClientLost( int nPlayerId,T_UID uid);

	bool HandleClientLoginRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	bool HandleClientLogoutRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);;
    
}

extern void DumpUserRecvPackets(void);

#endif