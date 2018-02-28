#ifndef EVENT_FN_H
#define EVENT_FN_H

#include "wintcp/IPacket.h"
#include <hash_set>
//typedef stdext::hash_map<int, Player> IdAccountMap;

//extern IdAccountMap g_loginPending;



/************************************************************************/
/* 全局消息的响应
/************************************************************************/
namespace EventFn
{
	void Initialize();

	//Net Event
	bool HandleRegisterRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	bool HandleClientLoginAccountRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);

	bool HandleClientWorldSavePlayerRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
    bool HandleGateClientLost( int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet );
    bool HandleGateWorldAccountRelink( int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet );
	bool HandleSelectPlayerRequest( int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandleCreatePlayerRequest( int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandleAutoPlayerNameRequest( int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandleDeletePlayerRequest( int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandleRWCreatePlayerRespond(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandleRWAutoPlayerNameRespond(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandleLoginGameRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandleClientLogout(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandlePlayerGateReLogin(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandlePlayerBindAccount(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);

	// PlayerEvent
	bool HandleClientPlayerDataTransRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream* pPacket );

	bool HandleClientWorldItemRequest( int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet );
	bool HandleZonePlayerLogin( int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet );

	void SwitchSendToRemote(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType = PT_GUARANTEED);
	void SwitchSendToAccount(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType = PT_GUARANTEED);
	//void SwitchSendToZone(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType = PT_GUARANTEED);
	void SwitchSendToClient(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType = PT_GUARANTEED);

	bool HandlePlayerLoginGate(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandlePlayerLogoutGate(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandleRWLoginPlayerResp(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
    bool HandleGetLinePlayerCount(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);
	bool HandleAlipayCharge(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet);

	bool HandlePlayerPurchaseResponse(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);//内购处理
	bool HandleRechargeResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);//内购处理
	bool HandleRecharge1Response(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);//内购处理
	bool HandleCodeResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);//内购处理
}

extern void DumpRecvPackets(void);

#endif
