//#include "stdafx.h"
#include "DB_DeletePlayer.h"
#include "DBContext.h"

DB_DeletePlayer::DB_DeletePlayer()
:id(0)
{
}

int DB_DeletePlayer::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "UPDATE TBL_PLAYER SET [Delete]=1 WHERE PlayerID=%d", id);
			pDBConn->Exec();
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount delete player fail.[PlayerID=%d]", id);
		}
	}
	return PACKET_OK;
}