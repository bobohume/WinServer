#ifndef USER_PACKET_PROCESS_H
#define USER_PACKET_PROCESS_H

#include "Common/CommonPacket.h"
#include "PlayerMgr.h"
#include "NetGate.h"

using namespace Base;

extern stdext::hash_map<U32, U32> g_UserEventNetFNListTotalFreq;

class UserPacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

protected:
	void HandleClientLogin();
	void HandleClientLogout();
	bool HandleGamePacket(stPacketHead *pHead,int DataSize);

public:
	void OnErrorPacket(const char *pInData,int iSize);
	void dispatchSendGamePacket(int SocketHandle,stPacketHead *pHead,int iSize);
};

_inline void UserPacketProcess::HandleClientLogin()
{
	char IP[COMMON_STRING_LENGTH]="";
	char *pAddress = dynamic_cast<dtServerSocketClient *>(m_pSocket)->GetConnectedIP();
	if(pAddress)
		sStrcpy(IP,COMMON_STRING_LENGTH,pAddress,COMMON_STRING_LENGTH);

	SERVER->GetUserWorkQueue()->PostEvent(m_pSocket->GetClientId(),IP,sStrlen(IP,COMMON_STRING_LENGTH)+1,true,WQ_CONNECT);

	Parent::HandleClientLogin();
}

_inline void UserPacketProcess::HandleClientLogout()
{
	//动态缩放BUF
	m_pSocket->SetMaxSendBufferSize(MIN_PACKET_SIZE);
	m_pSocket->SetMaxReceiveBufferSize(MIN_PACKET_SIZE);
	m_pSocket->SetIdleTimeout(0);//清空空闲时间

	SERVER->GetUserWorkQueue()->PostEvent(m_pSocket->GetClientId(),NULL,0,false,WQ_DISCONNECT);
	Parent::HandleClientLogout();
}

_inline bool UserPacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
	pHead->RSV = ((dtServerSocketClient *)m_pSocket)->GetConnectedIPi();

	if(pHead->Message == GetMessageCodeSimple("CLIENT_GATE_LoginRequest")  || pHead->Message == GetMessageCodeSimple("CLIENT_GAME_LoginRequest"))
	{
		//动态缩放BUF
		m_pSocket->SetMaxSendBufferSize(MAX_PACKET_SIZE);
		m_pSocket->SetMaxReceiveBufferSize(MAX_PACKET_SIZE);
	}

	SERVER->GetUserWorkQueue()->PostEvent(m_pSocket->GetClientId(),pHead,iSize,true);
	return true;
}

_inline void UserPacketProcess::OnErrorPacket(const char *pInData,int iSize)
{
	const stPacketHead *pHead = reinterpret_cast<const stPacketHead *>(pInData);
	char *pAddress = dynamic_cast<dtServerSocketClient *>(m_pSocket)->GetConnectedIP();
	g_Log.WriteWarn("收到一条未处理的消息OnErrorPacket，断开这个连接 MsgId:%s IP:%s",SHOWMSG_EVENT(pHead->Message),pAddress);

	dynamic_cast<dtServerSocketClient *>(m_pSocket)->PostEvent(dtServerSocket::OP_RESTART);
}

#endif
