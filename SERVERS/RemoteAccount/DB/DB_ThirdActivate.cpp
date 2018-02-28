//#include "stdafx.h"
#include "DB_ThirdActivate.h"
#include "DBContext.h"

DB_ThirdActivate::DB_ThirdActivate(const PollInfo& info)
:info(info)
{
}

int DB_ThirdActivate::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "EXEC USP_THIRDACTIVATE_SXZ '%s', '%s', '%s', 1, '%s', '%s', '%s', 'G%03d', 0"
				,info.name
				,""
				,"M"
				,""
				,""
				,""
				,CRemoteAccount::getInstance()->GetRemoteID() 
				);
			if (pDBConn->More())
			{
				char result[COMMON_STRING_LENGTH];
				strcpy_s(result, sizeof(result), pDBConn->GetString());
				if (strcmp(result, "0000")==0 || strcmp(result, "0002")==0)
				{
					g_Log.WriteLog("Third activate successfully, name = %s.", info.name);
					//CRemoteAccount::getInstance()->getPlayerManager()->onLogin(info);
				}
				else
				{
					g_Log.WriteError("Third activate failed.[name=%s]", info.name);
				}
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount Third activate error.[name=%s]", info.name);
		}
	}
	return PACKET_OK;
}