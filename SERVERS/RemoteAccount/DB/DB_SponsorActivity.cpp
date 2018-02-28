//#include "stdafx.h"
#include "DB_SponsorActivity.h"
#include "DBContext.h"
#include "CommLib\DBThreadManager.h"

DB_SponsorActivity::DB_SponsorActivity()
{
}

int DB_SponsorActivity::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{

			M_SQL(pDBConn, "EXEC USP_SponsorActivity %d, %d, %d", accountID, playerID, (int)sponsor);
			if (pDBConn->More())
			{
				char result[COMMON_STRING_LENGTH];
				strcpy_s(result, sizeof(result), pDBConn->GetString());
				if (strcmp(result, "0000")==0 || strcmp(result, "0003")==0)
				{
					char buf[128];
					Base::BitStream sendPacket(buf, sizeof(buf));
					stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, SPONSOR_ACTIVITY, playerID, SERVICE_ZONESERVER);
					sendPacket.writeInt(sponsor, Base::Bit8);
					sendPacket.writeFlag(strcmp(result, "0000")==0);
					pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
					CRemoteAccount::getInstance()->getServerSocket()->Send(socketID, sendPacket);
				}
				else
				{
					g_Log.WriteError("Sponsor activity failed.[account=%d, player=%d, error=%s]", accountID, playerID, result);
				}
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount sponsor activity error.[account=%d, player=%D]", accountID, playerID);
		}
	}
	return PACKET_OK;
}