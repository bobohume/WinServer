//#include "stdafx.h"
#include "DB_CreateLeague.h"
#include "DBContext.h"

DB_CreateLeague::DB_CreateLeague()
:areaId(0), account(0), socketId(0)
{
	strcpy_s(name, sizeof(name), "");
	strcpy_s(buffer, sizeof(buffer), "");
}

int DB_CreateLeague::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn,"EXECUTE USP_CREATELEAGUE_SXZ '%s', %d, %d", name, areaId, CRemoteAccount::getInstance()->GetRemoteID());
			if(pDBConn->More())
			{
				MAKE_PACKET(sendPacket, MSG_LEAGUE_RemoteCreate, account);
				sendPacketHead->DestZoneId = pDBConn->GetInt();
				sendPacket.writeString(name, sizeof(name));
				Base::BitStream packet(buffer, sizeof(buffer));
				sendPacket.copyFrom(&packet);
				SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount handle create league error.[name=%s]", name);
		}
	}
	return PACKET_OK;
}