#include "base/WorkQueue.h"
#include "wintcp/dtServerSocketClient.h"
#include "PlayerMgr.h"
#include "userPacketProcess.h"

extern WorldServerInfo g_worldServerInfo;
extern CMyCriticalSection g_worldServerInfo_cs;

bool UserPacketProcess::HandleInfoPacket(stPacketHead *pHead,int iSize)
{
	char Buffer[1024];
	Base::BitStream SendPacket(Buffer,1024);
	stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,"MSG_INFO_RET",SERVER->mRemoteId);

	{
		CLocker lock(g_worldServerInfo_cs);
		g_worldServerInfo.WriteData( &SendPacket );
	}

	dynamic_cast<dtServerSocketClient *>(m_pSocket)->Send(Buffer,SendPacket.getPosition());
	//m_pSocket->SetIdleTimeout(30);					//30√Îø’œ–¡¨Ω”≥¨ ±
	return true;
}

