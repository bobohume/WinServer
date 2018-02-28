#include "stdafx.h"
#include "DB_GetMailItem.h"
#include "DBLayer\Data\TBLMailList.h"
#include "WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"

DB_ResetGetMailFlag::DB_ResetGetMailFlag(void):m_nMailId(0)
{
}

int DB_ResetGetMailFlag::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
	assert(0 != pDBConn);
    
    TBLMailList	tb_maillist(0);
    tb_maillist.AttachConn(pDBConn);
	DBError err = tb_maillist.resetMailRecv(m_nMailId);
	if(err != DBERR_NONE)
	{
		g_Log.WriteWarn("发现有邮件重置标志失败的(mailid=%d)",m_nMailId);
	}
    return PACKET_OK;
}