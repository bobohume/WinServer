//#include "stdafx.h"
#include "DB_UseCode.h"
#include "DBContext.h"

DB_UseCode::DB_UseCode()
{
}

int DB_UseCode::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "EXECUTE USP_GetCodeItem %d, %d, '%s', %d, %d", accountID, playerID, sn, itemID, itemCount);
			pDBConn->Exec();
		}
		catch (CDBException& e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount add serial gift record error accountID=%s, playerID=%s, sn=%s", accountID, playerID, sn);
		}
	}
	return PACKET_OK;
}