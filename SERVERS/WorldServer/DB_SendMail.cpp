#include "stdafx.h"
#include "DB_SendMail.h"
#include "DBLayer\Data\TBLMailList.h"
#include "WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"
#include "GamePlayLog.h"
#include "DBContext.h"
#include "Common/MemGuard.h"

DB_SendMail::DB_SendMail(void):
	m_socketHandle(0),
	m_nRecver(0),
	m_error(0),
	m_playerId(0)
{
}

int DB_SendMail::Execute(int ctxId,void* param)
{
	if(ctxId == DB_CONTEXT_NOMRAL)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		assert(0 != pDBConn);
    
		if (0 == m_error)
		{
			TBLMailList	tb_maillist(0);
			tb_maillist.AttachConn(pDBConn);

			m_error = tb_maillist.Save(m_nRecver,m_mailItem);

			if(m_error == DBERR_NONE)
			{
				PostLogicThread(this);
				return PACKET_NOTREMOVE;
			}
			else
			{
				if(m_error == DBERR_MAIL_RECVERNAME || m_error == DBERR_MAIL_TOOMANY)
				{
					g_Log.WriteWarn("failed to send mail [sender=%d][recver=%d][recvname=%s][title=%s][money=%d][item=%d][err=%d]",
						m_playerId,m_nRecver,m_mailItem.szRecverName,m_mailItem.title,
						m_mailItem.nMoney, m_mailItem.nItemId, m_error);
				}
				else
				{
					g_Log.WriteError("failed to send mail [sender=%d][recver=%d][recvname=%s][title=%s][money=%d][item=%d][err=%d]",
						m_playerId,m_nRecver,m_mailItem.szRecverName,m_mailItem.title,
						m_mailItem.nMoney, m_mailItem.nItemId, m_error);
				}
			}
		}
	}
	else 
	{
		if(m_nRecver)
		{
			do 
			{
				AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(m_nRecver);
				if(!pAccount)
					break;
				
				CMemGuard Buffer(4096 MEM_GUARD_PARAM);
				Base::BitStream sendPacket(Buffer.get(), 4096);
				stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,"WC_MailRecvResponse", pAccount->GetAccountId(), SERVICE_CLIENT);
				sendPacket.writeInt(m_error,Base::Bit16);
				if(m_error == DBERR_NONE)
				{
					m_mailItem.PackData(sendPacket);
				}
				pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
				SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(),sendPacket);
			} while (false);
		}
	}
    
    return PACKET_OK;
}