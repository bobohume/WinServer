#include "stdafx.h"
#include "DB_DeleteAllMail.h"
#include "DBLayer\Data\TBLMailList.h"
#include "WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"

DB_DeleteAllMail::DB_DeleteAllMail(void)
{
    m_socketHandle = 0;
    m_playerId     = 0;
}

int DB_DeleteAllMail::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
	assert(0 != pDBConn);
    
	TBLMailList	tb_maillist(0);
    tb_maillist.AttachConn(pDBConn);
	std::vector<int> maillist;
	DBError err = tb_maillist.DeleteAll( m_playerId ,maillist);

	if(err != DBERR_NONE)
		return PACKET_ERROR;

	char buf[MAX_PACKET_SIZE];
/*	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, ZONE_WORLD_DeleteAllMailResponse,m_playerId,SERVICE_ZONESERVER,m_srcZoneId);

	sendPacket.writeInt(maillist.size(),Base::Bit16);

	for(size_t i=0;i<maillist.size();i++)
    {
		sendPacket.writeInt(maillist[i],Base::Bit32);
    }

	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send(m_socketHandle, sendPacket );*/
    return PACKET_OK;
}