#include "stdafx.h"
#ifdef _NTJ_UNITTEST
#include "DBLib/dbLib.h"
#endif
#include "DBThreadManager.h"
#include "DBThreadProcedure.h"
#include "Thread.h"

DBThreadManager::DBThreadManager(void)
{
    m_pConnPool = 0;
	m_isStopped = false;
	memset(m_dbThreadProcedures,0,sizeof(m_dbThreadProcedures));
	memset(m_dbThreads,0,sizeof(m_dbThreads));
	m_dbThreadCount = 0;
}

DBThreadManager::~DBThreadManager()
{	
    Stop();
    
    //清理资源
    for(U32 i = 0; i < m_dbThreadCount; ++i)
    {
        //if (0 != m_dbThreads[i])
        //{
        //    m_dbThreads[i]->WaitForCompletion();
        //}

        //线程需要先释放
        SAFE_DELETE(m_dbThreads[i]);
        
        if (m_pConnPool && m_dbThreadProcedures[i])
        {
            m_pConnPool->Unlock(m_dbThreadProcedures[i]->GetDBConn());
        }
        
        SAFE_DELETE(m_dbThreadProcedures[i]);
    }
    
    m_dbThreadCount = 0;
    m_isStopped = true;
}

bool DBThreadManager::Start(CDBConnPool* pConnPool,int connCount,int ctxId)
{
	if (m_isStopped)
	    return false;

	assert(0 != pConnPool);
    
    if (0 == pConnPool)
        return false;
    
    m_isStopped     = false;
    m_dbThreadCount = 0;
    m_pConnPool     = pConnPool;
    
    memset(m_dbThreadProcedures,0,sizeof(DBThreadProcedure*)*MAX_DB_CONN);
    memset(m_dbThreads,0,sizeof(CThread*)*MAX_DB_CONN);
    
    for(int i = 0; i < connCount; ++i)
    {
        CDBConn* pConn = pConnPool->Lock();
        
        if (0 != pConn)
        {   
			pConn->SetThreadID(i);
            DBThreadProcedure* pThreadProcedure = new DBThreadProcedure(ctxId,pConn);
            
            m_dbThreadProcedures[m_dbThreadCount] = pThreadProcedure;
            m_dbThreads[m_dbThreadCount] = CThread::Create(pThreadProcedure);

            m_dbThreadCount++;
        }
    }
	    
    for(U32 i = 0; i < m_dbThreadCount; ++i)
    {
        if(m_dbThreads[i])
        {
            m_dbThreads[i]->Resume();
        }
    }
    
    return true;
}

bool DBThreadManager::Stop()
{
    m_isStopped = true;
    
    //通知线程结束
    for(U32 i = 0; i < m_dbThreadCount; ++i)
    {
        if(m_dbThreadProcedures[i])
        {
            m_dbThreadProcedures[i]->Stop();
        }
    }

    for(U32 i = 0; i < m_dbThreadCount; ++i)
    {
        if(m_dbThreads[i])
        {
            m_dbThreads[i]->WaitForCompletion();
        }
    }
    
    return true;
}

bool DBThreadManager::SendPacket(PacketBase* pPacket,int SocketHandle,stPacketHead *pHead)
{
    if (m_isStopped || 0 == m_pConnPool)
    {
        assert(0);
	    return false;
    }
	    
    assert(0 != pPacket);
    assert(0 != pHead);
    
    static U32 ms_pos = 0;
    U32 pos = ms_pos % m_dbThreadCount;
    ms_pos++;
    
    if (pos >= MAX_DB_CONN || 0 == m_dbThreadProcedures[pos])
    {
        assert(0);
        return false;
    }
    
	return m_dbThreadProcedures[pos]->GetProcess()->SendPacket(pPacket,SocketHandle,pHead);
}

bool DBThreadManager::SendPacket(PacketBase* pPacket)
{
    static stPacketHead local;
    return SendPacket(pPacket,0,&local);
}

bool DBThreadManager::IsFinished(void)
{
	for (U32 i = 0; i < m_dbThreadCount; ++i)
	{
		if (0 == m_dbThreadProcedures[i])
			continue;

		if (!m_dbThreadProcedures[i]->GetProcess()->IsFinished())
			return false;
	}

	return true;
}