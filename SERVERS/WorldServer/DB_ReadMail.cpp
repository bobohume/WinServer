#include "stdafx.h"
#include "DB_ReadMail.h"
#include "DBLayer\Data\TBLMailList.h"
#include "WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"
#include "MailManager.h"
#include "Common/MemGuard.h"

DB_ReadMail::DB_ReadMail(void)
{
    m_socketHandle = 0;
    m_mailId       = 0;
    m_playerId     = 0;
	m_accountId	   = 0;
}

int DB_ReadMail::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
	assert(0 != pDBConn);
    
    stMailItem mailItem;

	TBLMailList	tb_maillist(0);
    tb_maillist.AttachConn(pDBConn);
	DBError err =tb_maillist.UpdateReaded(m_mailId);
 	err = tb_maillist.Load(m_mailId,mailItem,true);

	//文本邮件看完就删除掉
	if(mailItem.nItemId == 0 && mailItem.nMoney == 0 && mailItem.nMoney == 0)
	{
		if(err == DBERR_NONE)
		{
			MAILMGR->deleteMail(m_playerId, m_mailId, m_accountId, m_socketHandle);
		}
	}
	else
	{
		CMemGuard Buffer(4096 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 4096);
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, "WC_ReadMailResponse", m_accountId, SERVICE_CLIENT);
		sendPacket.writeInt(err, Base::Bit16);
		if(err == DBERR_NONE)
		{
			mailItem.PackData(sendPacket,false);
		}
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( m_socketHandle, sendPacket );
	}

    return PACKET_OK;
}