#include "stdafx.h"
#include "DB_UpdateAccountLogoutTime.h"
#include "DBLayer/common/DBUtility.h"
#include "DBLayer/data/TBLAccount.h"

int DB_UpdateAccountLogoutTime::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
    assert(0 != pDBConn);

    char szTime[32] = {0};
	GetDBTimeString(m_time,szTime);

    TBLAccount tb_account(0);
    tb_account.AttachConn(pDBConn);
	DBError err = tb_account.UpdateAccountLogoutTime(m_accountId,szTime);

	if(err != DBERR_NONE)
	{
		g_Log.WriteError("更新帐号登出时间出错(Account=%d, err=%s)",m_accountId,getDBErr(err));
	}

    return PACKET_OK;
}