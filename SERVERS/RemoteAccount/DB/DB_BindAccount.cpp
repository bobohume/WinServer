//#include "stdafx.h"
#include "DB_BindAccount.h"
#include "DBContext.h"
#include "../RemoteAccount.h"
#include "DBLayer/Common/DBUtility.h"
DB_BindAccount::DB_BindAccount()
:account(0), m_Error(0)
{
	memset(m_AccountName, 0, sizeof(m_AccountName));
	memset(m_Password, 0, sizeof(m_Password));
}

int DB_BindAccount::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			if(!StrSafeCheck(m_AccountName, 50))
				throw "";

			if(!StrSafeCheck(m_Password, 32))
				throw "";

			std::stringstream s;
			s << "EXEC USP_BINDACCOUNT_SXZ '";
			s << m_AccountName << "','";
			s << m_Password    << "',";
			s << account;				
			pDBConn->SQLExt(s.str().c_str());
			if (pDBConn->More())
			{
				m_Error = pDBConn->GetInt();
				MAKE_PACKET(sendPacket, "CLIENT_Bind_AccountResponse", account, SERVICE_WORLDSERVER);
				sendPacketHead->DestZoneId = m_Error;
				sendPacket.writeString(m_AccountName, 50);
				sendPacket.writeString(m_Password, 32);
				sendPacketHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();

				int socketHandle = SERVER->GetWorldSocket(SERVER->GetAreaaId());
				if(socketHandle > 0)
					SERVER->getServerSocket()->Send(socketHandle, sendPacket);
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount create player fail.[name=%s]", m_AccountName);
		}
	}
	return PACKET_OK;
}