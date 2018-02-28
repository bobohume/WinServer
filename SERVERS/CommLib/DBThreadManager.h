#ifndef _DBTHREADMANAGER_H_
#define _DBTHREADMANAGER_H_

#include "base/types.h"
#ifndef _NTJ_UNITTEST
#include "DBLib/dbLib.h"
#else
class CDBConnPool;
#endif
#include "Thread.h"

#define MAX_DB_CONN 255

class  PacketBase;
struct stPacketHead;
class  DBThreadProcedure;
class  CThread;

//db线程也许需要在执行完成后统一进行处理
typedef void (*DB_PACKET_HANDLE)(PacketBase*,void* param);

//绑定DB线程和DBConn对象,使每个线程都操作固定的DBConn
class DBThreadManager
{
public:
    DBThreadManager(void);
    ~DBThreadManager(void);
    
    //开始和结束只能调用1次
    bool Start(CDBConnPool* pConnPool,int connCount,int ctxId);
    bool Stop(void);
    
    //随机分配操作请求到相应的db线程
    bool SendPacket(PacketBase* pPacket,int SocketHandle,stPacketHead *pHead);
    bool SendPacket(PacketBase* pPacket);
	
	//是否处理完所有的数据包
	bool IsFinished(void);
	
	//如果包还在db process里,则调用这个函数统一处理包
	//void DoEachPacket(DB_PACKET_HANDLE,void* param);
private:
    bool m_isStopped;
    
    CDBConnPool* m_pConnPool;
       
    DBThreadProcedure* m_dbThreadProcedures[MAX_DB_CONN];
    CThread*           m_dbThreads[MAX_DB_CONN];
    
    U32 m_dbThreadCount;
};

#endif /*_DBTHREADMANAGER_H_*/