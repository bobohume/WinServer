// MonitorClient.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "CommLib.h"
#include "MonitorClient.h"
#include "ServerCloseMonitor.h"

ServerCloseMonitor* g_pServerCloseMonitor = 0;

MONITORDECL bool Monitor_Init(unsigned int& runStatus)
{
    if (0 != g_pServerCloseMonitor)
        return true;

    g_pServerCloseMonitor = new ServerCloseMonitor(runStatus);
    return true;
}

MONITORDECL bool Monitor_Close(void)
{
    if (0 == g_pServerCloseMonitor)
        return true;

    delete g_pServerCloseMonitor;
    g_pServerCloseMonitor = 0;
    return true;
}

MONITORDECL int  Monitor_GetCloseSeconds(void)
{
    if (0 == g_pServerCloseMonitor)
        return 0;

    return g_pServerCloseMonitor->GetCloseTime();
}