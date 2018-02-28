#ifndef COMMON_SERVER_CLIENT_H
#define COMMON_SERVER_CLIENT_H

#include "WINTCP/dtServerSocketClient.h"
#include "Common/CommonPacket.h"

class CommonServerClient : public dtServerSocketClient
{
protected:
	void OnDisconnect()
	{
		(dynamic_cast<CommonPacket *>(m_pPacket))->OnDisconnect();
	}

	void OnConnect(int ErrorCode)
	{
        dtServerSocketClient::OnConnect(ErrorCode);

		if(!ErrorCode)
			(dynamic_cast<CommonPacket *>(m_pPacket))->OnConnect();
	}
};

#endif