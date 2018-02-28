#include "base/WorkQueue.h"
#include "WinTCP/IPacket.h"
#include "Common/PacketType.h"
#include "Common/CommonClient.h"
#include "userpacketProcess.h"
#include "Common/UDP.h"
#include "encrypt/AuthCrypt.h"
#include "NetGate.h"
#include "ServerMgr.h"


void UserPacketProcess::dispatchSendGamePacket(int SocketHandle,stPacketHead *pHead,int iSize)
{
	int bufferSize = pHead->PacketSize;
	const char *pBuffer	= (char *)(pHead) + IPacket::GetHeadSize();

#ifdef ENABLE_ENCRPY_PACKET
	AuthCrypt::Instance()->DecryptRecv((uint8*)pBuffer,bufferSize);
#endif

	uRawAddress *pRawAddr;
	int switchIp;
	int switchPort;

	pRawAddr = (uRawAddress *)(&pBuffer[bufferSize-sizeof(uRawAddress)]);

	switchPort	= pRawAddr->port;
	switchIp	= pRawAddr->ip;

	//IP在ZoneServer端注册客户端连接用，PORT无用，被赋予PlayerId
	pRawAddr->ip	= pHead->RSV;
	pRawAddr->port	= pRawAddr->id;
	pRawAddr->id	= SocketHandle;


#ifdef DATA_OVER_TCP
	/*int ZoneSocketId = SERVER->GetServerManager()->GetZoneSocketId(switchPort);
	if(ZoneSocketId)
	{
		SERVER->GetServerSocket()->Send(ZoneSocketId,(const char *)pHead,iSize,PT_GUARANTEED);
	}
	else
	{
		g_Log.WriteError( "dispatchSendGamePacket 未能发现地图[%d] msg:%d, id:%d", switchPort, pHead->Message, pHead->Id );
	}*/

#else
	SERVER->GetUDPZoneSide()->Send(pBuffer,bufferSize,switchIp,switchPort);
#endif

}