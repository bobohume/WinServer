#include "stdafx.h"
#include "DB_DeleteMail.h"
#include "DBLayer\Data\TBLMailList.h"
#include "WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"
#include "GamePlayLog.h"

DB_DeleteMail::DB_DeleteMail(void)
{
    m_socketHandle = 0;
    m_nRecver      = 0;
	m_accountId    = 0;
}

int DB_DeleteMail::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
	assert(0 != pDBConn);
    
	TBLMailList	tb_maillist(0);
    tb_maillist.AttachConn(pDBConn);
	DBError err = tb_maillist.DeletePage(m_nRecver,m_idlist);

	if(err == DBERR_NONE)
	{
		char buf[MAX_PACKET_SIZE];
		Base::BitStream sendPacket(buf,sizeof(buf));
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, "WC_DeleteMailResponse", m_accountId, SERVICE_CLIENT);

		sendPacket.writeInt( err, Base::Bit16);

		if(err == DBERR_NONE)
		{
			sendPacket.writeInt( m_idlist[0], Base::Bit32);
		}

		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send(m_socketHandle,sendPacket);

		//注：这里删除多封邮件，没有记录每封邮件详细数据
		time_t t;
		time(&t);
		stMailItem mailitem;
		mailitem.nRecver = m_nRecver;
		mailitem.nTime   = (int)t;

		for(size_t i = 0; i < m_idlist.size(); i++)
		{			
			mailitem.id = m_idlist[i];
		}		
	}

    return PACKET_OK;
}