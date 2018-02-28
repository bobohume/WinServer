#ifndef _PACKETHANDLE_H_
#define _PACKETHANDLE_H_

#include "Common/CommonPacket.h"

class PacketProcess;

//简化网络的产生和处理
class PacketHandle : public CommonPacket
{
public:
	typedef CommonPacket Parent;

	PacketHandle(void);
	
protected:
	virtual bool HandleGamePacket(stPacketHead *pHead,int DataSize);
	virtual void HandleClientLogin()  {	Parent::HandleClientLogin();}
	virtual void HandleClientLogout() {	Parent::HandleClientLogout();}

	PacketProcess* m_pktProcess;
};

#endif /*_PACKETHANDLE_H_*/