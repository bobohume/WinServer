// WorldServer.cpp : 定义控制台应用程序的入口点。
//

#include <signal.h>

#include "DBLib/dbPool.h"
//#include "Common/Script.h"
#include "Common/CommonServer.h"
#include "Common/CommonClient.h"
#include "base/WorkQueue.h"
#include "Common\LogHelper.h"
#include "PlayerMgr.h"
#include "ServerMgr.h"
#include "WorldServer.h"
#include "UserPacketProcess.h"
#include "EventProcess.h"
#include "SavePlayerJob.h"
#include "BanList.h"
#include "SocialManager.h"
#include "common/cfgbuilder.h"
#include "Common/UdpLog.h"
#include "DBLayer/Data/DataBase.h"
#include "Common/BanInfoData.h"
#include "Common/ChatBase.h"
#include "ChatHelper.h"
#include "Common/CollectItemData.h"
#include "Common/DumpHelper.h"
#include "RemotePacketProcess.h"
#include "CommLib/ServerCloseMonitor.h"
#include "DBContext.h"
#include "wintcp/dtServerSocket.h"
#include <atltime.h>
#include "CommLib/ConfigChangeMonitor.h"
#include "Common/mRandom.h"
#include "Script/lua_tinker.h"
#include "Script/LuaConsole.h"
#include "Common/OrgBase.h"
//#include "Room/RoomMgr.h"
#include "Logic/PlayerInfoMgr.h"
#include "Logic/GMMgr.h"

extern lua_State* L;

SERVER_CLASS_NAME *	SERVER_CLASS_NAME::m_pInstance		    =	NULL;
CScript *			SERVER_CLASS_NAME::m_pScript		    =	NULL;
CWorkQueue *		SERVER_CLASS_NAME::m_pWorkQueue		    =	NULL;
CWorkQueue *		SERVER_CLASS_NAME::m_pSocialWorkQueue	=	NULL;

std::string RemoteCount;
std::string RemoteNetIP;
std::string RemoteNetPort;
std::string MaxRemoteCount;
std::string RemoteAccountIP;
std::string RemoteAccountPort;
std::string DB_ActorServer;
std::string DB_ActorUserId;
std::string DB_ActorPassword;
std::string DB_ActorName;

std::string DB_BillingServer;
std::string DB_BillingUserId;
std::string DB_BillingPassword;
std::string DB_BillingName;

std::string RedisIp;
std::string RedisPort;
std::string RedisPassword;

std::string LogServerIP;
std::string LogServerPort;
std::string AreaId;
std::string MaxDBConnection;

std::string g_openTime;

const char GLOBAL_EXIT_EVENT[] = "md_worldserver";

int g_nopass9 = 1;
U32 g_runMode = 0;

//开启1个线程监护world server网络线程的卡死现象.
//如果超过时间没有反应,则产生日志和相应的dump信息
class MainThreadMonitorProcedure : public ThreadProcedure
{
public:
	MainThreadMonitorProcedure()
	{   
		m_isEnded = false;
		m_count   = 5;
	}

	~MainThreadMonitorProcedure(void)
	{
		m_isEnded = true;
	}

	void Stop(void) {m_isEnded = true;}

	//method from ThreadProcedure
	virtual int run(void)
	{
		if (0 == SERVER->GetPlayerManager())
			return 0;

		while(!m_isEnded && m_count > 0)
		{
			try 
			{
				time_t curTime = time(0);

				if (0 != SERVER->GetPlayerManager()->m_lastUpdateTime &&
					(abs(int(curTime - SERVER->GetPlayerManager()->m_lastUpdateTime)) > 3 * 60))
				{
					g_Log.WriteError("检测到逻辑线程处于死锁状态!");
					debugDump(MAXI_DUMP);

					//强制设置时间,再下次的时候输出dump
					SERVER->GetPlayerManager()->m_lastUpdateTime = curTime;
					m_count--;
				}
			}
			catch(...)
			{
				m_isEnded = true;
			}

			Sleep(1000);
		}

		m_isEnded = true;
		return 0;
	}
private:
	bool m_isEnded;
	int  m_count;
};

static MainThreadMonitorProcedure g_mainThreadMonitorProcedure;

SERVER_CLASS_NAME::SERVER_CLASS_NAME()
{
	m_Inited			=	false;
	m_accountSocket    =   0;
	m_pServerService	=	NULL;
	m_pWorkQueue		=	NULL;
	m_pSocialWorkQueue	=	NULL;
	m_pPlayerManager	=	NULL;
	m_pServerManager	=	NULL;
	m_pSocialManager	=	NULL;
	m_logHelper         =   NULL;
	m_ActorDB			=	NULL;
	m_BillingDB			=   NULL;
	m_LogDB				=	NULL;
	m_pRemoteAccountClient = NULL;
	m_pCommonDBManager  = 0;
	m_pQueryDBManager   = 0;
	m_pTopDBManager     = 0;
	m_pMailDBManager = 0;
	m_pMonitorThread    = 0;
	m_pGMThread = 0;
	m_pPlayerDBManager  = 0;

	m_bIsQuit			=	0;
	m_pUncompressThread = 0;
	m_pMonitorMemoryThread = 0;

	ISocket::InitNetLib();

	srand( ::GetTickCount() );
}

SERVER_CLASS_NAME::~SERVER_CLASS_NAME()
{   
    //释放线程和DB线程
    SAFE_DELETE(m_pCommonDBManager);
	SAFE_DELETE(m_pQueryDBManager);
	SAFE_DELETE(m_pTopDBManager);
	SAFE_DELETE(m_pMailDBManager);
	SAFE_DELETE(m_pPlayerDBManager);
    SAFE_DELETE(m_pUncompressThread);
	SAFE_DELETE(m_pMonitorThread);
	SAFE_DELETE(m_pGMThread);
	SAFE_DELETE(m_pMonitorMemoryThread);

    //释放work queue
    //----------------------------------------------------------------------
    SAFE_DELETE(m_pWorkQueue);
	SAFE_DELETE(m_pSocialWorkQueue);

    //释放常规对象
    //----------------------------------------------------------------------
	SAFE_DELETE(m_pServerService);
	SAFE_DELETE(m_pPlayerManager);
	SAFE_DELETE(m_pServerManager);
	SAFE_DELETE(m_pSocialManager);
	SAFE_DELETE(m_logHelper);;
	SAFE_DELETE(m_pRemoteAccountClient);
	SAFE_DELETE(m_pSocialManager);

    //最后释放数据库的链接对象
    //----------------------------------------------------------------------
	SAFE_DELETE(m_ActorDB);
	SAFE_DELETE(m_BillingDB);
	SAFE_DELETE(m_LogDB);
	
	m_pInstance = NULL;
	ISocket::UninitNetLib();
}

void SERVER_CLASS_NAME::OnBreak(int)
{
	g_runMode = SERVER_RUN_REQUESTCLOSE;
}

void SERVER_CLASS_NAME::ShowMessage()
{
	g_Log.WriteLog("**********************************************************");
	g_Log.WriteLog("\tWorldServer Version:\t%s", G_PVERSION);
	//g_Log.WriteLog("\tRemoteCount\t\t\t%s",  RemoteCount.c_str() );
	//g_Log.WriteLog("\tMaxRemoteCount\t\t\t%s", MaxRemoteCount.c_str() );
	g_Log.WriteLog("\tWorldServerIP(LAN):\t%s:%s", RemoteNetIP.c_str(), RemoteNetPort.c_str() );
	g_Log.WriteLog("\tRemoteAccount(LAN):\t%s:%s", RemoteAccountIP.c_str(), RemoteAccountPort.c_str() );
	//g_Log.WriteLog("\tMaxDBConnection\t\t\t%s",  MaxDBConnection.c_str() );
	g_Log.WriteLog("\tActorDBServer(LAN):\t%s", DB_ActorServer.c_str());
	g_Log.WriteLog("\tActorDBName:\t\t%s", DB_ActorName.c_str());
	//g_Log.WriteLog("\tDB_ActorUserId\t\t\t%s", DB_ActorUserId.c_str());
	//g_Log.WriteLog("\tDB_ActorPassword\t\t%s", DB_ActorPassword.c_str());
	//g_Log.WriteLog("\tBillingDBServer(LAN):\t%s", DB_BillingServer.c_str());
	//g_Log.WriteLog("\tBillingDBName:\t\t%s", DB_BillingName.c_str());
	g_Log.WriteLog("\tLogServer(LAN):\t\t%s:%s", LogServerIP.c_str(), LogServerPort.c_str());
	g_Log.WriteLog("\tAreaId:\t\t\t%s",AreaId.c_str());
	g_Log.WriteLog("\tOpenTime:\t\t\t%s",g_openTime.c_str());
	g_Log.WriteLog("**********************************************************");
}

void SERVER_CLASS_NAME::OnServerStarted()
{
	//在未开服前角色允许登录创建时间范围
	//默认可以创建1000个
	char strLoginTimeRange[64] = {0};

	for (int i = 1; i <= 1000; ++i)
	{
		sprintf_s(strLoginTimeRange,"LoginTime_range_%d",i);
		std::string strTime = CFG_BUIILDER->Get(strLoginTimeRange);

		if ("" == strTime)
			continue;

		TimeRange range;

		if (!range.InitFromString(strTime.c_str(),true))
		{
			g_Log.WriteError("开服前登录时间[%s]范围限制设置格式或时间不正确",strLoginTimeRange);
			continue;
		}

		SERVER->GetServerManager()->AddLoginTimeRange(range);
	}

	int y = 0,m = 0,d = 0,h = 0,min = 0,s = 0;

	if (6 == sscanf_s(g_openTime.c_str(),"%d-%d-%d|%d:%d:%d",&y,&m,&d,&h,&min,&s) || "" == g_openTime)
	{
		if ("" != g_openTime)
		{
			CTime timeVal(y,m,d,h,min,s);
			SERVER->GetServerManager()->SetOpenTime(timeVal.GetTime());
		}
	}
	else
	{
		g_Log.WriteError("无法设置服务器的开服时间");
	}

	g_Log.WriteLog("*****************世界服务已启动****************************");
}

bool SERVER_CLASS_NAME::InstallBreakHandlers()
{
	signal(SIGBREAK, OnBreak);
	signal(SIGINT, OnBreak);

	return true;
}

void *Create_UserPacketProcess()
{
	return new UserPacketProcess;
}

bool SERVER_CLASS_NAME::InitializeDatabase()
{
	static const int OtherCounts = 6;//另外再增加6个固定数据库连接用于独立数据线程
	int nCount = atoi(MaxDBConnection.c_str()) + OtherCounts;

	m_ActorDB = new DataBase;
	DBError err = m_ActorDB->Initialize(DB_ActorServer.c_str(),DB_ActorName.c_str(),
		DB_ActorUserId.c_str(),DB_ActorPassword.c_str(),nCount);

	if(err != DBERR_NONE)
	{
		g_Log.WriteFocus("Warning:: %s数据库初始化失败!", DB_ActorName.c_str());
		return false;
	}

	//m_BillingDB = new DataBase;
	//err = m_BillingDB->Initialize(DB_BillingServer.c_str(),DB_BillingName.c_str(),
	//	DB_BillingUserId.c_str(),DB_BillingPassword.c_str(),nCount);

	//if(err != DBERR_NONE)
	//{
	//	g_Log.WriteFocus("Warning:: %s数据库初始化失败!", DB_BillingName.c_str());
	//	return false;
	//}

	m_RedisPool = new CRedisConnPool();
	if (!m_RedisPool->Open(10, RedisIp.c_str(), atoi(RedisPort.c_str()), RedisPassword.c_str()))
	{
		g_Log.WriteFocus("Warning:: %s redis初始化失败!", RemoteNetIP.c_str());
		return false;
	}

	g_Log.WriteLog("%s数据库初始化成功!", DB_ActorName.c_str());
	//g_Log.WriteLog("%s和%s数据库初始化成功!", DB_ActorName.c_str(), DB_BillingName.c_str());
	return true;
}
void SERVER_CLASS_NAME::InitData()
{
}

bool SERVER_CLASS_NAME::Initialize()
{
	HeapSetInformation(NULL,HeapEnableTerminationOnCorruption,NULL,0);
	if(m_Inited)
		return true;

	if(!InstallBreakHandlers())
		return false; 

	//设置日志文件名
	ConfigLog("WorldLog");
	g_Log.SetIdleWarning(3);

	RemoteCount = "50";
	MaxRemoteCount = "200";
	CFG_BUIILDER->Get("World_LANAddress", ':', RemoteNetIP, RemoteNetPort);
	CFG_BUIILDER->Get("Log_LANAddress", ':', LogServerIP, LogServerPort);
	CFG_BUIILDER->Get("RemoteAccount_LANAddress", ':', RemoteAccountIP, RemoteAccountPort);
	DB_ActorServer		= CFG_BUIILDER->Get("ActorDB_LANIP");
	DB_ActorName		= CFG_BUIILDER->Get("ActorDB_Name");
	DB_ActorUserId		= CFG_BUIILDER->Get("ActorDB_UserId");
	DB_ActorPassword	= CFG_BUIILDER->Get("ActorDB_Password");

	RedisIp				= CFG_BUIILDER->Get("RedisIp");
	RedisPort           = CFG_BUIILDER->Get("RedisPort");
	RedisPassword       = CFG_BUIILDER->Get("RedisPassword");
	//DB_BillingServer	= CFG_BUIILDER->Get("BillingDB_LANIP");
	//DB_BillingName		= CFG_BUIILDER->Get("BillingDB_Name");
	//DB_BillingUserId	= CFG_BUIILDER->Get("BillingDB_UserId");
	//DB_BillingPassword	= CFG_BUIILDER->Get("BillingDB_Password");
	AreaId				= CFG_BUIILDER->Get("AreaID");
	MaxDBConnection		= "17";//8cpu*2+1
	
	std::string nopass9 = CFG_BUIILDER->Get("NoPass9").c_str();
	if (nopass9 != "")
		g_nopass9	= atoi(nopass9.c_str());	

	std::string maxLinePlayers = CFG_BUIILDER->Get("MaxLinePlayerCount");

	if( maxLinePlayers != "" )
		g_maxLinePlayers = atoi( maxLinePlayers.c_str() );

	g_openTime = CFG_BUIILDER->Get("OpenTime");

	SetConsole("world", G_PVERSION, RemoteNetIP, RemoteNetPort);
	ShowMessage();

	if(g_nopass9 == 1)
	{
		g_Log.WriteFocus("特别关注: nopass9=1仅适用于内部测试,正式上线前请设置nopass9=0并且清空重置角色数据库数据.");
		g_Log.WriteFocus("特别关注: nopass9=1仅适用于内部测试,正式上线前请设置nopass9=0并且清空重置角色数据库数据.");
		g_Log.WriteFocus("特别关注: nopass9=1仅适用于内部测试,正式上线前请设置nopass9=0并且清空重置角色数据库数据.");
	}

	g_Log.WriteLog("正在初始化数据库连接...");

	//初始化数据库连接
	if(!InitializeDatabase())
		return false;

	m_logHelper = new CLogHelper;
	m_logHelper->connect(LogServerIP.c_str(), atoi(LogServerPort.c_str()));
	InitData();//读取data

	m_pServerManager = new CServerManager;
	m_pServerManager->Initialize();

	m_pWorkQueue = new CWorkQueue;
	m_pWorkQueue->Initialize(EventProcess,1,"EventProcess", 50);

	//m_pOrgWorkQueue	= new CWorkQueue;
	//m_pOrgWorkQueue->Initialize(EventProcess, 1, "EventProcess");

	m_pSocialWorkQueue = new CWorkQueue;
	m_pSocialWorkQueue->Initialize(EventProcess, 1, "EventProcess", INFINITE);

	//m_pGMWorkQueue = new CWorkQueue;
	//m_pGMWorkQueue->Initialize(EventProcess, 1, "EventProcess");

	m_pPlayerManager = new CPlayerManager;
	m_pWorkQueue->GetTimerMgr().notify(m_pPlayerManager, &CPlayerManager::TimeProcess, 1000);
	//m_pLogicEventManager = new CLogicEventManager;
	m_pSocialManager = new CSocialManager(m_ActorDB->GetPool());
	//TimerMgr::GetInstance()->AddTimer(m_pSocialManager, 60*1000);
	//m_EventManager = new CEventManager;
	//CheckCodeCache::GetInstancePtr()->Init();
	//m_LeagueManager = new LeagueManager(m_ActorDB->GetPool());
	//m_LeagueManager->Start();
	//m_OrgManager = new OrgManager(m_ActorDB->GetPool());
	//m_OrgManager->Init();
	//m_pTimeMgr->AddObject(m_OrgManager);    

	currentAreaId = atoi( AreaId.c_str() );

	//消息处理绑定
	EventFn::Initialize();

	stServerParam Param;
	//开放给帐号服务器和网关服务器的服务
	m_pServerService	=	new CommonServer<UserPacketProcess>;  //CommonServer<>(Create_UserPacketProcess);
	dStrcpy(Param.Name,64, "ServerService");
	Param.MinClients	=	atoi( RemoteCount.c_str() );
	Param.MaxClients	=	atoi( MaxRemoteCount.c_str() );
	m_pServerService->Initialize( RemoteNetIP.c_str(), atoi( RemoteNetPort.c_str() ),&Param);
	m_pServerService->SetMaxReceiveBufferSize(MAX_PACKET_SIZE);
	m_pServerService->SetMaxSendBufferSize(MAX_PACKET_SIZE);

	//连接远程帐号服务器
	stAsyncParam aParam;
	dStrcpy(aParam.Name, 64, "RemoteAccount");
	m_pRemoteAccountClient = new CommonClient<RemotePacketProcess>;
	m_pRemoteAccountClient->Initialize(RemoteAccountIP.c_str(), atoi(RemoteAccountPort.c_str()),&aParam);
	m_pRemoteAccountClient->SetConnectType(ISocket::SERVER_CONNECT);

	//监听GM端口
	//CGMRecver::Instance()->Initialize();
	//CBanlist::Instance()->Init();

	//内存共享
	//m_pPlayerManager->InitSMData();
	//m_teamManager.InitSMData();
	//if(m_stallManager.InitSMData() == false)
	//	return false;

	m_Inited = true;

	//数据库的连接基本是要设定1条线程操作
	m_pCommonDBManager = new DBThreadManager;
	m_pCommonDBManager->Start(m_ActorDB->GetPool(),1,DB_CONTEXT_NOMRAL);

	m_pQueryDBManager = new DBThreadManager;
	m_pQueryDBManager->Start(m_ActorDB->GetPool(),1,DB_CONTEXT_NOMRAL);

	//m_pStallDBManager = new DBThreadManager;
	//m_pStallDBManager->Start(m_ActorDB->GetPool(),1,DB_CONTEXT_NOMRAL);

	m_pTopDBManager = new DBThreadManager;
	m_pTopDBManager->Start(m_ActorDB->GetPool(),1,DB_CONTEXT_NOMRAL);

	//m_pBillingDBManager = new DBThreadManager;
	//m_pBillingDBManager->Start(m_BillingDB->GetPool(),1, DB_CONTEXT_NOMRAL);

	//m_pGoldTradeDBManager = new DBThreadManager;
	//m_pGoldTradeDBManager->Start(m_ActorDB->GetPool(),1, DB_CONTEXT_NOMRAL);

	//m_pAuctionDBManager = new DBThreadManager;
	//m_pAuctionDBManager->Start(m_ActorDB->GetPool(),1,DB_CONTEXT_NOMRAL);

	m_pMailDBManager = new DBThreadManager;
	m_pMailDBManager->Start(m_ActorDB->GetPool(),1,DB_CONTEXT_NOMRAL);

	m_pPlayerDBManager = new DBThreadManager;
	m_pPlayerDBManager->Start(m_ActorDB->GetPool(),1,DB_CONTEXT_NOMRAL);

	m_pUncompressThread = new ThreadPool(1,1);
	m_pUncompressThread->Start();

    //m_pChallengeTimeMgr->Start();


	//g_Log.WriteLog("正在同步数据库的商铺数据到共享内存......");
	/*if(GetStallManager()->LoadAllStalls() == false)
	{
		g_Log.WriteFocus(">>>>同步数据库的商铺数据到共享内存失败!<<<<");
		return false;
	}
	else
	{
		g_Log.WriteLog("同步数据库的商铺数据到共享内存全部完成.");
	}*/

	m_pMonitorThread = CThread::Create(&g_mainThreadMonitorProcedure);
	m_pMonitorThread->Resume();

	GMCmdProcedure* m_pGm = new GMCmdProcedure();
	m_pGMThread = CThread::Create(m_pGm);
	m_pGMThread->Resume();

	//设置随机种子
	//MRandomLCG::setGlobalRandSeed();

	//初始化lua
	Player::Export(L);
	PLAYERINFOMGR->Export(L);
	PLAYERINFOMGR->Initialize();
	SERVER->GetWorkQueue()->PostEvent(0,0,0,false,WQ_STARTED);
	return m_Inited;
}

dtServerSocket* SERVER_CLASS_NAME::GetServerSocket()	
{
	IF_ASSERT(m_pServerService == NULL)
		return NULL;
	return m_pServerService;
}

CPlayerManager* SERVER_CLASS_NAME::GetPlayerManager()	
{		
	/*IF_ASSERT(m_pPlayerManager == NULL)
		return NULL;*/
	return m_pPlayerManager;
}

CServerManager* SERVER_CLASS_NAME::GetServerManager()	
{		
	IF_ASSERT(m_pServerManager == NULL)
		return NULL;
	return m_pServerManager;
}

CSocialManager* SERVER_CLASS_NAME::GetSocialManager()	
{
	IF_ASSERT(m_pSocialManager == NULL)
		return NULL;
	return m_pSocialManager;
}

CAsyncSocket2* SERVER_CLASS_NAME::GetRemoteAccountSocket()	
{
	IF_ASSERT(m_pRemoteAccountClient == NULL)
		return NULL;
	return m_pRemoteAccountClient; 
}

CWorkQueue* SERVER_CLASS_NAME::GetWorkQueue()		   
{
	IF_ASSERT(m_pWorkQueue == NULL)
		return NULL;
	return m_pWorkQueue;	
}

CWorkQueue* SERVER_CLASS_NAME::GetSocialWorkQueue()
{
	IF_ASSERT(m_pSocialWorkQueue == NULL)
		return NULL;
	return m_pSocialWorkQueue;	
}

CLogHelper* SERVER_CLASS_NAME::GetLog()     
{       
	if (0 != m_logHelper)
		return m_logHelper;
}

void SERVER_CLASS_NAME::StartService()
{
	if(m_pServerService)	
		m_pServerService->Start();

	if (m_pRemoteAccountClient)
		m_pRemoteAccountClient->Start();

	if (0 != m_pPlayerManager)
	{
		m_pPlayerManager->LoadSimpePlayerDatas(30);
        //m_pPlayerManager->LoadFakenames();
        //m_pPlayerManager->LoadZonePositions();
	}

	OnServerStarted();

	m_StartEvent = CreateEventA( NULL, FALSE, FALSE, GLOBAL_EXIT_EVENT);
}

__time64_t endStartTime;

void SERVER_CLASS_NAME::Maintenance()
{
	g_runMode = 0;
	ServerCloseMonitor monitor(g_runMode);
	ConfigChangeMonitor configMonitor;

	//配置目录监控
	std::vector<std::string> files;
	files.push_back(MONITOR_CONFIGFILE);
	files.push_back(MONITOR_PLAYERNAME);
	files.push_back(MONITOR_SCRIPT_MAINLUA);
	//files.push_back(MONITOR_COPYMAPDATA);
	//files.push_back(MONITOR_BANNAME);
	//files.push_back(MONITOR_BANWORD);
	//files.push_back(MONITOR_COLLECTITEMDATA);
	//files.push_back(MONITOR_ITEMDATA);
	//files.push_back(MONITOR_ORGDEPOTDATA);
	//files.push_back(MONITOR_ORGBOSSDATA);

	configMonitor.Start(SERVER->GetWorkQueue(),files);

	while(!IsClosed(g_runMode))
	{   
		if(m_pRemoteAccountClient)
		{
			if(m_pRemoteAccountClient->GetState() == ISocket::SSF_DEAD)
				m_pRemoteAccountClient->Restart();
		}

		m_logHelper->check();

		int t = _time32(NULL);
		if(t%10==0)
		{
			int totalSendBytes = m_pRemoteAccountClient->m_WaitingSendBytes+m_pRemoteAccountClient->m_PendingSendBytes;

			if(totalSendBytes > 1024 * 1024 * 10)  //>10M 开始输出
			{
				g_Log.WriteWarn("==10秒统计待处理TCP发送字节数统计============================================");
				g_Log.WriteWarn("RemoteAccount服务端口发送 [Waiting:%d,Peding:%d]",m_pRemoteAccountClient->m_WaitingSendBytes,m_pRemoteAccountClient->m_PendingSendBytes);
				g_Log.WriteWarn("==总字节数:%d==========================================================",totalSendBytes);
			}
		}

		Sleep(2000);
	}
}

bool SERVER_CLASS_NAME::IsClosed(int runMode)
{
    if (runMode != SERVER_RUN_REQUESTCLOSE)
        return false;

    ServerCloseMonitor* pMonitor = ServerCloseMonitor::Instance();

    if (0 != pMonitor && pMonitor->IsClosedInOrder())
    {
        //判断data agent是否已经关闭
        if (SERVER->GetServerManager()->HasDataAgent())
        {
            g_Log.WriteLog("正在等待DATA AGENT服务器关闭...");
            Sleep(1000);

            return false;
        }
    }

    return true;
}

void SERVER_CLASS_NAME::DenyService()
{

}

void _WriteEmptyAccountInfo(void)
{
	FILE* fp;
	fopen_s( &fp, "account.info", "w+" );

	if( !fp )
		return ;

	fprintf( fp, "\nPlayerCount: %d\n",0);
	fprintf( fp, "|AccountName\t|PlayerName\t|playerId\t|Status\t|LineId\t|ZoneId\n" );

	fclose(fp);
}

void SERVER_CLASS_NAME::StopService()
{
    //先关闭线程死锁监控.
    g_mainThreadMonitorProcedure.Stop();

	//检测data agent是否已经关闭
	SimpleTimer<30> timeout;
	timeout.Start();

	if (m_pServerManager->HasDataAgent() && !timeout.CheckTimer())
	{
		g_Log.WriteLog("正在等待DataAgent退出...");
		Sleep(1000);
	}

    //m_pChallengeTimeMgr->RemoveAll();

	if(m_logHelper)         m_logHelper->disconnect();
	if(m_pServerService)	m_pServerService->Stop();

	if(m_pWorkQueue)		m_pWorkQueue->Stop();
	//if(m_pOrgWorkQueue)		m_pOrgWorkQueue->Stop();
	if(m_pSocialWorkQueue)	m_pSocialWorkQueue->Stop();
	//if(m_pGMWorkQueue)	    m_pGMWorkQueue->Stop();

	if (m_pRemoteAccountClient) m_pRemoteAccountClient->Stop();

	CloseHandle( m_StartEvent );

	//写入空的accountinfo文件
	_WriteEmptyAccountInfo();

	m_pSocialManager->OnQuit();
	//m_LeagueManager->Stop();
	//m_OrgManager->Stop();

	m_pCommonDBManager->Stop();
	m_pQueryDBManager->Stop();
	//m_pStallDBManager->Stop();
	m_pTopDBManager->Stop();
	//m_pBillingDBManager->Stop();
	//m_pGoldTradeDBManager->Stop();
	m_pUncompressThread->Stop();
	//m_pAuctionDBManager->Stop();
	m_pMailDBManager->Stop();
	m_pPlayerDBManager->Stop();

	g_Log.WriteLog( "WorldServer 正常退出 ... " );
}

void SEND_WORKQUEUE_PACKET(Base::BitStream& packet, WORKQUEUE_MESSGAE workQueue, OTHERWORKQUEUE_MSG msg)
{
	switch (workQueue)
	{
	//case WQ_ORG:
	//	SERVER->GetOrgWorkQueue()->PostEvent(msg, packet.getBuffer(), packet.getPosition(), true, workQueue);
	//	break;
	case WQ_SOCIAL:
		SERVER->GetSocialWorkQueue()->PostEvent(msg, packet.getBuffer(), packet.getPosition(), true, workQueue);
		break;
	case WQ_NORMAL:
		SERVER->GetWorkQueue()->PostEvent(msg, packet.getBuffer(), packet.getPosition(), true, workQueue);
		break;
	default:
		assert(!"SEND_WORKQUEUE_PACKET error argument."); 
		break;
	}
}
