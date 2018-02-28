#ifndef USER_PACKET_PROCESS_H
#define USER_PACKET_PROCESS_H

#include "Common/CommonPacket.h"
#include "ZoneServer.h"

using namespace Base;

class UserPacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

protected:
	void HandleClientLogin();
	void HandleClientLogout();
	bool HandleGamePacket(stPacketHead *pHead,int DataSize);
	bool HandleInfoPacket(stPacketHead *pHead,int DataSize);

public:
	void OnErrorPacket(const char *pInData,int InSize);
};

_inline void UserPacketProcess::HandleClientLogin()
{
	char IP[COMMON_STRING_LENGTH]="";
	char *pAddress = dynamic_cast<dtServerSocketClient *>(m_pSocket)->GetConnectedIP();
	if(pAddress)
		sStrcpy(IP,COMMON_STRING_LENGTH,pAddress,COMMON_STRING_LENGTH);

	m_pSocket->SetMaxSendBufferSize(1024*10);
	m_pSocket->SetMaxReceiveBufferSize(1024*10);

	SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(),IP,sStrlen(IP,COMMON_STRING_LENGTH)+1,true,WQ_CONNECT);

	Parent::HandleClientLogin();
}

_inline void UserPacketProcess::HandleClientLogout()
{
	m_pSocket->SetIdleTimeout(60*2);					//2分钟空闲连接超时
	m_pSocket->SetMaxSendBufferSize(MIN_PACKET_SIZE);
	m_pSocket->SetMaxReceiveBufferSize(MIN_PACKET_SIZE);
	SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(),NULL,0,false,WQ_DISCONNECT);

	Parent::HandleClientLogout();
}

_inline bool UserPacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
	SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(),pHead,iSize,true);
	return true;
}

_inline void UserPacketProcess::OnErrorPacket(const char *pInData,int iSize)
{
	const stPacketHead *pHead = reinterpret_cast<const stPacketHead *>(pInData);
	char *pAddress = dynamic_cast<dtServerSocketClient *>(m_pSocket)->GetConnectedIP();
	g_Log.WriteWarn("收到一条未处理的消息OnErrorPacket，断开这个连接 MsgId:%d IP:%s",pHead->Message,pAddress);

	dynamic_cast<dtServerSocketClient *>(m_pSocket)->PostEvent(dtServerSocket::OP_RESTART);
}

#endif
