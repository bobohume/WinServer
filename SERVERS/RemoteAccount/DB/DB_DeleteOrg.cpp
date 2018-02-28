//#include "stdafx.h"
#include "DB_DeleteOrg.h"
#include "DBContext.h"

DB_DeleteOrg::DB_DeleteOrg()
:id(0)
{
}

int DB_DeleteOrg::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "UPDATE TBL_ORG SET [Delete]=1 WHERE OrgID=%d", id);
			pDBConn->Exec();
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func=" __FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount delete org fail.[OrgID=%d]", id);
		}
	}
	return PACKET_OK;
}