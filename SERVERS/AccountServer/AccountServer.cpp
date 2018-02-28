// AccountServer.cpp : 定义控制台应用程序的入口点。
//

#include <signal.h>
#include "base/WorkQueue.h"

#include "DBLib/dbPool.h"
#include "Common/Script.h"
#include "Common/CommonServer.h"
#include "Common/CommonClient.h"
#include "Common/DumpHelper.h"


#include "AccountServer.h"
#include "UserPacketProcess.h"
#include "WorldClientPacketProcess.h"
//#include "RemoteAccountPacketProcess.h"
#include "EventProcess.h"

#include "PlayerMgr.h"
#include "common/cfgbuilder.h"
#include "Common/UdpLog.h"
#include "DBLayer/Data/DataBase.h"
#include "CommLib/ServerCloseMonitor.h"
#include "CommLib/ConfigChangeMonitor.h"

//#include "Common/CheckCodeCache.h"

SERVER_CLASS_NAME *	SERVER_CLASS_NAME::m_pInstance		=	NULL;
CScript *			SERVER_CLASS_NAME::m_pScript		=	NULL;
CWorkQueue *		SERVER_CLASS_NAME::m_pWorkQueue		=	NULL;

std::string UserCount;
std::string MaxUserCount;
std::string UserNetIP;
std::string UserNetPort;
std::string WorldServerIP;
std::string WorldServerPort;
std::string RemoteAccountIP;
std::string RemoteAccountPort;
std::string AreaId;

std::string MaxDBConnection;
std::string DB_Server;
std::string DB_Name;
std::string DB_UserId;
std::string DB_Password;

int g_nopass9 = 1;
U32 g_runMode = 0;

const char GLOBAL_EXIT_EVENT[] = "md_accountserver";

SERVER_CLASS_NAME::SERVER_CLASS_NAME()
{
	m_Inited			=	false;
	m_pUserService		=	NULL;
	m_pScript			=	NULL;
	m_pWorkQueue		=	NULL;
	m_pWorldClient		=	NULL;
	m_pPlayerManager	=	NULL;
	//m_pRemoteAccountClient = NULL;
	m_ActorDB			=	NULL;
	m_loginFlag        =   true;
	m_StartEvent		= INVALID_HANDLE_VALUE;
    m_pCommonDBManager  = 0;

	ISocket::InitNetLib();
}

SERVER_CLASS_NAME::~SERVER_CLASS_NAME()
{
    SAFE_DELETE(m_pCommonDBManager);

	if(m_pUserService)			delete m_pUserService;
	if(m_ActorDB)				delete m_ActorDB;
	if(m_pScript)				delete m_pScript;
	if(m_pWorkQueue)			delete m_pWorkQueue;
	if(m_pPlayerManager)		delete m_pPlayerManager;
	if(m_pWorldClient)			delete m_pWorldClient;

	//CheckCodeCache::GetInstancePtr()->Dispose();
	//CheckCodeCache::FreeInstance();

	m_pInstance = NULL;
	ISocket::UninitNetLib();
}

void SERVER_CLASS_NAME::OnBreak(int)
{
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

void SERVER_CLASS_NAME::ShowMessage()
{
	g_Log.WriteLog("**********************************************************");
	g_Log.WriteLog("\tAccountServer Version:\t%s",G_PVERSION);
	//g_Log.WriteLog("\tMinUserCount\t\t\t%s", UserCount.c_str());
	//g_Log.WriteLog("\tMaxUserCount\t\t\t%s", MaxUserCount.c_str());
	g_Log.WriteLog("\tAccountServerIP(LAN):\t%s:%s", UserNetIP.c_str(), UserNetPort.c_str());		
	g_Log.WriteLog("\tWorldServerIP(LAN):\t%s:%s", WorldServerIP.c_str(), WorldServerPort.c_str());
	g_Log.WriteLog("\tActorDBServer(LAN):\t%s", DB_Server.c_str());
	g_Log.WriteLog("\tActorDBName:\t\t%s", DB_Name.c_str());
	//g_Log.WriteLog("\tAccountDB_UserId\t\t\t%s", DB_UserId.c_str());
	//g_Log.WriteLog("\tAccountDB_Password\t\t\t%s", DB_Password.c_str());		
	g_Log.WriteLog("**********************************************************");
}

void SERVER_CLASS_NAME::OnServerStarted()
{
	g_Log.WriteLog("*****************帐号服务已启动****************************");
}

bool SERVER_CLASS_NAME::InitializeDatabase()
{
	if(!m_pScript)
		return false;

	int nCount = atoi( MaxDBConnection.c_str() );
	const char* szServer = DB_Server.c_str();
	const char* szUserId = DB_UserId.c_str();
	const char* szPassword = DB_Password.c_str();
	const char* szDBName = DB_Name.c_str();

	m_ActorDB = new DataBase;
	DBError err = m_ActorDB->Initialize(szServer,szDBName,szUserId,szPassword,nCount);
	if(err != DBERR_NONE)
	{
		g_Log.WriteFocus("Warning:: %s数据库初始化失败!", szDBName);
		return false;
	}
	else
	{
		g_Log.WriteLog("[%s]数据库初始化成功!", szDBName);
		return true;
	}
}

bool SERVER_CLASS_NAME::IsNoPass9()
{
	return (g_nopass9 != 0)? true: false;
}

bool SERVER_CLASS_NAME::Initialize()
{
	HeapSetInformation(NULL,HeapEnableTerminationOnCorruption,NULL,0);

	if(m_Inited)
		return true;

	if(!InstallBreakHandlers())
		return false;

	//设置日志文件名
	ConfigLog("AccountLog");

	MaxUserCount = CFG_BUIILDER->Get("Account_WANClients");
	char temp[20];
	_itoa_s(atoi(MaxUserCount.c_str())/5, temp, 20, 10);
	UserCount = temp;
	CFG_BUIILDER->Get("Account_WANAddress", ':', UserNetIP, UserNetPort);
	CFG_BUIILDER->Get("World_LANAddress", ':', WorldServerIP, WorldServerPort);
	CFG_BUIILDER->Get("RemoteAccount_LANAddress", ':', RemoteAccountIP, RemoteAccountPort);
	AreaId			= CFG_BUIILDER->Get("AreaID");
    mRemoteId	    = atoi(CFG_BUIILDER->Get("RemoteID").c_str());
	DB_Server		= CFG_BUIILDER->Get("ActorDB_LANIP");
	DB_Name			= CFG_BUIILDER->Get("ActorDB_Name");
	DB_UserId		= CFG_BUIILDER->Get("ActorDB_UserId");
	DB_Password		= CFG_BUIILDER->Get("ActorDB_Password");
	std::string nopass9 = CFG_BUIILDER->Get("NoPass9").c_str();
	if (nopass9 != "")
	{
		g_nopass9	= atoi(nopass9.c_str());	
	}

	MaxDBConnection	= "17";//8cpu*2+1

	//执行初始化脚本
	m_pScript = CScript::GetInstance();
	m_pScript->Initialize();

	SetConsole("account", G_PVERSION, UserNetIP, UserNetPort);
	ShowMessage();

	if(g_nopass9 == 1)
	{
		g_Log.WriteFocus("特别关注: nopass9=1仅适用于内部测试,正式上线前请设置nopass9=0并且清空重置角色数据库数据.");
		g_Log.WriteFocus("特别关注: nopass9=1仅适用于内部测试,正式上线前请设置nopass9=0并且清空重置角色数据库数据.");
		g_Log.WriteFocus("特别关注: nopass9=1仅适用于内部测试,正式上线前请设置nopass9=0并且清空重置角色数据库数据.");
	}

	//批量生成校验码
	//CheckCodeCache::GetInstancePtr()->Init();

    g_Log.WriteLog("正在初始化数据库连接...");

	//初始化数据库连接
	if(!InitializeDatabase())		//用于模拟帐号验证平台
		return false;

	mAreaId = atoi( AreaId.c_str() );

	//初始化工作队列
	m_pWorkQueue = new CWorkQueue;
	m_pWorkQueue->Initialize(EventProcess,1,"EventProcess", 100);
	//m_pWorkQueue->Initialize(EventProcess,10,"EventProcess");

	//消息处理绑定
	EventFn::Initialize();

	stServerParam Param;
	//开放给用户的服务
	m_pUserService		=	new CommonServer<UserPacketProcess>;  //CommonServer<>(Create_UserPacketProcess);
	m_pUserService->SetIdleTimeout(60*2);					//2分钟空闲连接超时
	strcpy_s(Param.Name, 64, "UserService");
	Param.MinClients	=	atoi( UserCount.c_str() );
	Param.MaxClients	=	atoi( MaxUserCount.c_str() );

	char ip[COMMON_STRING_LENGTH] = "";
	strcpy_s(ip, sizeof(ip), UserNetIP.c_str());
	_strupr_s(ip, sizeof(ip));
	if (strcmp(ip, "ANY")==0 || strcmp(ip, "")==0)
		m_pUserService->Initialize(NULL, atoi(UserNetPort.c_str()), &Param);
	else
		m_pUserService->Initialize(UserNetIP.c_str(), atoi(UserNetPort.c_str()), &Param);
	//Ray: 将开始包大小缩减到MIN_PACKET_SIZE，这样可以将AcceptEx多投放N倍用来抵御无效链接和查询链接
	m_pUserService->SetMaxSendBufferSize(MIN_PACKET_SIZE);
	m_pUserService->SetMaxReceiveBufferSize(MIN_PACKET_SIZE);

	//连接世界服务器
	stAsyncParam aParam;
	strcpy_s(aParam.Name, 64, "WorldClient");
	m_pWorldClient		=	new CommonClient<WorldClientPacketProcess>;	//CommonClient<>(Create_WorldClientPacketProcess)
	m_pWorldClient->Initialize( WorldServerIP.c_str(), atoi( WorldServerPort.c_str() ),&aParam );
	m_pWorldClient->SetConnectType(ISocket::SERVER_CONNECT);
   
	m_pPlayerManager = new CPlayerManager;
	m_pWorkQueue->GetTimerMgr().notify(m_pPlayerManager, &CPlayerManager::TimeProcess, 100);
	m_pWorkQueue->GetTimerMgr().notify(&CVersion::GetInstance(), &CVersion::TimeProcess, 10*60*1000);

    m_pCommonDBManager = new DBThreadManager;
	m_pCommonDBManager->Start(m_ActorDB->GetPool(),1,0);

	m_Inited = true;
	return m_Inited;
}

void SERVER_CLASS_NAME::StartService()
{
	if(m_pWorldClient) 
    {
        m_pWorldClient->Start();
    }
}

void SERVER_CLASS_NAME::OnServerStart()
{
	if(m_pUserService)
	{
		if(m_pUserService->GetState() == ISocket::SSF_DEAD)
		{
			m_pUserService->Start();
			OnServerStarted();
			m_StartEvent = CreateEventA( NULL, FALSE, FALSE, GLOBAL_EXIT_EVENT);
		}
	}

	if(m_pWorldClient)
	{
		if(m_pWorldClient->GetState() == ISocket::SSF_CONNECTED)
		{
			char Buf[64];
			Base::BitStream sendPacket( Buf, 64 );
			stPacketHead* pHead = IPacket::BuildPacketHead( sendPacket, "ACCOUNT_REMOTE_PlayerCountRequest", 0, SERVICE_REMOTESERVER);
			pHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
			SERVER->GetWorldSocket()->Send(sendPacket);
		}
	}
}


void SERVER_CLASS_NAME::Maintenance()
{
    g_runMode = 0;
	ServerCloseMonitor monitor(g_runMode);
	static int getInfoTime = 0;

    ConfigChangeMonitor configMonitor;
    std::vector<std::string> files;
	files.push_back(MONITOR_CONFIGFILE);

    configMonitor.Start(SERVER->GetWorkQueue(),files);

	while(g_runMode != SERVER_RUN_REQUESTCLOSE)
	{
		if(m_pWorldClient)
		{
			if(m_pWorldClient->GetState() == ISocket::SSF_DEAD)
			{
				m_pWorldClient->Restart();
			}
		}

		int curTime = (int)_time32(NULL);
		if(curTime-getInfoTime > 60)
		{
			if(m_pWorldClient->GetState() == ISocket::SSF_CONNECTED)
			{
				char Buf[64];
				Base::BitStream sendPacket( Buf, 64 );
				stPacketHead* pHead = IPacket::BuildPacketHead( sendPacket, "ACCOUNT_REMOTE_PlayerCountRequest", 0, SERVICE_REMOTESERVER);
				pHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
				SERVER->GetWorldSocket()->Send(sendPacket);

				getInfoTime = curTime;
			}
		}

        Sleep(2000);
	}
}

void SERVER_CLASS_NAME::DenyService()
{

}

void SERVER_CLASS_NAME::StopService()
{
	if(m_pUserService)		m_pUserService->Stop();
	if(m_pWorldClient)		m_pWorldClient->Stop();
	if(m_pWorkQueue)		m_pWorkQueue->Stop();

	if (0 != m_pCommonDBManager)
	{
		m_pCommonDBManager->Stop();
	}

	if( m_StartEvent != INVALID_HANDLE_VALUE )
		CloseHandle( m_StartEvent );

	g_Log.WriteLog("帐号服务器正常退出");
}










