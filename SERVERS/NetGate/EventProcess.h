#ifndef EVENT_FN_H
#define EVENT_FN_H

#include "wintcp/IPacket.h"

namespace EventFn
{
	void Initialize();

	bool HandleRegisterRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	bool HandleRegisterResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);

	bool HandleClientLoginResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	bool HandleClientReLoginResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	bool HandleClientLogoutResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);

	bool HandleServerChatSendMessageAll(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	bool HandleServerChatSendMessage(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);

	void SwitchSendToClient(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType=PT_GUARANTEED);
	void SwitchSendToZone(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType=PT_GUARANTEED);
	void SwitchSendToWorld(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType=PT_GUARANTEED);
	void SwitchSendToChat(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType=0);
	void SwitchSendToDataAgent( stPacketHead* pHead, Base::BitStream &switchPacket,int ctrlType=PT_GUARANTEED);
	//bool HandleWGBillingResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	//bool HandleZGBillingRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
}

extern void DumpRecvPackets(void);

#endif