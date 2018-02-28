#include "stdafx.h"
#include "DB_SetAccountStatus.h"
#include "DBLayer/common/DBUtility.h"
#include "DBLayer/data/TBLAccount.h"

DB_SetAccountStatus::DB_SetAccountStatus(void)
{
    m_accountId = 0;
    m_status    = 0;
}

int DB_SetAccountStatus::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
    assert(0 != pDBConn);

    g_Log.WriteLog("收到帐号[%d]的状态设置请求",m_accountId);

    TBLAccount tbl_account(0);
    tbl_account.AttachConn(pDBConn);

    if (DBERR_NONE != tbl_account.SetAccountStatus(m_accountId,m_status))
    {
        g_Log.WriteError("无法设置帐号[%d]的状态为[%d]",m_accountId,m_status);
        return true;
    }
    
    return PACKET_OK;
}