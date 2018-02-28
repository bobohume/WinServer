#ifndef COMMON_SERVER_H
#define COMMON_SERVER_H

#include "WINTCP/dtServerSocket.h"
#include "Common/CommonServerClient.h"
#include "Common/CommonPacket.h"

typedef void*(*CREATE_FN)();

extern void *Create_CommonServerClient();

template<class T1=CommonPacket,class T2=CommonServerClient>
class CommonServer : public dtServerSocket
{
	CREATE_FN m_pSocketCreator;
	CREATE_FN m_pPacketCreator;

public:
	CommonServer(CREATE_FN PacketFN,CREATE_FN SocketFN=Create_CommonServerClient):
		m_pDefaultHandler(0)
	{
		m_pSocketCreator = SocketFN;
		m_pPacketCreator = PacketFN;
	}
	
	CommonServer():
		m_pDefaultHandler(0)
	{
		m_pSocketCreator = NULL;
		m_pPacketCreator = NULL;
	}

	bool Initialize(const char* ip, int port,void *param=NULL)
	{
		SetMaxSendBufferSize(MAX_PACKET_SIZE);
		SetMaxReceiveBufferSize(MAX_PACKET_SIZE);
		
		return dtServerSocket::Initialize(ip,port,param);
	}

	void SetDefaultHandler(T1* pHandler)
	{
		m_pDefaultHandler = pHandler;
	}
protected:
	ISocket *LoadClient()
	{
		dtServerSocketClient *pSocketClient = NULL;
		if(m_pSocketCreator)
			pSocketClient = reinterpret_cast<dtServerSocketClient *> (m_pSocketCreator());
		else
			pSocketClient = dynamic_cast<dtServerSocketClient *>(new T2);

		IPacket *pPacket = NULL;

		if (0 == m_pDefaultHandler)
		{
			if(m_pPacketCreator)
				pPacket = reinterpret_cast<IPacket *>(m_pPacketCreator());
			else
				pPacket = dynamic_cast<IPacket *>(new T1);
		}
		else
		{
			pPacket = m_pDefaultHandler;
		}

		pPacket->BindSocket(pSocketClient);
		pSocketClient->SetMaxSendBufferSize(m_MaxSendBufferSize);
		pSocketClient->SetMaxReceiveBufferSize(m_MaxReceiveBufferSize);
		pSocketClient->BindPacket(pPacket);

		return pSocketClient;
	}

	T1* m_pDefaultHandler;
};

#endif