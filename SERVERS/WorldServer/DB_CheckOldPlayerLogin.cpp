#include "stdafx.h"
#include "DB_CheckOldPlayerLogin.h"
#include "DBLayer/common/DBUtility.h"
#include "DBLayer/data/TBLAccount.h"

DB_CheckOldPlayerLogin::DB_CheckOldPlayerLogin(void)
{
    m_accountId = 0;
    m_playerId  = 0;
}

int DB_CheckOldPlayerLogin::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
    assert(0 != pDBConn);

    TBLAccount tb_account(0);
    tb_account.AttachConn(pDBConn);

    DBError err_chk = tb_account.CheckOldPlayerLogin(m_accountId,m_playerId,m_playerName.c_str());

    if(err_chk != DBERR_NONE)
    {
        g_Log.WriteError("ÀÏÍæ¼ÒµÇÂ¼½±ÀøÊ§°Ü[AccountId = %d, PlayerName=%s]",m_accountId,m_playerName.c_str());
    }

    return PACKET_OK;
}