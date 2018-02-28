//#include "stdafx.h"
#include "DB_OtherLogin.h"
#include "DBContext.h"
#include "DB_ThirdActivate.h"
#include "CommLib\DBThreadManager.h"

DB_OtherLogin::DB_OtherLogin(const PollInfo& info)
:info(info)
{
}

int DB_OtherLogin::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{

			M_SQL(pDBConn, "EXEC USP_OTHERLOGINCHECK '%s', '%s'", info.name, info.pass);
			if (pDBConn->More())
			{
				char result[COMMON_STRING_LENGTH];
				strcpy_s(result, sizeof(result), pDBConn->GetString());
				if (strcmp(result, "0000") == 0)
				{
					g_Log.WriteLog("Other login check successfully, name = %s.", info.name);
					DB_ThirdActivate* pDBHandle = new DB_ThirdActivate(info);
					CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
				}
				else
				{
					g_Log.WriteError("Other login failed.[name=%s, error=%s]", info.name, result);
				}
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount Other login error.[name=%s]", info.name);
		}
	}
	return PACKET_OK;
}