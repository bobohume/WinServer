//#include "stdafx.h"
#include "DB_ChangeAccountFlag.h"
#include "DBContext.h"
#include "DBLayer/Common/DBUtility.h"

DB_ChangeAccountFlag::DB_ChangeAccountFlag()
:accountID(0)
{

}

int DB_ChangeAccountFlag::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源
		
		//这里这么写不是很规范不过现在就这么写，之后修改很简单，就这么着吧
#ifdef _DEBUG
		g_Log.WriteLog("收到World请求修改数据库标识...");
#endif
		try
		{
			M_SQL(pDBConn,"EXEC USP_Change_AccountFlag %d", accountID);
			pDBConn->Exec();
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount Change Account Flag fail...[AccountId=%d]", accountID);
		}
	}
	return PACKET_OK;
}