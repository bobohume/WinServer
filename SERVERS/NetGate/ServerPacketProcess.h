#ifndef SERVER_PACKET_PROCESS_H
#define SERVER_PACKET_PROCESS_H

#include "Common/CommonPacket.h"
#include "NetGate.h"

extern DWORD g_EventNetFNListTotalFreq[END_NET_MESSAGE];

class ServerPacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

protected:
	void HandleClientLogin();
	void HandleClientLogout();
	bool HandleGamePacket(stPacketHead *pHead,int DataSize);

public:
	void dispatchSendZonePacket(int SocketHandle,stPacketHead *pHead,int iSize);
};

_inline void ServerPacketProcess::HandleClientLogin()
{
	char IP[COMMON_STRING_LENGTH]="";
	char *pAddress = dynamic_cast<dtServerSocketClient *>(m_pSocket)->GetConnectedIP();
	if(pAddress)
		sStrcpy(IP,COMMON_STRING_LENGTH,pAddress,COMMON_STRING_LENGTH);

	SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(),IP,sStrlen(IP,COMMON_STRING_LENGTH)+1,true,WQ_CONNECT);

	Parent::HandleClientLogin();
}

_inline void ServerPacketProcess::HandleClientLogout()
{
	SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(),NULL,0,false,WQ_DISCONNECT);
	Parent::HandleClientLogout();
}

_inline bool ServerPacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
	if(pHead->Message == ZONE_GATE_gamePacket)
	{
		//游戏数据包不过EventQueue了，一是减少乱序情况，二是减少消耗
		dispatchSendZonePacket(m_pSocket->GetClientId(),pHead,iSize);
		//InterlockedIncrement((volatile LONG *)&g_EventNetFNListTotalFreq[pHead->Message]);
		g_EventNetFNListTotalFreq[pHead->Message]++;

		return true;
	}
	else if(pHead->Message == MSG_PING)
	{
		g_EventNetFNListTotalFreq[pHead->Message]++;

		pHead->Message = MSG_PING_RESP;
		pHead->DestServerType = SERVICE_ZONESERVER;
		m_pSocket->Send((char *)pHead,iSize);

		return true;
	}
	else if(pHead->Message == MSG_PING_RESP)
	{
		char *pData	= (char *)(pHead) + IPacket::GetHeadSize();
		Base::BitStream RecvPacket(pData,iSize-IPacket::GetHeadSize());

		U32 clientSendTime  = 0;
		U32 gateSendTime    = 0;
		U32 zoneSendTime    = 0;

		U32 type = RecvPacket.readInt(Base::Bit32);
		clientSendTime = RecvPacket.readInt(Base::Bit32);
		gateSendTime = RecvPacket.readInt(Base::Bit32);
		zoneSendTime = RecvPacket.readInt(Base::Bit32);

		int gateRecvTime = GetTickCount();

		g_Log.WriteLog("(收到)Zone到客户端的Pong(%d) 当前时间 %d",clientSendTime,gateRecvTime);
	}

	SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(),pHead,iSize,true);
	return true;
}

#endif
