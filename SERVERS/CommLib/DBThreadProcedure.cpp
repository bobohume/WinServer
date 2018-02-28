#include "stdafx.h"

#include "DBThreadProcedure.h"

int DBThreadProcedure::run(void)
{
    while(!m_isEnded)
    {
        m_pktProcess.run(m_pDBConn);
        
        Sleep(1);
    }
    
    //处理完最后的数据
    m_pktProcess.run(m_pDBConn);

    return 0;
}