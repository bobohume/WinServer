//#include "stdafx.h"
#include "DB_DefrayExp.h"
#include "DBContext.h"
#include "CommLib\DBThreadManager.h"

DB_DefrayExp::DB_DefrayExp()
{
}

int DB_DefrayExp::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{

			M_SQL(pDBConn, "EXEC USP_DefrayExperience %d, %d, %d", accountID, playerID, experience);
			if (pDBConn->More())
			{
				char result[COMMON_STRING_LENGTH];
				strcpy_s(result, sizeof(result), pDBConn->GetString());
				if (strcmp(result, "0000") == 0)
				{
					int curExp = pDBConn->GetInt();
					char buf[128];
					Base::BitStream sendPacket(buf, sizeof(buf));
					stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, DEFRAY_EXPERIENCE, playerID, SERVICE_ZONESERVER);
					sendPacket.writeInt(curExp, Base::Bit32);
					pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
					CRemoteAccount::getInstance()->getServerSocket()->Send(socketID, sendPacket);
				}
				else
				{
					g_Log.WriteError("Defray experience failed.[account=%d, player=%d, error=%s]", accountID, playerID, result);
				}
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount defray experience error.[account=%d, player=%D]", accountID, playerID);
		}
	}
	return PACKET_OK;
}