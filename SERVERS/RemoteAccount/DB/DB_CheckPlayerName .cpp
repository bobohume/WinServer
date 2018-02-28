//#include "stdafx.h"
#include "DB_CheckPlayerName.h"
#include "DBContext.h"
#include "DBLayer/Common/DBUtility.h"
DB_CheckPlayerName::DB_CheckPlayerName()
:areaId(0), account(0), socketId(0)
{
	memset(m_CreateName, 0, COMMON_STRING_LENGTH);	
}

int DB_CheckPlayerName::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			if(!StrSafeCheck(m_CreateName, 32))
				throw "";

			std::stringstream s;
			s << "EXEC USP_CHECKPLAYERRENAME '";
			s << m_CreateName << "',";
			s << account;				
			pDBConn->SQLExt(s.str().c_str());
			if (pDBConn->More())
			{
				MAKE_PACKET(sendPacket, "WORLD_REMOTE_AutoPlayerName", account);
				sendPacketHead->DestZoneId = pDBConn->GetInt();
				sendPacket.writeString(m_CreateName, COMMON_STRING_LENGTH);
				sendPacketHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();
				SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount auto player name  fail.[name=%s]", m_CreateName);
		}
	}
	return PACKET_OK;
}