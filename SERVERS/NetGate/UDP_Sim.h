#ifndef UDP_SIM_H
#define UDP_SIM_H

#include "NetGate.h"
#include "encrypt/AuthCrypt.h"

class CUDP_Sim : public CUDPServer
{
public:
	CUDP_Sim(const char *ip,int nPort,int side) : CUDPServer(ip,nPort,side)
	{
		
	}

	virtual BOOL Send(const char *pBuffer,int nSize,int IP,int socketHandle)
	{
#ifdef ENABLE_ENCRPY_PACKET
		AuthCrypt::Instance()->EncryptSend((uint8*)const_cast<char*>(pBuffer),nSize);
#endif

		//Ray: 由于UDP包较多，需要减少一次数据拷贝
		static int flaxHeadSize = IPacket::GetHeadSize();
		stPacketHead *pSendHead = (stPacketHead *)(pBuffer - flaxHeadSize);
		IPacket::BuildPacketHead(pSendHead,"CLIENT_GATE_gamePacket");
		SERVER->GetUserSocket()->Send(socketHandle,(const char*)pSendHead,nSize+flaxHeadSize,PT_NEG);

		m_SendCount++;

		return TRUE;
	}

	virtual BOOL Start()
	{
		return TRUE;
	}

	virtual BOOL Stop()
	{
		return TRUE;
	}
};

#endif
