#include "base/bitStream.h"
#include <string>
#include "LogServer.h"
#include "LogManager.h"
#include "Common/version.h"
#include "common/cfgbuilder.h"
#include "Common/UdpLog.h"
#include "CommLib/CommLib.h"
#include "Common/build_no.h"
#include <time.h>

bool CLogServer::onInit()
{
	//设置日志文件名
	ConfigLog("LogLog");
	std::string DBServer, DBUser, DBPwd, DBName;
	std::string LogServerIP, LogServerPort;
	std::string MaxUserCount, MinUserCount;

	CFG_BUIILDER->Get("Log_LANAddress", ':', LogServerIP, LogServerPort);
	DBServer		= CFG_BUIILDER->Get("LogDB_LANIP");
	DBName			= CFG_BUIILDER->Get("LogDB_Name");
	DBUser			= CFG_BUIILDER->Get("LogDB_UserId");
	DBPwd			= CFG_BUIILDER->Get("LogDB_Password");
	MinUserCount	= "500";
	MaxUserCount	= "1000";

	//根据CPU数计算实际线程数
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	t_server_param param;
	param.workThreadCount	= siSysInfo.dwNumberOfProcessors * 2 + 1;
	param.ipAddr			= LogServerIP.c_str();
	param.port				= atoi( LogServerPort.c_str() );
	param.param.MaxClients	= atoi( MaxUserCount.c_str() );
	param.param.MinClients	= atoi( MinUserCount.c_str() );

	SetConsole("log", G_PVERSION, LogServerIP, LogServerPort);
	g_Log.WriteLog("**********************************************************");
	g_Log.WriteLog("\tLogServer Version:\t%s",G_PVERSION);
	g_Log.WriteLog("\tLogServerIP(LAN):\t%s:%d", param.ipAddr, param.port);
	g_Log.WriteLog("\tLogDBServer(LAN):\t%s", DBServer.c_str());
	g_Log.WriteLog("\tLogDBName:\t\t%s", DBName.c_str());
	g_Log.WriteLog("**********************************************************");
	g_Log.WriteLog("Initialize ...... ");

	//先初始化日志数据库，检查是否成功
	int MaxDBConnection = param.workThreadCount + 1;
	if(!CLogManager::GetInstance()->Initialize(DBServer.c_str(), DBName.c_str(), DBUser.c_str(), DBPwd.c_str(), MaxDBConnection))		
		return false;

	mWorkQueue->Initialize( _eventProcess, param.workThreadCount ,"_eventProcess", INFINITE);
	strcpy_s(param.param.Name, 64, "LogServer");
	mServer->Initialize( param.ipAddr, param.port, &param.param );
	mServer->Start();
	CLogManager::GetInstance()->Startup();
	return true;
}

void CLogServer::onMainLoop()
{
	//CLogManager::GetInstance()->ChangeDataBase();
}

void CLogServer::onShutdown()
{
	CLogManager::GetInstance()->ShutDown();
}

extern U32 g_runMode;

bool CLogServer::IsClosed(void)
{
    if (g_runMode != SERVER_RUN_REQUESTCLOSE)
        return false;

    ServerCloseMonitor* pMonitor = ServerCloseMonitor::Instance();

    if (0 != pMonitor && pMonitor->IsClosedInOrder())
    {
        //判断world server是否已经关闭
        if (EventGuard::CheckExist("_worldserver_"))
        {
            g_Log.WriteLog("正在等待World服务器关闭...");
            Sleep(1000);

            return false;
        }
    }

    return true;
}