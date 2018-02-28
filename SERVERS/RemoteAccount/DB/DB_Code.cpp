//#include "stdafx.h"
#include "DBContext.h"
#include "DB_Code.h"

// ----------------------------------------------------------------------------
// 帐号奖励的领取
int DB_Code_Draw::Execute(int ctxId,void* param)
{
	CDBConn* pDBConn = (CDBConn*)param;
	IF_ASSERT(pDBConn == NULL)
		return PACKET_ERROR; //TODO: 检查是否释放资源
	try
	{
		M_SQL(pDBConn,"UPDATE TBL_CodeHistory SET [Status] = 1 WHERE UID = '%s'", UID.c_str());
		pDBConn->Exec();
	}
	catch (CDBException &e)
	{
		g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func=" __FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
	}
	catch (...)
	{
		g_Log.WriteError("RemoteAccount code failed");
	}
	return PACKET_OK;
}