#include "stdafx.h"
#include "DB_TryGetMailItem.h"
#include "DBLayer\Data\TBLMailList.h"
#include "WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"
#include "Common/MemGuard.h"
#include "MailManager.h"
#include "DBContext.h"
#include "Logic/PlayerInfoMgr.h"
#include "Logic/ItemMgr.h"

DB_TryGetMailItem::DB_TryGetMailItem(void)
{
    m_socketHandle = 0;
    m_nMailId      = 0;
    m_nPlayerId    = 0;
	m_accountId    = 0;
}

int DB_TryGetMailItem::Execute(int ctxId,void* param)
{
	if(ctxId == DB_CONTEXT_NOMRAL)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		assert(0 != pDBConn);

		TBLMailList	tb_maillist(0);
		tb_maillist.AttachConn(pDBConn);
		DBError err = tb_maillist.Load(m_nMailId,m_mailItem,true);

		if(err != DBERR_NONE)
			return PACKET_ERROR;

		if( m_mailItem.nRecver != m_nPlayerId )  //不是这个人的直接忽略
			return PACKET_ERROR;

		err = tb_maillist.setMailRecv(m_nMailId);

		if(err != DBERR_NONE)
			return PACKET_ERROR;

		PostLogicThread(this);
		return PACKET_NOTREMOVE;
	}
	else
	{
		do
		{
			AccountRef pAcccount = SERVER->GetPlayerManager()->GetOnlinePlayer(m_nPlayerId);
			if (!pAcccount)
				break;

			int money = m_mailItem.nMoney;
			int nItemId[MAX_MAIL_ITEM_NUM] = { m_mailItem.nItemId[0], m_mailItem.nItemId[1], m_mailItem.nItemId[2], m_mailItem.nItemId[3] };
			int nItemCount[MAX_MAIL_ITEM_NUM] = { m_mailItem.nItemCount[0],m_mailItem.nItemCount[1],m_mailItem.nItemCount[2],m_mailItem.nItemCount[3] };

			//领取邮件物品
			//if (m_mailItem.nMoney > 0)
			//	PLAYERINFOMGR->AddMoney(m_nPlayerId, m_mailItem.nMoney);

			for(auto i = 0; i < MAX_MAIL_ITEM_NUM; ++i)
			{
				if (nItemId[i] == 1)
					SERVER->GetPlayerManager()->AddGold(m_nPlayerId, nItemCount[i]);
				else if(nItemId[i] > 0)
					ITEMMGR->AddItem(m_nPlayerId, nItemId[i], nItemCount[i]);
			}

			U32 nError = 0;
			CMemGuard Buffer(1024 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 1024);
			stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, "WC_GetMailItemsResponse", m_accountId, SERVICE_CLIENT);
			sendPacket.writeInt(nError, Base::Bit16);
			sendPacket.writeInt(MAX_MAIL_ITEM_NUM, Base::Bit16);
			for (auto i = 0; i < MAX_MAIL_ITEM_NUM; ++i)
			{
				sendPacket.writeFlag(true);
				sendPacket.writeInt(nItemId[i], Base::Bit32);
				sendPacket.writeInt(nItemCount[i], Base::Bit16);
			}
			sendPacket.writeInt( m_mailItem.id, Base::Bit32);
			pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
			SERVER->GetServerSocket()->Send( m_socketHandle, sendPacket );

			//删除邮件
			{
				MAILMGR->deleteMail(m_nPlayerId, m_nMailId, m_accountId, m_socketHandle);
			}
		} while (false);
	}

    return PACKET_OK;
}