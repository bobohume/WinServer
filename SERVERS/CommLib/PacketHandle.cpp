#include "stdafx.h"

#include "PacketHandle.h"
#include "PacketProcess.h"
#include "PacketFactoryManager.h"

#include "wintcp/ISocket.h"

PacketHandle::PacketHandle(void)
{
	m_pktProcess = 0;
}

bool PacketHandle::HandleGamePacket(stPacketHead *pHead,int iSize)
{
	if (0 == m_pktProcess)
	{
		assert(0);
		return false;
	}

	PacketBase* pkt = CREATE_PACKET(pHead->Message);

	if (0 == pkt)
	{
		g_Log.WriteWarn("未处理的网络消息[%d]",pHead->Message);
		return false;
	}

	//读取包数据
	char *pData	= (char*)(pHead) + IPacket::GetHeadSize();
	Base::BitStream RecvPacket(pData,iSize - IPacket::GetHeadSize());
	
	if (!pkt->Read(RecvPacket))
	{
		g_Log.WriteWarn("网络消息[%d],数据读取错误",pHead->Message);	
		return false;
	}
	
	m_pktProcess->SendPacket(pkt,m_pSocket->GetClientId(),pHead);
	return true;
}