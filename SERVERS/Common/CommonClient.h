#ifndef COMMON_CLIENT_H
#define COMMON_CLIENT_H

#include "WINTCP/AsyncSocket.h"
#include "Common/CommonPacket.h"

typedef void*(*CREATE_FN)();

template<class T1=CommonPacket>
class CommonClient : public CAsyncSocket2
{
	CREATE_FN m_pPacketCreator;

	typedef CAsyncSocket2 Parent;

public:
	CommonClient(CREATE_FN PacketFN):
		m_pDefaultHandler(0)
	{
		m_pPacketCreator = PacketFN;
	}

	CommonClient():
		m_pDefaultHandler(0)
	{
		m_pPacketCreator = NULL;
	}

	bool Initialize(const char *ip,int nPort,void *Param=NULL)
	{
		Parent::Initialize(ip,nPort,Param);

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

		pPacket->BindSocket(this);
		BindPacket(pPacket);

		SetMaxSendBufferSize(MAX_PACKET_SIZE);
		SetMaxReceiveBufferSize(MAX_PACKET_SIZE);

		return true;
	}

	void SetDefaultHandler(T1* pHandler)
	{
		m_pDefaultHandler = pHandler;
	}
protected:
	void OnDisconnect()
	{
		(dynamic_cast<CommonPacket *>(m_pPacket))->OnDisconnect();
	}

	void OnConnect(int nErrorCode)
	{
		(dynamic_cast<CommonPacket *>(m_pPacket))->OnConnect(nErrorCode);
	}

	T1*	m_pDefaultHandler;
};

#endif