// WorldServer.cpp : 定义控制台应用程序的入口点。
//

#include <signal.h>
#include "DBLib/dbPool.h"
#include "Common/Script.h"
#include "Common/CommonServer.h"
#include "Common/CommonClient.h"
#include "base/WorkQueue.h"
#include "Common/UDP.h"
#include "common/DataAgentHelper.h"

#include "NetGate.h"
//#include "ServerPacketProcess.h"
#include "EventProcess.h"
#include "UserPacketProcess.h"
#include "UserEventProcess.h"
#include "WorldClientPacketProcess.h"
#include "ChatClientPacketProcess.h"
#include "UDP_Sim.h"

#include "Common/MemoryShare.h"

#include "PlayerMgr.h"
//#include "ServerMgr.h"

#include "encrypt/AuthCrypt.h"
#include "common/cfgbuilder.h"

#include "Common/UdpLog.h"
//#include "encrypt/BigNumber.h"
#include "Common/PlayerStructEx.h"
#include "CommLib/ServerCloseMonitor.h"

SERVER_CLASS_NAME *	SERVER_CLASS_NAME::m_pInstance		=	NULL;
CScript *			SERVER_CLASS_NAME::m_pScript		=	NULL;
CDBConnPool *		SERVER_CLASS_NAME::m_pDBConnPool	=	NULL;
CWorkQueue *		SERVER_CLASS_NAME::m_pWorkQueue		=	NULL;
CWorkQueue *		SERVER_CLASS_NAME::m_pUserWorkQueue	=	NULL;

const char GLOBAL_EXIT_EVENT[] = "md_netgate";

std::string UserCount;
std::string MaxUserCount;
std::string UserNetIP;
std::string UserNetIP2;
std::string UserNetPort;
std::string WorldServerIP;
std::string WorldServerPort;
std::string ChatServerIP;
std::string ChatServerPort;
std::string RemoteCount;
std::string MaxRemoteCount;
std::string RemoteNetIP;
std::string RemoteNetPort;

std::string NetLineId;
std::string NetGateId;
std::string ServerGroup;
U32 g_runMode = 0;

SERVER_CLASS_NAME::SERVER_CLASS_NAME()
{
	m_Inited			=	false;
	m_pServerService	=	NULL;
	m_pUserService		=	NULL;
	m_pWorldClient		=	NULL;
	m_pChatClient		=	NULL;
	m_pDBConnPool		=	NULL;
	m_pScript			=	NULL;
	m_pWorkQueue		=	NULL;
	m_pUserWorkQueue	=	NULL;
	m_pPlayerManager	=	NULL;
	//m_pServerManager	=	NULL;

	m_pUDPServerSide	=	NULL;
	m_pUDPClientSide	=	NULL;

	ISocket::InitNetLib();
}

SERVER_CLASS_NAME::~SERVER_CLASS_NAME()
{
	if(m_pServerService)		delete m_pServerService;
	if(m_pUserService)			delete m_pUserService;
	if(m_pWorldClient)			delete m_pWorldClient;
	if(m_pChatClient)			delete m_pChatClient;
	if(m_pDBConnPool)			delete m_pDBConnPool;
	if(m_pScript)				delete m_pScript;
	if(m_pWorkQueue)			delete m_pWorkQueue;
	if(m_pUserWorkQueue)		delete m_pUserWorkQueue;
	if(m_pPlayerManager)		delete m_pPlayerManager;
	//if(m_pServerManager)		delete m_pServerManager;

	if(m_pUDPServerSide)		delete m_pUDPServerSide;
	if(m_pUDPClientSide)		delete m_pUDPClientSide;

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

/*void *Create_ServerPacketProcess()
{
	return new ServerPacketProcess;
}*/

void SERVER_CLASS_NAME::ShowMessage()
{	
	g_Log.WriteLog("**********************************************************");
	g_Log.WriteLog("\tNetGate Version:\t%s",G_PVERSION);
	//g_Log.WriteLog("*\tUserCount\t\t\t%s", UserCount.c_str());
	//g_Log.WriteLog("*\tMaxUserCount\t\t\t%s", MaxUserCount.c_str());
	g_Log.WriteLog("\tNetGateIP(WAN):\t\t%s:%s", UserNetIP.c_str(), UserNetPort.c_str());
	g_Log.WriteLog("\tNetGateIP(WAN Second):\t\t%s:%s", UserNetIP2.c_str(), UserNetPort.c_str());
	//g_Log.WriteLog("\tRemoteCount\t\t\t%s", RemoteCount.c_str());
	//g_Log.WriteLog("\tMaxRemoteCount\t\t\t%s", MaxRemoteCount.c_str());
	g_Log.WriteLog("\tNetGateIP(LAN):\t\t%s:%s", RemoteNetIP.c_str(), RemoteNetPort.c_str());
	g_Log.WriteLog("\tWorldServerIP(LAN):\t%s:%s", WorldServerIP.c_str(), WorldServerPort.c_str());
	g_Log.WriteLog("\tNetLineId:\t\t%s", NetLineId.c_str() );
	g_Log.WriteLog("\tNetGateId:\t\t%s", NetGateId.c_str());
	g_Log.WriteLog("**********************************************************");
}

void SERVER_CLASS_NAME::OnServerStarted()
{
	g_Log.WriteLog("*****************游戏网关服务已启动************************");

	static EventGuard gateEvent("_gatestarted_");
}


bool SERVER_CLASS_NAME::InitializeDatabase()
{
	return true;
}

bool SERVER_CLASS_NAME::Initialize()
{
	HeapSetInformation(NULL,HeapEnableTerminationOnCorruption,NULL,0);

	if(m_Inited)
		return true;
	
	if(	!InstallBreakHandlers())
		return false;
   
	//设置日志文件名
	ConfigLog("NetGateLog");
	//g_Log.SetLogName("NetGate_");

	//执行初始化脚本
	m_pScript = CScript::GetInstance();
	m_pScript->Initialize();

	NetLineId		= CFG_BUIILDER->GetByHostIP("LineID");
	NetGateId		= CFG_BUIILDER->GetByHostIP("GateID");
	RemoteNetIP		= CFG_BUIILDER->GetMatchHostIP();
	RemoteNetPort	= CFG_BUIILDER->GetByHostIP("NetGate_LANPort");
	MaxUserCount = CFG_BUIILDER->GetByHostIP("NetGate_WANClients");
	char temp[20];
	_itoa_s(atoi(MaxUserCount.c_str())/5, temp, 20, 10);
	UserCount = temp;
	CFG_BUIILDER->GetByHostIP("NetGate_WANAddress", ':', UserNetIP, UserNetPort);

	std::string tmpPort;
	CFG_BUIILDER->GetByHostIP("NetGate_WANAddress2", ':', UserNetIP2, tmpPort);

	CFG_BUIILDER->Get("World_LANAddress", ':', WorldServerIP, WorldServerPort);
	CFG_BUIILDER->Get( "Chat_LANAddress", ':', ChatServerIP, ChatServerPort  );
	RemoteCount = "100";
	MaxRemoteCount = "500";

	LineId = atoi( NetLineId.c_str() );
	GateId = atoi( NetGateId.c_str() );

	SetConsole("netgate", G_PVERSION, UserNetIP, UserNetPort);
	ShowMessage();

#ifdef ENABLE_ENCRPY_PACKET
    //初始化数据包的发送key
    //char keyarray[] = {"sxz12345"};
    //BigNumber key;
    //key.SetBinary((uint8*)keyarray,sizeof(keyarray));
    AuthCrypt::Instance()->SetKey(0);
#endif

	//初始化工作队列
	m_pWorkQueue = new CWorkQueue;
	m_pWorkQueue->Initialize(EventProcess,1,"EventProcess", 100);
	//m_pWorkQueue->Initialize(EventProcess,1,"EventProcess");

	//开发给客户端用的workque
	m_pUserWorkQueue = new CWorkQueue;
	m_pUserWorkQueue->Initialize(UserEventProcess,10,"UserEventProcess", INFINITE);

	//屏蔽地图
	//m_pServerManager = new CServerManager;
	//m_pServerManager->Initialize();

	m_pPlayerManager = new CPlayerManager;
	m_pWorkQueue->GetTimerMgr().notify(m_pPlayerManager, &CPlayerManager::TimeProcess, 100);

	m_StartEvent = INVALID_HANDLE_VALUE;

	//消息处理绑定
	EventFn::Initialize();
	UserEventFn::Initialize();

	stServerParam Param;
	//开放给地图服务器的服务
	//m_pServerService	=	new CommonServer<ServerPacketProcess>;  //CommonServer<>(Create_UserPacketProcess);
	//strcpy(Param.Name,"ServerService");
	//Param.MinClients	=	atoi( RemoteCount.c_str() );
	//Param.MaxClients	=	atoi( MaxRemoteCount.c_str() );
	//m_pServerService->Initialize(  RemoteNetIP.c_str(),atoi(RemoteNetPort.c_str()),&Param);
	//m_pServerService->SetMaxReceiveBufferSize(MAX_PLAYER_STRUCT_SIZE);
	//m_pServerService->SetMaxSendBufferSize(MAX_PLAYER_STRUCT_SIZE);

	//开放给玩家的服务
	m_pUserService		=	new CommonServer<UserPacketProcess>;  //CommonServer<>(Create_UserPacketProcess);
	//m_pUserService->SetIdleTimeout(60*6);						//6分钟空闲连接超时
	m_pUserService->SetNagle(false);
	strcpy(Param.Name,"UserService");
	Param.MinClients	=	atoi( UserCount.c_str());
	Param.MaxClients	=	atoi( MaxUserCount.c_str() );
	//m_pUserService->Initialize(UserNetIP.c_str(),atoi( UserNetPort.c_str() ),&Param);
	//如果绑定了多个地址，则选择any作为监听地址
	m_pUserService->Initialize(("" != UserNetIP2) ? 0 : UserNetIP.c_str(), atoi(UserNetPort.c_str()), &Param);

	m_pUserService->SetMaxReceiveBufferSize(MIN_PACKET_SIZE);
	m_pUserService->SetMaxSendBufferSize(MIN_PACKET_SIZE);

	//连接世界服务器
	stAsyncParam aParam;
	strcpy(aParam.Name,"WorldClient");
	m_pWorldClient		=	new CommonClient<WorldClientPacketProcess>;	//CommonClient<>(Create_WorldClientPacketProcess)
	m_pWorldClient->Initialize(WorldServerIP.c_str(), atoi( WorldServerPort.c_str() ),&aParam);
	m_pWorldClient->SetConnectType(ISocket::SERVER_CONNECT);
	m_pWorldClient->SetMaxReceiveBufferSize(MAX_PACKET_SIZE);
	m_pWorldClient->SetMaxSendBufferSize(MAX_PACKET_SIZE);

	//连接聊天服务器
	strcpy(aParam.Name,"ChatClient");
	m_pChatClient		=	new CommonClient<ChatClientPacketProcess>;	//CommonClient<>(Create_WorldClientPacketProcess)
	m_pChatClient->Initialize(ChatServerIP.c_str(), atoi( ChatServerPort.c_str() ),&aParam);
	m_pChatClient->SetConnectType(ISocket::SERVER_CONNECT);

#ifndef DATA_OVER_TCP
	//UDP转发
#ifdef PACKET_POST_BY_TCP
    m_pUDPClientSide	=	new CUDP_Sim (UserNetIP.c_str(),atoi(UserNetPort.c_str()),CUDPServer::UDP_SIDE_CLIENT);
#else
	m_pUDPClientSide	=	new CUDPServer(UserNetIP.c_str(),atoi(UserNetPort.c_str()),CUDPServer::UDP_SIDE_CLIENT);
#endif

	m_pUDPServerSide	=	new CUDPServer(RemoteNetIP.c_str(),atoi(RemoteNetPort.c_str()),CUDPServer::UDP_SIDE_ZONE);

	m_pUDPClientSide->SetOutServer(m_pUDPServerSide);
	m_pUDPServerSide->SetOutServer(m_pUDPClientSide);

#endif /*DATA_OVER_TCP*/

	m_Inited	= true;
	return m_Inited;
}

#ifdef _NTJ_UNITTEST_PRESS
	void SERVER_CLASS_NAME::StartServiceA()
#else
	void SERVER_CLASS_NAME::StartService()
#endif
{
	if(m_pWorldClient)		m_pWorldClient->Start();
	if(m_pChatClient)		m_pChatClient->Start();
}

void SERVER_CLASS_NAME::OnServerStart()
{
	/*if(m_pServerService)
	{
		if(m_pServerService->GetState() == ISocket::SSF_DEAD)
			m_pServerService->Start();
	}*/

	if(m_pUserService)
	{
		if(m_pUserService->GetState() == ISocket::SSF_DEAD)
		{
			m_pUserService->Start();
			OnServerStarted();

			m_StartEvent = CreateEventA( NULL, FALSE, FALSE, GLOBAL_EXIT_EVENT);

			if(m_pUDPClientSide)	m_pUDPClientSide->Start();
			if(m_pUDPServerSide)	m_pUDPServerSide->Start();
		}
	}
}

void SERVER_CLASS_NAME::Maintenance()
{
    g_runMode = 0;
	ServerCloseMonitor monitor(g_runMode);

	while(SERVER_RUN_REQUESTCLOSE != g_runMode)
	{
		if(m_pWorldClient)
		{
			if(m_pWorldClient->GetState() == ISocket::SSF_DEAD)
			{
				m_pWorldClient->Restart();
			}
		}
 
		if( m_pChatClient )
		{
			if( m_pChatClient->GetState() == ISocket::SSF_DEAD )
			{
				m_pChatClient->Restart();
			}
		}

		int t = time(NULL);
		if(t%10==0)
		{
			int totalSendBytes = /*m_pServerService->m_WaitingSendBytes+m_pServerService->m_PendingSendBytes+*/ \
				m_pUserService->m_WaitingSendBytes+m_pUserService->m_PendingSendBytes+
				m_pWorldClient->m_WaitingSendBytes+m_pWorldClient->m_PendingSendBytes+
				m_pChatClient->m_WaitingSendBytes+m_pChatClient->m_PendingSendBytes;

			if(totalSendBytes > 1024 * 1024 * 10)  //>10M 开始输出
			{
				g_Log.WriteWarn("==10秒统计待处理TCP发送字节数统计============================================");
				//g_Log.WriteWarn("地图服务端口");
				//g_Log.WriteWarn("连接状态 [Total:%d,Accepted:%d,Connected:%d]",m_pServerService->m_TotalNum,m_pServerService->m_AcceptedNum,m_pServerService->m_ConnectedNum);
				//g_Log.WriteWarn("端口发送 [Waiting:%d,Peding:%d]",m_pServerService->m_WaitingSendBytes,m_pServerService->m_PendingSendBytes);
				//g_Log.WriteWarn("---------------------------");
				g_Log.WriteWarn("玩家服务端口");
				g_Log.WriteWarn("连接状态 [Total:%d,Accepted:%d,Connected:%d]",m_pUserService->m_TotalNum,m_pUserService->m_AcceptedNum,m_pUserService->m_ConnectedNum);
				g_Log.WriteWarn("端口发送 [Waiting:%d,Peding:%d]",m_pUserService->m_WaitingSendBytes,m_pUserService->m_PendingSendBytes);
				g_Log.WriteWarn("---------------------------");
				g_Log.WriteWarn("世界服务端口发送 [Waiting:%d,Peding:%d]",m_pWorldClient->m_WaitingSendBytes,m_pWorldClient->m_PendingSendBytes);
				g_Log.WriteWarn("---------------------------");
				g_Log.WriteWarn("聊天服务端口发送 [Waiting:%d,Peding:%d]",m_pChatClient->m_WaitingSendBytes,m_pChatClient->m_PendingSendBytes);
				g_Log.WriteWarn("==总字节数:%d==========================================================",totalSendBytes);
			}
        }

        if (m_PacketLogTimer.CheckTimer())
        {
            try
            {
				//DumpRecvPackets();
				//SERVER->GetServerSocket()->DumpPackets(0);

				DumpUserRecvPackets();
				SERVER->GetUserSocket()->DumpPackets(0);
                
                if (0 != m_pUDPServerSide)
                {
				    g_Log.WriteLog("转发客户端UDP包 成功[%d] 失败[%d]",m_pUDPServerSide->DumpSendPackets(),m_pUDPServerSide->DumpSendErrPackets());
                    g_Log.WriteLog("接收到地图UDP包 成功[%d] 失败[%d]",m_pUDPServerSide->DumpRecvPackets(),m_pUDPServerSide->DumpRecvErrPackets());
                }
			}
            catch(...)
            {
            }
        }

		if (m_TimeTraceTimer.CheckTimer())
		{
			m_pWorldClient->CheckTimeEclipse();
			m_pChatClient->CheckTimeEclipse();
			m_pWorkQueue->CheckTimeEclipse();
			m_pUserWorkQueue->CheckTimeEclipse();
			//m_pServerService->CheckTimeEclipse();
			m_pUserService->CheckTimeEclipse();

            if (0 != m_pUDPServerSide)
            {
			    m_pUDPServerSide->CheckTimeEclipse();
            }
		}
		

        // 时不时的要检查与DataAgent的连接
		//m_dataAgentHelper.Check();

        Sleep(1000);
	}
}

void SERVER_CLASS_NAME::DenyService()
{

}

void SERVER_CLASS_NAME::StopService()
{
	if(m_pServerService)	m_pServerService->Stop();
	if(m_pUserService)		m_pUserService->Stop();
	if(m_pChatClient)		m_pChatClient->Stop();
	if(m_pWorldClient)		m_pWorldClient->Stop();

	if(m_pWorkQueue)		m_pWorkQueue->Stop();
	if(m_pUserWorkQueue)	m_pUserWorkQueue->Stop();

	if(m_pUDPClientSide)	m_pUDPClientSide->Stop();
	if(m_pUDPServerSide)	m_pUDPServerSide->Stop();

	if( m_StartEvent != INVALID_HANDLE_VALUE )
		CloseHandle( m_StartEvent );

	g_Log.WriteLog("网关服务器正常退出");
}