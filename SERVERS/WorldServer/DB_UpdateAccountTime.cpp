#include "stdafx.h"
#include "DB_UpdateAccountTime.h"
#include "DBLayer/common/DBUtility.h"
#include "DBLayer/data/TBLAccount.h"

DB_UpdateAccountTime::DB_UpdateAccountTime(void)
{
    m_accountId         = 0;
    m_lastTime          = 0;
    m_dTotalOfflineTime = 0;
    m_dTotalOnlineTime  = 0;
}

int DB_UpdateAccountTime::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
    assert(0 != pDBConn);

    TBLAccount tb_account(0);
    tb_account.AttachConn(pDBConn);

	DBError err = tb_account.UpdateLogoutTime(m_accountId,m_lastTime,m_dTotalOfflineTime,m_dTotalOnlineTime + m_lastTime);

	if(err != DBERR_NONE)
	{
		g_Log.WriteError("更新帐号登出时间出错(Account=%d, err=%s)",m_accountId,getDBErr(err));
	}

    return PACKET_OK;
}