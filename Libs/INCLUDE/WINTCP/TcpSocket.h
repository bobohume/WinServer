#ifndef _TCP_SOCKET_H
#define _TCP_SOCKET_H

#include "ISocket.h"

class IPacket;		//Packet½Ó¿ÚÀà
class CTcpSocket : public ISocket
{
public:
	typedef ISocket Parent;

protected:
	IPacket*	m_pPacket;
	int			m_pPacketHeadSize;	

	char *	m_pInBuffer;
	int		m_iHalfSize;
	bool	m_bHalf;

protected:
	virtual void OnClear();
	virtual void OnReceive(const char *pBuf, int nBufferSize);

public:
	CTcpSocket();
	~CTcpSocket(void);

	virtual void SetMaxReceiveBufferSize(int MaxReceiveSize);

	virtual void BindPacket(IPacket* pPacket);
	virtual IPacket* const GetPacket(){return m_pPacket;}

protected:
	virtual void OnHandleAPacket(const char* pInBuffer);
};

#endif