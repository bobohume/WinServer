#ifndef _SERVERCLOSEMONITOR_H_
#define _SERVERCLOSEMONITOR_H_

#include "CommLib.h"
#include "ThreadProcedure.h"
#include <string>

class CThread;

enum SERVERMONITORSTATUS
{
    SERVER_RUN_NORMAL,
    SERVER_RUN_REQUESTCLOSE,
    SERVER_RUN_REQUESTCLOSEINTIME,
};

//开启1个线程用来监控服务器关闭事件
class ServerCloseMonitor : public ThreadProcedure
{
public:
	ServerCloseMonitor(U32& runMode,const char* append = 0);
	~ServerCloseMonitor(void);

	virtual int run(void);

    U32 GetCloseTime();
    time_t GetFromTime(void) {return m_fromTime;}

    static ServerCloseMonitor* Instance();

	void Close(void) {OnBreak(0);}

    bool IsClosedInOrder(void) const {return m_closeMode == 2;}
    
    void LoopTick(void);
protected:
    static void OnBreak(int);

    bool     m_isEnded;
    time_t   m_closeTime;
    time_t   m_fromTime;
	U32&     m_runMode;
	CThread* m_pThread;
	void*    m_closeEvent;
    void*    m_closeEventEx;
    std::string m_append;
    int      m_closeMode;
    bool     m_hbSended;     
    bool     m_hbTickEnable;

    static ServerCloseMonitor* g_pServerMonitor;
};

#endif /*_SERVERCLOSEMONITOR_H_*/