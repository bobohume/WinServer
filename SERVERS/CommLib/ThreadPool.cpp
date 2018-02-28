#include "stdafx.h"

#include "ThreadPool.h"
#include "Thread.h"

int ThreadPoolProcedure::run(void)
{
    ::CoInitialize(0);

    while(!m_isEnded)
    {
        m_pktProcess.run(0);
        
        Sleep(1);
    }
	
	//处理完所有数据
	m_pktProcess.run(0);

    ::CoUninitialize();
    return 0;
}

ThreadPool::ThreadPool(int connCount,int ctxId)
{
	if (connCount > MAX_THREAD_POOL)
		connCount = MAX_THREAD_POOL;

    m_isStopped     = false;
    m_threadCount = connCount;
	m_pos			= 0;
    
    memset(m_threadProcedures,0,sizeof(ThreadPoolProcedure*) * MAX_THREAD_POOL);
    memset(m_threads,0,sizeof(CThread*) * MAX_THREAD_POOL);
    
	for(int i = 0; i < connCount; ++i)
	{
		ThreadPoolProcedure* pThreadProcedure = new ThreadPoolProcedure(ctxId);

		m_threadProcedures[i] = pThreadProcedure;
		m_threads[i] = CThread::Create(pThreadProcedure);
	}
}

ThreadPool::~ThreadPool()
{	
    Stop();
    
    //清理资源
    for(U32 i = 0; i < m_threadCount; ++i)
    {
        SAFE_DELETE(m_threads[i]);
        SAFE_DELETE(m_threadProcedures[i]);
    }
    
    m_threadCount = 0;
    m_isStopped = true;
}

bool ThreadPool::Start(void)
{
	if (m_isStopped)
	    return false;
	    
    for(U32 i = 0; i < m_threadCount; ++i)
    {
        if(m_threads[i])
        {
            m_threads[i]->Resume();
        }
    }
    
    return true;
}

bool ThreadPool::Stop()
{
    m_isStopped = true;
    
    //通知线程结束
    for(U32 i = 0; i < m_threadCount; ++i)
    {
        if(m_threadProcedures[i])
        {
            m_threadProcedures[i]->Stop();
        }
    }

	//等待结束
    for(U32 i = 0; i < m_threadCount; ++i)
    {
        if(m_threads[i])
        {
            m_threads[i]->WaitForCompletion();
        }
    }
    
    return true;
}

bool ThreadPool::SendPacket(PacketBase* pPacket)
{
    if (m_isStopped)
	    return false;
	    
    assert(0 != pPacket);
    
    U32 pos = m_pos % m_threadCount;
    m_pos++;
    
    if (pos >= MAX_THREAD_POOL || 0 == m_threadProcedures[pos])
    {
        assert(0);
        return false;
    }
    
	static stPacketHead local;
	return m_threadProcedures[pos]->GetProcess()->SendPacket(pPacket,0,&local);
}