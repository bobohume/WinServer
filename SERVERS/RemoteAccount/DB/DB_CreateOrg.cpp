//#include "stdafx.h"
#include "DB_CreateOrg.h"
#include "DBContext.h"
#include "Common/PacketType.h"
#include "DBLayer/Data/TBLBase.h"

DB_CreateOrg::DB_CreateOrg()
: socketId(0), name(""), qq(""), memo(""), autoAcceptJoin(false)
{
	memset(buffer, 0, sizeof(buffer));
}

int DB_CreateOrg::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "EXECUTE USP_CREATEORG '%s'", name.c_str());
			if (pDBConn->More())
			{
				MAKE_PACKET(sendPacket, "RW_CreateOrg", account);
				sendPacketHead->DestZoneId = pDBConn->GetInt();
				sendPacket.writeString(name);
				sendPacket.writeString(qq);
				sendPacket.writeString(memo);
				sendPacket.writeFlag(autoAcceptJoin);
				Base::BitStream packet(buffer, sizeof(buffer));
				sendPacket.copyFrom(&packet);
				SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func=" __FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount handle create org error.[name=%s]", name);
		}
	}
	return PACKET_OK;
}