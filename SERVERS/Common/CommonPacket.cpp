#include "CommonPacket.h"
#include "wintcp/dtServerSocketClient.h"

#ifdef TCP_SERVER
#include "Common/CommonServerClient.h"
#else
#include "wintcp/ISocket.h"
#endif
#ifdef _NTJ_UNITTEST
#include "../../../../UnitTest/UnitTest/UnitCallback/UnitCallback.h"
#endif

void CommonPacket::HandleClientLogin()
{
	m_pSocket->popSendList();
}

void CommonPacket::HandleClientLogout()
{
	m_pSocket->pushSendList();
}

void CommonPacket::OnDisconnect()
{
	HandleClientLogout();

	m_loginStatus = MSG_INVALID;
}

bool CommonPacket::Send(Base::BitStream &SendPacket,int ctrlType)
{
#ifdef _NTJ_UNITTEST
	UnitTestCallback("CallBack for Mail Packet",&SendPacket);
#endif
	return Parent::Send(reinterpret_cast<char*>(SendPacket.getBuffer()),SendPacket.getPosition(),ctrlType);
}

bool CommonPacket::SendClientConnect()
{
	char Buffer[64];
	Base::BitStream SendPacket(Buffer,sizeof(Buffer));
	stPacketHead *pSendHead = BuildPacketHead(SendPacket,"MSG_CONNECT", SERVER_VERSION );
	pSendHead->PacketSize = SendPacket.getPosition()-GetHeadSize();
	return Send(SendPacket,PT_SYS);
}

bool CommonPacket::RegisterServer(char ServerType,char ServerLineId,int ServerId,int ConnectTimes,const char*ServiceIP,short Port,const char*ServiceIP2)
{
	char Buffer[64];
	Base::BitStream SendPacket(Buffer,sizeof(Buffer));
	stPacketHead *pSendHead = BuildPacketHead(SendPacket,"COMMON_RegisterRequest");
	SendPacket.writeInt(ServerType	,Base::Bit8);
	SendPacket.writeInt(ServerLineId,Base::Bit8);
	SendPacket.writeInt(ServerId	,Base::Bit32);
	SendPacket.writeInt(ConnectTimes,Base::Bit32);

	if(SendPacket.writeFlag(ServiceIP!=NULL))
	{
		unsigned long ip = inet_addr(ServiceIP);
		SendPacket.writeInt(ip		,Base::Bit32);
		SendPacket.writeInt(htons(Port)	,Base::Bit16);
	}

	if(SendPacket.writeFlag(ServiceIP2!=NULL))
	{
		unsigned long ip = inet_addr(ServiceIP2);
		SendPacket.writeInt(ip		,Base::Bit32);
	}

	pSendHead->PacketSize = SendPacket.getPosition()-GetHeadSize();
	return Send(SendPacket,PT_SYS|PT_FIRST);
}

bool CommonPacket::HandlePacket(const char *pInData,int InSize)
{
	const stPacketHead *pHead = reinterpret_cast<const stPacketHead *>(pInData);
	const char *pData	= pInData + GetHeadSize();

	if(GetMessageCodeSimple("MSG_INFO") == pHead->Message)
		return HandleInfoPacket(const_cast<stPacketHead*>(pHead),InSize);
	else if(GetMessageCodeSimple("MSG_INFO_RET") == pHead->Message)
		return HandleInfoRetPacket(const_cast<stPacketHead*>(pHead),InSize);
	else if(GetMessageCodeSimple("MSG_CONNECT") == pHead->Message)
	{
		char Buffer[256];
		Base::BitStream SendPacket(Buffer,sizeof(Buffer));
		stPacketHead *pSendHead = BuildPacketHead(SendPacket,"MSG_CONNECTED");
			
		if( pHead->Id != SERVER_VERSION )
		{
			char IP[COMMON_STRING_LENGTH]="";
			char *pAddress = dynamic_cast<dtServerSocketClient *>(m_pSocket)->GetConnectedIP();
			if(pAddress)
				sStrcpy(IP,COMMON_STRING_LENGTH,pAddress,COMMON_STRING_LENGTH);

			g_Log.WriteWarn("%s 请求连接时无效的客户端版本号:%d",IP,pHead->Id );
			return false;
		}

		m_loginStatus = MSG_CONNECTED;

		SendPacket.writeInt(m_pSocket->GetClientId(),Base::Bit32);
		pSendHead->PacketSize = SendPacket.getPosition()-GetHeadSize();
		Send(SendPacket,PT_SYS);
		return true;
	}
	else if(GetMessageCodeSimple("MSG_AUTH") == pHead->Message)
	{
		if(m_loginStatus!=MSG_CONNECTED)
		{
			g_Log.WriteWarn("请求验证时无效的链接状态");
			return false;
		}

		Base::BitStream RecvPacket(const_cast<char *>(pData),InSize-GetHeadSize());
		int sequence;
		sequence = RecvPacket.readInt(Base::Bit32);
#ifdef TCP_SERVER
		if(sequence!=m_pSocket->GetClientId())
		{
			dtServerSocketClient* socket = dynamic_cast<dtServerSocketClient *>(m_pSocket);
			if( socket )
				socket->PostEvent(dtServerSocket::OP_RESTART);
			return true;
		}
#endif
		m_loginStatus = MSG_AUTHED;

		if(RecvPacket.readFlag())
			m_pSocket->SetConnectType(ISocket::SERVER_CONNECT);
		else
			m_pSocket->SetConnectType(ISocket::CLIENT_CONNECT);
			
		HandleClientLogin();

		char Buffer[64];
		Base::BitStream SendPacket(Buffer,sizeof(Buffer));
		stPacketHead *pSendHead = BuildPacketHead(SendPacket,"MSG_AUTHED");
		pSendHead->PacketSize = SendPacket.getPosition()-GetHeadSize();
		Send(SendPacket,PT_SYS);
		return true;
	}
	else if(GetMessageCodeSimple("MSG_CONNECTED") == pHead->Message)
	{
		if(m_loginStatus!=MSG_INVALID)
		{
			g_Log.WriteWarn("收到连接时无效的链接状态");
			return false;
		}

		m_loginStatus = MSG_CONNECTED;

		Base::BitStream RecvPacket(const_cast<char *>(pData),InSize-GetHeadSize());
		m_pSocket->SetClientId(RecvPacket.readInt(Base::Bit32));

		char Buffer[64];
		Base::BitStream SendPacket(Buffer,sizeof(Buffer));
		stPacketHead *pSendHead = BuildPacketHead(SendPacket,"MSG_AUTH");
		SendPacket.writeInt(m_pSocket->GetClientId(),Base::Bit32);
		SendPacket.writeFlag(m_pSocket->GetConnectType()==ISocket::SERVER_CONNECT);
		pSendHead->PacketSize = SendPacket.getPosition()-GetHeadSize();
		Send(SendPacket,PT_SYS);
		return true;
	}
	else if(GetMessageCodeSimple("MSG_AUTHED") == pHead->Message)
	{
		if(	m_loginStatus != MSG_CONNECTED)
		{
			g_Log.WriteWarn("收到验证时无效的链接状态");
			return false;
		}

		m_loginStatus = MSG_AUTHED;

		HandleClientLogin();
		return true;
	}
	else
	{
		if(m_loginStatus!=MSG_AUTHED)
			return false;

		/*if(pHead->Message<=BEGIN_NET_MESSAGE || pHead->Message>=END_NET_MESSAGE)
		{
			return false;
		}*/

#ifdef TCP_SERVER
		if(!m_pSocket->HasClientId())
		{
			dtServerSocketClient *pSocket = dynamic_cast<dtServerSocketClient *>(m_pSocket);
			if(pSocket)
				pSocket->PostEvent(dtServerSocket::OP_RESTART);

			return true;
		}
#endif
		return HandleGamePacket(const_cast<stPacketHead*>(pHead),InSize);
	}

	return false;
}


