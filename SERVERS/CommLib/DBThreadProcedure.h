#ifndef _DBTHREADPROCEDURE_H_
#define _DBTHREADPROCEDURE_H_

#include "PacketProcess.h"
#include "ThreadProcedure.h"

class CDBConn;

//db的线程处理函数,用来推进当前db线程的消息包处理
class DBThreadProcedure : public ThreadProcedure
{
public:
    DBThreadProcedure(int ctxId,CDBConn* pConn)
    {   
        assert(0 != pConn);
        
        m_pDBConn = pConn;
        m_pktProcess.SetCtxId(ctxId);
        m_isEnded = false;
    }
    
    ~DBThreadProcedure(void)
    {
        m_isEnded = true;
        m_pDBConn = 0;
    }
    
    void Stop(void) {m_isEnded = true;}
       
    PacketProcess* GetProcess(void) {return &m_pktProcess;}
    
    CDBConn* GetDBConn(void) {return m_pDBConn;}

    //method from ThreadProcedure
    virtual int run(void);
private:
    //数据库连接
    CDBConn*      m_pDBConn;
    
    bool          m_isEnded;
    PacketProcess m_pktProcess;
};

#endif /*_DBTHREADPROCEDURE_H_*/