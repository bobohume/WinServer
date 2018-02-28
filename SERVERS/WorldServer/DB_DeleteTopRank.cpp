#include "stdafx.h"
#include "DB_DeleteTopRank.h"
#include "../Common/TopType.h"

DB_DeleteTopRank::DB_DeleteTopRank(void)
{
	m_playerId = 0;
    m_type     = 0;
	m_isAllClear = false;
}

int DB_DeleteTopRank::Execute(int ctxId,void* param)
{
	CDBConn* pDBConn = (CDBConn*)param;
	assert(0 != pDBConn);

	TBLTopRank tb_toprank(0);
	tb_toprank.AttachConn(pDBConn);
    
    if (!m_isAllClear)
    {
		if (-1 == m_type && m_playerId != 0)
		{
			DBError err = tb_toprank.Delete(m_playerId);

			if(err != DBERR_NONE)
				g_Log.WriteError("删除角色%d所有排行榜数据失败，原因:%s",m_playerId,getDBErr(err));
			else
				g_Log.WriteLog("删除角色%d所有排行榜数据成功!",m_playerId);
		}
		else
		{
			if (m_uid != 0)
			{
				DBError err = tb_toprank.Delete(m_uid,m_type);

				if(err != DBERR_NONE)
					g_Log.WriteError("删除uid%I64d排行榜[%d]数据失败，原因:%s",m_uid,m_type,getDBErr(err));
				else
					g_Log.WriteLog("删除uid%I64d排行榜[%d]数据成功!",m_uid,m_type);

			}
			else if(m_playerId != 0)
			{
				DBError err = tb_toprank.Delete(m_playerId,m_type);

				if(err != DBERR_NONE)
					g_Log.WriteError("删除角色%d排行榜[%d]数据失败，原因:%s",m_playerId,m_type,getDBErr(err));
				else
					g_Log.WriteLog("删除角色%d排行榜[%d]数据成功!",m_playerId,m_type);

			}
		}
    }
    else
    {
		//删除指定的排行榜类型
        DBError err = tb_toprank.Clear(m_type);

        if(err != DBERR_NONE)
        {
            g_Log.WriteError("删除 TopRank[%d] 数据失败,原因:%s",m_type,getDBErr(err));
        }
    }

	return PACKET_OK;
}