#ifndef COMMON_PACKET_H
#define COMMON_PACKET_H

#include <cstring>
#include "base/SafeString.h"
#include "WINTCP/IPacket.h"
#include "Common/MemGuard.h"
#include "Common/Packettype.h"
#include "wintcp/ISocket.h"

class CommonPacket : public IPacket
{
	typedef IPacket Parent;
protected:
#ifdef _NTJ_UNITTEST
public:
#endif
	U32 m_loginStatus;
public:
	CommonPacket()
	{
		m_loginStatus = MSG_INVALID;
	}

protected:
	virtual bool HandleGamePacket(stPacketHead *pHead,int DataSize) = 0;
	virtual void HandleClientLogin();
	virtual void HandleClientLogout();
	virtual bool HandleInfoPacket(stPacketHead *pHead,int DataSize) {return false;}
	virtual bool HandleInfoRetPacket(stPacketHead *pHead,int DataSize){return false;}

public:
	virtual void OnConnect(int Error=0){};
	virtual void OnConnectFailed() {};
	virtual void OnDisconnect();
	virtual bool HandlePacket(const char *pInData,int InSize);

	bool SendClientConnect();
	bool RegisterServer(char ServerType,char ServerLineId,int ServerId,int ConnectTimes,const char*ServiceIP=NULL,short port=0,const char*ServiceIP2 = 0);
	virtual bool Send(Base::BitStream &SendPacket,int ctrlType=PT_GUARANTEED);
	virtual void clearSendList(){};

	U32 getLoginStatus(){return m_loginStatus;}
};

#endif

