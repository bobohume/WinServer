//#include "stdafx.h"
#include "DB_DeleteLeague.h"
#include "DBContext.h"

DB_DeleteLeague::DB_DeleteLeague()
:id(0)
{
}

int DB_DeleteLeague::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "UPDATE TBL_LEAGUE SET [Delete]=1 WHERE LeagueID=%d", id);
			pDBConn->Exec();
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount delete league fail.[LeagueID=%d]", id);
		}
	}
	return PACKET_OK;
}