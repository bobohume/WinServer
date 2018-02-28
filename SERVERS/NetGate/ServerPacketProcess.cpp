#include "base/WorkQueue.h"
#include "WinTCP/IPacket.h"
#include "Common/PacketType.h"
#include "Common/CommonClient.h"
#include "serverpacketProcess.h"
#include "Common/UDP.h"
#include "encrypt/AuthCrypt.h"

void ServerPacketProcess::dispatchSendZonePacket(int SocketHandle,stPacketHead *pHead,int iSize)
{
	int nSize = pHead->PacketSize;
	char *pBuffer = (char *)pHead+IPacket::GetHeadSize();

	uRawAddress *pRawAddr = (uRawAddress *)(&pBuffer[nSize-sizeof(uRawAddress)]);
	int switchId = pRawAddr->id;

	pRawAddr->id = pHead->Id;

#ifdef ENABLE_ENCRPY_PACKET
	AuthCrypt::Instance()->EncryptSend((uint8*)const_cast<char*>(pBuffer),nSize);
#endif

	SERVER->GetUserSocket()->Send(switchId,(const char*)pHead,iSize,PT_NEG);
}