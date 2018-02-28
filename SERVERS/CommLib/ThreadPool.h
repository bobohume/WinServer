#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include "PacketBase.h"
#include "PacketProcess.h"
#include "ThreadProcedure.h"

class CThread;

//需要多线程处理的对象
class ThreadBase : public PacketBase
{
public:
	ThreadBase(void) {}
	virtual ~ThreadBase(void) {}
	
	virtual bool Read(Base::BitStream& stream) {return true;}
	virtual int  Execute(int ctxId,void* param) = 0;
	virtual int	 Execute(int ctxId,stPacketHead *pHead,int SocketHandle,void* param)
	{
		return Execute(ctxId,param);
	}
	
	virtual	int	 GetPacketID(void)  const {return -1;}
};

class ThreadPoolProcedure : public ThreadProcedure
{
public:
    ThreadPoolProcedure(int ctxId)
    {   
        m_pktProcess.SetCtxId(ctxId);
        m_isEnded = false;
    }
    
    ~ThreadPoolProcedure(void)
    {
        m_isEnded = true;
    }
    
    void Stop(void) {m_isEnded = true;}
       
    PacketProcess* GetProcess(void) {return &m_pktProcess;}
    
    //method from ThreadProcedure
    virtual int run(void);
private:
    bool          m_isEnded;
    PacketProcess m_pktProcess;
};

#define MAX_THREAD_POOL 10

class ThreadPool
{
public:
    ThreadPool(int connCount,int ctxId);
    ~ThreadPool(void);
    
    //开始和结束只能调用1次
    bool Start(void);
    bool Stop(void);
    
    bool SendPacket(PacketBase* pPacket);
private:
    bool m_isStopped;
       
    ThreadPoolProcedure* m_threadProcedures[MAX_THREAD_POOL];
    CThread* m_threads[MAX_THREAD_POOL];

	U32 m_pos;
    U32 m_threadCount;
};

#endif /*_THREADPOOL_H_*/