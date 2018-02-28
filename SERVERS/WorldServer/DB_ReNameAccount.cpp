#include "stdafx.h"
#include "DB_ReNameAccount.h"
#include "DBLayer/common/DBUtility.h"
#include "DBLayer/data/TBLAccount.h"
#include "DBLayer/Data//TBLBase.h"
#include "DBContext.h"

DB_ReNameAccount::DB_ReNameAccount(void): m_accountId(0)
{
    memset(m_AccountName, 0, 50);
}

int DB_ReNameAccount::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
    assert(0 != pDBConn);

	M_SQL(pDBConn,"UPDATE Tbl_Account SET AccountName = '%s' WHERE AccountId = %d", m_AccountName, m_accountId);
	pDBConn->Exec();

    return PACKET_OK;
}