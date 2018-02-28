/*
#include "stdafx.h"
#include "UncompressThread.h"
#include "DBLayer/common/DBUtility.h"
#include "DBLayer/data/TBLPlayer.h"
#include "WorldServer.h"
#include "ServerMgr.h"
#include "PlayerMgr.h"
#include "wintcp/dtServerSocket.h"
#include "wintcp/asyncSocket.h"
#include "../Common/TopType.h"
#include "../Common/TopType.h"

UncompressThread::UncompressThread(void)
{
	m_tarPlayerId=0;
	m_srcPlayerId=0;
	m_socketHandle=0;
	m_size=0;
    m_rank = 9999;
    
    memset(m_buff,0,sizeof(m_buff));
}

UncompressThread::~UncompressThread()
{
}

int UncompressThread::Execute(int ctxId,void* param)
{
	char* pCompBuf = new char[MAX_PLAYER_STRUCT_SIZE];

    if (0 == pCompBuf)
        return PACKET_ERROR;

	int dataSize = MAX_PLAYER_STRUCT_SIZE;
	int ret = SXZ_UNCOMPRESS(pCompBuf, &dataSize,m_buff,m_size);

	if(!ret)
	{
		Base::BitStream DataPacket(pCompBuf,dataSize);
		m_Player.ReadData( &DataPacket );

		AccountRef pa = SERVER->GetPlayerManager()->GetOnlinePlayer( m_srcPlayerId );
		if(!pa)
		{
			SAFE_DELETE_ARRAY(pCompBuf);
			return PACKET_ERROR;
		}

		char buf[MAX_PACKET_SIZE];
		Base::BitStream sendPacket(buf,sizeof(buf));
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, CW_CHECK_PLAYERITEM_ACK, pa->AccountId, SERVICE_CLIENT);

		PlayerEquip stPlayerEquip;
		stPlayerEquip.packData(sendPacket,m_Player);
        sendPacket.writeInt(m_rank, Base::Bit32);

		for (int i = 0; i < 16; ++i)
		{
			sendPacket.writeInt(m_Player.MainData.xingXiangInfo[i].ItemID, Base::Bit32);
			sendPacket.writeInt(m_Player.MainData.xingXiangInfo[i].CurAdroit, Base::Bit32);
		}

		sendPacket.writeInt(m_Player.MainData.PartData.OtherFlag, Base::Bit32);

		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( m_socketHandle, sendPacket );

		SAFE_DELETE_ARRAY(pCompBuf);
		return PACKET_OK;
	}
    
    SAFE_DELETE_ARRAY(pCompBuf);
	return PACKET_ERROR;
}
*/