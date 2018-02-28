#include "stdafx.h"
#include "DB_UpdatePlayerDeleteTime.h"
#include "DBLayer/common/DBUtility.h"
#include "DBLayer/data/TblPlayer.h"

DB_UpdatePlayerDeleteTime::DB_UpdatePlayerDeleteTime(void)
{
    m_playerId = 0;
    m_time     = 0;
}

int DB_UpdatePlayerDeleteTime::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
    assert(0 != pDBConn);

    TBLPlayer tbl_player(0);
    tbl_player.AttachConn(pDBConn);

    if (DBERR_NONE != tbl_player.SetPlayerDeleteTime(m_playerId,m_time))
    {
        return PACKET_ERROR;
    }
    
    return PACKET_OK;
}