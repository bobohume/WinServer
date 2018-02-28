#include "stdafx.h"
#include <signal.h>
#include "ServerCloseMonitor.h"
#include "CommLib.h"
#include "Thread.h"
#include "Common/UdpLog.h"
#include <windows.h>
#include <time.h>
#include <atltime.h>

const char* g_genExitEventName(bool isCloseInTime,const char* append)
{
    static std::string eventName;

    if (isCloseInTime)
    {
        eventName = "sxz_et_";
    }
    else
    {
        eventName = "sxz_e_";
    }

    char buf[256] = {0};
    _itoa_s(GetCurrentProcessId(),buf,256,10);
    eventName += buf;
    eventName += append;

    return eventName.c_str();
}

ServerCloseMonitor* ServerCloseMonitor::g_pServerMonitor = 0;

ServerCloseMonitor::ServerCloseMonitor(U32& runMode,const char* append):
	m_runMode(runMode)
{
    srand((unsigned)time( NULL ));
    
    m_isEnded      = false;
    m_runMode      = 0;
    m_append       = append ? append : "";
	m_closeEvent   = CreateEvent(NULL,TRUE,FALSE,g_genExitEventName(false,m_append.c_str()));
    m_closeEventEx = CreateEvent(NULL,TRUE,FALSE,g_genExitEventName(true,m_append.c_str()));
	m_pThread      = CThread::Create(this);
    g_pServerMonitor = this;
    m_closeMode    = 0;
    m_fromTime     = 0;
    m_hbSended     = true;
    m_hbTickEnable = false;

	if (m_pThread)
		m_pThread->Resume();
}

ServerCloseMonitor::~ServerCloseMonitor(void)
{
	m_isEnded = true;
	CloseHandle((HANDLE)m_closeEvent);
    CloseHandle((HANDLE)m_closeEventEx);
    g_pServerMonitor = 0;

	if (0 != m_pThread)
	{
		m_pThread->WaitForCompletion();
	}

	SAFE_DELETE(m_pThread);
}

ServerCloseMonitor* ServerCloseMonitor::Instance()
{
    return g_pServerMonitor;
}

void ServerCloseMonitor::OnBreak(int)
{
    if (g_pServerMonitor)
    {
	    g_pServerMonitor->m_runMode   = SERVER_RUN_REQUESTCLOSE;
        g_pServerMonitor->m_closeMode = 0;
    }
}

int ServerCloseMonitor::run(void)
{
    signal(SIGBREAK, OnBreak);
    signal(SIGINT,   OnBreak);

	bool initFlag = true;

    HANDLE hWaitExit[2] = {m_closeEvent,m_closeEventEx};

	while(!m_isEnded)
	{   
		DWORD retVal = WaitForMultipleObjects(2,hWaitExit,FALSE,1000);

        if (WAIT_OBJECT_0 == retVal)
		{
			m_runMode   = SERVER_RUN_REQUESTCLOSE;
            m_isEnded   = true;
            m_closeMode = 1;
            g_Log.WriteLog("收到监控对服务器的关闭请求");
		}
		
        if (WAIT_OBJECT_0 +1 == retVal)
        {
            ResetEvent(m_closeEventEx);

            m_runMode   = SERVER_RUN_REQUESTCLOSEINTIME;
            m_closeTime = time(0) + 60 * ONE_MINUTE;

            //获取关闭的时间
            const TCHAR REG_GAME_INSTALL[] =  "Software\\FireRain\\serverclosetime";

            HKEY key;
            char closeTime[256] = {0};
            
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,REG_GAME_INSTALL,0,KEY_READ,&key) == ERROR_SUCCESS)
            {
                DWORD size, type;
                size = sizeof(closeTime);

                if(RegQueryValueEx(key,"time",NULL,&type,(LPBYTE)&closeTime,&size) == ERROR_SUCCESS)
                {   
                    int yy,mm,dd,h,m,s,delay;
                    sscanf_s(closeTime,"%d-%d-%d|%d:%d:%d|%d",&yy,&mm,&dd,&h,&m,&s,&delay);
                    
                    try
                    {
                        CTime fromTime(yy,mm,dd,h,m,s);
                        m_fromTime  = fromTime.GetTime();
                        m_closeTime = m_fromTime + (3 + delay) * 60;
                    }
                    catch(...)
                    {
                        g_Log.WriteError("收到定时监控时间设置不正确!!!服务器将在10分钟后关闭！");

                        m_fromTime  = time(0);
                        m_closeTime = m_fromTime + 600;
                    }
                }

                RegCloseKey(key);
            }

            g_Log.WriteLog("收到监控对服务器的定时关闭请求,time[%s]",closeTime);
        }
        
        static int lastSendTime = (int)time(0);
        
        if (SERVER_RUN_REQUESTCLOSEINTIME == m_runMode)
        {
            time_t curTime = time(0);

            if (curTime > m_closeTime)
            {
                g_Log.WriteLog("设定时间已到,关闭服务器");
                m_runMode = SERVER_RUN_REQUESTCLOSE;
                m_isEnded = true;
                m_closeMode = 2;
            }
            else
            {
                if (curTime >= m_fromTime && curTime - lastSendTime > 2)
                {
                    int remain = (int)(m_closeTime - curTime) / 60;

                    if (remain > 0)
                        g_Log.WriteLog("服务器将在[%d]分后关闭.",remain);
                    else
                        g_Log.WriteLog("服务器将在[%d]秒后关闭.",m_closeTime - curTime);

                    lastSendTime = (int)curTime;
                }
            }
        }

        if (SERVER_RUN_REQUESTCLOSE == m_runMode)
        {
            m_isEnded = true;
        }
        
        if (!m_hbTickEnable || m_hbSended)
        {
		    UdpLog::Instance()->SendHeartBeat(initFlag);
            m_hbSended = false;
        }

		initFlag = false;

        Sleep(1000);
	}

	return 0;
}

U32 ServerCloseMonitor::GetCloseTime()
{
    time_t curTime = time(0);

    if (curTime >= m_closeTime)
    {
        return 1;
    }
    else
    {
        return (U32)(m_closeTime - curTime);
    }
}

void ServerCloseMonitor::LoopTick(void)
{
    m_hbSended = true;

    if (!m_hbTickEnable)
    {
        m_hbTickEnable = true;
    }
}