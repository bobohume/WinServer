#include "DB_CreateAccount.h"
#include "DBContext.h"
#include "DBLayer/Common/DBUtility.h"

DB_CreateAccount::DB_CreateAccount():AccountId(0), error(ERR_NONE), socketId(0), AccountSocketId(0), bApple(false)
{
	memset(AccountName, 0, sizeof(AccountName));
	memset(Password, 0, sizeof(Password));
}

int DB_CreateAccount::Execute(int ctxId, void* param)
{
	if(DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR;

		unsigned long createTime = _time32(NULL);
		char szCreateTime[32] = {0};
		GetDBTimeString(createTime, szCreateTime);

		try
		{
			if(bApple)
			{
				M_SQL(pDBConn, "USP_ACTIVATEGAMEAPPLE'%s', '%s', '%s'", AccountName, Password, szCreateTime);
			}
			else
			{
				M_SQL(pDBConn, "USP_ACTIVATEGAME'%s', '%s', '%s'", AccountName, Password, szCreateTime);
			}

			if(pDBConn->More())
			{
				error = atoi(pDBConn->GetString());
				AccountId = pDBConn->GetInt();
				if(error == ERR_AccountEXISTS)
				{
					throw ExceptionResult(DBERR_ACCOUNT_IDEXISTS);
				}
				else if(error == ERR_AccountInsertDBERROR)
				{
					throw ExceptionResult(DBERR_UNKNOWERR);
				}
				else
				{
					CMemGuard Buffer(256 MEM_GUARD_PARAM);
					Base::BitStream sendPacket(Buffer.get(), 256);
					stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "RA_RegisterAck", AccountId, SERVICE_ACCOUNTSERVER, AccountSocketId);
					sendPacket.writeString(AccountName, sizeof(AccountName));
					sendPacket.writeString(Password, sizeof(Password));
					pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
					dtServerSocket* pServer = CRemoteAccount::getInstance()->getServerSocket();
					if (0 != pServer)
						pServer->Send(socketId,sendPacket);

				}
			}
		}
		catch (CDBException e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);
		}

		catch (...)
		{
			g_Log.WriteError("RemoteAccount handle create account error.[AccountId=%d]", AccountId);
		}
	}

	return PACKET_OK;
}