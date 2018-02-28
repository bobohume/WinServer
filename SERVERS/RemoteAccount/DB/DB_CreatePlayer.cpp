//#include "stdafx.h"
#include "DB_CreatePlayer.h"
#include "DBContext.h"
#include "DBLayer/Common/DBUtility.h"
DB_CreatePlayer::DB_CreatePlayer()
:areaId(0), account(0), socketId(0)
{
	
}

int DB_CreatePlayer::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			if(!StrSafeCheck(info.m_CreateName, 32))
				throw "";

			std::stringstream s;
			s << "EXEC USP_CREATEPLAYER_SXZ '";
			s << info.m_CreateName << "',";
			s << account;				
			pDBConn->SQLExt(s.str().c_str());
			if (pDBConn->More())
			{
				MAKE_PACKET(sendPacket, "WORLD_REMOTE_CreatePlayer", account);
				sendPacketHead->DestZoneId = pDBConn->GetInt();
				info.WriteData(&sendPacket);
				SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount create player fail.[name=%s]", info.m_CreateName);
		}
	}
	return PACKET_OK;
}