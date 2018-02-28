#include "stdafx.h"
#include "DB_QueryPlayerCount.h"
#include "DBLayer/Data/TBLAccount.h"
#include "PlayerMgr.h"
#include "ServerMgr.h"
#include "DBContext.h"

DB_QueryPlayerCount::DB_QueryPlayerCount(void)
{
    m_count = 0;
}

int DB_QueryPlayerCount::Execute(int ctxId,void* param)
{
    if (DB_CONTEXT_NOMRAL == ctxId)
    {
        CDBConn* pDBConn = (CDBConn*)param;
        assert(0 != pDBConn);

        TBLAccount tblAccount(0);
        tblAccount.AttachConn(pDBConn);
        m_count = tblAccount.QueryCreatedPlayerCount();

        PostLogicThread(this);
        return PACKET_NOTREMOVE;
    }
    else
    {
        SERVER->GetPlayerManager()->SetCreatedPlayerCount(m_count);
    }

    return PACKET_OK;
}