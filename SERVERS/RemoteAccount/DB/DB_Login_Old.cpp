#include "DB_Login.h"
#include "DBContext.h"
#include "DBLayer/Common/DBUtility.h"
#include "DB_CreateAccount.h"
#include "CommLib/DBThreadManager.h"

DB_Login::DB_Login(): socketId(0), AccountSocketId(0), AccountId(0)
{
	memset(AccountName, 0, sizeof(AccountName));
	memset(Password, 0, sizeof(Password));
}

int DB_Login::Execute(int ctxId, void* param)
{
	if(DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR;

		unsigned long createTime = _time32(NULL);
		char szCreateTime[32] = {0};
		GetDBTimeString(createTime, szCreateTime);
		char result[32]  = "";
		ERROR_CODE error = UNKNOW_ERROR;

		try
		{
			M_SQL(pDBConn, "USP_LOGIN_SXZ '%s', '%s', '%s'", AccountName, Password, result);
			if (pDBConn->More())
			{
				strcpy_s(result, sizeof(result), pDBConn->GetString());
				if(strcmp(result, "0000") == 000 )
				{
					AccountId = pDBConn->GetInt();
					CMemGuard Buffer(256 MEM_GUARD_PARAM);
					Base::BitStream sendPacket(Buffer.get(), 256);
					stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "RA_LoginAck", AccountSocketId, SERVICE_ACCOUNTSERVER);
					sendPacket.writeInt(AccountId, Base::Bit32);
					sendPacket.writeString(AccountName, sizeof(AccountName));
					sendPacket.writeString(Password, sizeof(Password));
					pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);

					dtServerSocket* pServer =  CRemoteAccount::getInstance()->getServerSocket();
					if(pServer)
						pServer->Send(socketId, sendPacket);

				}
				else if (strcmp(result, "0001") == 0)
				{
					//×¢²á
					DB_CreateAccount* pDBHandle = new DB_CreateAccount();
					pDBHandle->socketId = socketId;
					pDBHandle->AccountSocketId = AccountSocketId;
					pDBHandle->bApple = false;

					dMemcpy(pDBHandle->AccountName, AccountName, sizeof(AccountName));
					dMemcpy(pDBHandle->Password, Password, sizeof(Password));

					CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
					//g_Log.WriteLog("ÕÊºÅ[%s]µÇÂ¼Ê§°Ü,ÕÊºÅ±»¶³½á",AccountName);
					//error = ACCOUNT_FREEZE;
				}
				else
				{
					g_Log.WriteLog("ÕÊºÅ[%d]µÇÂ¼Ê§°Ü,ÕÊºÅID´íÎó",AccountId);
					error = ACCOUNT_ID_ERROR;
				}	
			}
		}
		catch (CDBException e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);
		}

		catch (...)
		{
			g_Log.WriteError("RemoteAccount handle create account error.[AccountName=%s]", AccountName);
		}
	}

	return PACKET_OK;
}