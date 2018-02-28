#include "stdafx.h"
#include "DB_SaveTopRank.h"
#include "../Common/TopType.h"

DB_SaveTopRank::DB_SaveTopRank(void)
{
    m_iType = 0;
    m_isEnded = false;
	m_iSaveBlobType = -1;
}

int DB_SaveTopRank::Execute(int ctxId,void* param)
{
    CDBConn* pDBConn = (CDBConn*)param;
	assert(0 != pDBConn);
    
    TBLTopRank tb_toprank(0);
    tb_toprank.AttachConn(pDBConn);
	DBError err = tb_toprank.Save(m_iType,m_toprankList,m_iSaveBlobType);	

	if(err != DBERR_NONE)
	{
		g_Log.WriteError("存储TopRank %s 数据失败，原因:%s",strTopRankType[m_iType],getDBErr(err));
	}
	else
	{
		g_Log.WriteLog("存储TopRank %s 数据成功!",strTopRankType[m_iType]);

		if (eTopType_End-1 == m_iType)
		{
			g_Log.WriteLog("存储TopRank完毕,表交换成功!");
		}
	}
    

    if (!m_isEnded)
    {
        //这里停止10秒钟,防止对数据库造成压力
        Sleep(10000);
    }

    return PACKET_OK;
}