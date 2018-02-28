#include "stdafx.h"
#include "DB_MailList.h"
#include "DBLayer\Data\TBLMailList.h"
#include "WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "Common/MemGuard.h"

DB_MailList::DB_MailList(void)
{
    m_socketHandle = 0;
    m_accountId    = 0;
	m_playerId	   = 0;
	m_recvCount	   = 0;
	m_noReadCount  = 0;
}

int DB_MailList::Execute(int ctxId,void* param)
{   
    CDBConn* pDBConn = (CDBConn*)param;
	assert(0 != pDBConn);

	std::vector<stMailItem> maillist;
	TBLMailList	tb_maillist(0);
    tb_maillist.AttachConn(pDBConn);
	DBError err = tb_maillist.LoadForPage(m_playerId, maillist, m_recvCount, m_noReadCount);

	if(err != DBERR_NONE)
		return PACKET_ERROR;

	CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
	Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_MailListResponse", m_accountId, SERVICE_CLIENT);
	sendPacket.writeInt(err, Base::Bit16);
	sendPacket.writeInt(maillist.size(), Base::Bit16);
	for(size_t i = 0; i < maillist.size(); i++)
	{
		sendPacket.writeFlag(true);
		maillist[i].PackData( sendPacket);
	}
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SERVER->GetServerSocket()->Send(m_socketHandle, sendPacket);

    return PACKET_OK;
}