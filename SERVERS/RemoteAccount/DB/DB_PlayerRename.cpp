//#include "stdafx.h"
#include "DB_PlayerRename.h"
#include "DBContext.h"
#include "DBLayer/Common/DBUtility.h"

DB_PlayerRename::DB_PlayerRename()
:accountID(0), socketId(0), playerID(0)
{
	memset(name, 0, sizeof(name));
}

int DB_PlayerRename::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			if (!StrSafeCheck(name, COMMON_STRING_LENGTH))
				throw "";

			std::stringstream s;
			s << "EXEC USP_PLAYERRENAME '";
			s << name << "',";
			s << accountID << ",";
			s << playerID;
			pDBConn->SQLExt(s.str().c_str());
			if (pDBConn->More())
			{
				char result[COMMON_STRING_LENGTH] = "";
				strcpy_s(result, sizeof(result), pDBConn->GetString());
				int err = atoi(result);
				MAKE_PACKET(sendPacket, MSG_PLAYER_Rename_Respone, accountID, SERVICE_WORLDSERVER, err);
				if (err == 0)
				{
					sendPacket.writeInt(playerID, Base::Bit32);
					sendPacket.writeString(name);
				}
				SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount rename player fail.[id=%d][name=%s]", playerID, name);
		}
	}
	return PACKET_OK;
}