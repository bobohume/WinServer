//#include "stdafx.h"
#include "DB_Alipay.h"
#include "DBContext.h"
#include "..\Soap_Alipay.h"

DB_Alipay::DB_Alipay(const AlipayInfo& info)
:info(info)
{
}

int DB_Alipay::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "EXEC USP_ALIPAYORDER_SXZ %d, %d, %d, '%s', '%s', %d",
				info.areaId, CRemoteAccount::getInstance()->GetRemoteID(),
				info.accountID, info.accountName, info.IP, info.playerID);
			if (pDBConn->More())
			{
				info.orderID = pDBConn->GetInt();
				if (info.orderID)
				{
					Soap_Alipay* pHandler = new Soap_Alipay(info);
					CRemoteAccount::getInstance()->GetThreadPool()->SendPacket(pHandler);
				}
				else
				{
					g_Log.WriteError("Fail to get alipay order.[name=%s]", info.accountName);
				}
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount get alipay order error.[name=%s]", info.accountName);
		}
	}
	return PACKET_OK;
}