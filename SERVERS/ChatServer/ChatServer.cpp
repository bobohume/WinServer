#include "ChatServer.h"
#include "ServerMgr.h"
#include "PlayerMgr.h"
#include "..\WorldServer\ChatMgr.h"
#include "Common\CfgBuilder.h"
#include "Common\CommonClient.h"
#include "WorldPacketProcess.h"
#include <time.h>
#include "Common\UdpLog.h"
#include "Common\BanInfoData.h"
#include "CommLib\ConfigChangeMonitor.h"
#include "Common\build_no.h"
#include "CommLib\CommLib.h"

std::string WorldServerIP;
std::string WorldServerPort;
std::string ipAddress;
std::string port;
std::string maxClientCount;
std::string minClientCount;
std::string workThreadCount;
std::string AreaId;

U32 g_runMode = 0;

CChatServer::CChatServer() : currentAreaId(0)
{
	m_pThreadPool = 0;
}

void CChatServer::_loadParam(t_server_param &param)
{
	param.ipAddr = ipAddress.c_str();
	param.port = atoi(port.c_str());
	param.param.MaxClients = atoi(maxClientCount.c_str());
	param.param.MinClients = atoi(minClientCount.c_str());
	param.workThreadCount = atoi(workThreadCount.c_str());
}

void CChatServer::ShowMessage()
{
	g_Log.WriteLog("**********************************************************");
	g_Log.WriteLog("\tChatServer Version:\t%s", G_PVERSION);
	g_Log.WriteLog("\tChatServerIP(LAN):\t%s:%s", ipAddress.c_str(), port.c_str());
	g_Log.WriteLog("\tWorldServerIP(LAN):\t%s:%s", WorldServerIP.c_str(), WorldServerPort.c_str());
	g_Log.WriteLog("**********************************************************");
}

bool CChatServer::onInit()
{
	//设置日志文件名
	ConfigLog("ChatLog");

	m_pWorldClient = NULL;

	mPlayerMgr = new CPlayerMgr();
	mServerMgr = new CServerMgr();
	mChatManager = new CChatManager();
	mChatManager->Initialize();

	CFG_BUIILDER->Get("World_LANAddress", ':', WorldServerIP, WorldServerPort);
	CFG_BUIILDER->Get("Chat_LANAddress", ':', ipAddress, port);
	AreaId = CFG_BUIILDER->Get("AreaID");
	m_ChatLog_Url = CFG_BUIILDER->Get("ChatLog_Url");
	currentAreaId = atoi(AreaId.c_str());
	workThreadCount = "5";
	minClientCount = "100";
	maxClientCount = "500";

	SetConsole("chat", G_PVERSION, ipAddress, port);
	ShowMessage();

	mChatManager->readFile(MONITOR_SENSWORDFILE, ChatBanInfo::SENS);
	mChatManager->readFile(MONITOR_SUSWORDFILE, ChatBanInfo::SUSP);


	t_server_param param;
	_loadParam(param);

	mWorkQueue->Initialize(_eventProcess, 1, "_eventProcess", 100);
	//mWorkQueue->Initialize( _eventProcess, param.workThreadCount,"_eventProcess" );
	dStrcpy(param.param.Name, sizeof(param.param.Name), "charServer");
	mServer->Initialize(param.ipAddr, param.port, &param.param);
	mServer->Start(); // start the service

	stAsyncParam aParam;
	dStrcpy(aParam.Name, sizeof(aParam.Name), "WorldClient");
	m_pWorldClient = new CommonClient< WorldClientPacketProcess >();
	m_pWorldClient->Initialize(WorldServerIP.c_str(), atoi(WorldServerPort.c_str()), &aParam);
	m_pWorldClient->SetConnectType(ISocket::SERVER_CONNECT);

	m_pWorldClient->Start();
	m_pThreadPool = new ThreadPool(1, 1);
	if (!m_pThreadPool->Start())
		return false;

	return true;
}

void CChatServer::onShutdown()
{
	/*while( m_pWorldClient->GetState() != ISocket::SSF_DEAD )
	{
	Sleep( 1000 );
	}*/

	if (m_pWorldClient)
	{
		m_pWorldClient->Stop();
	}

	if (0 != m_pThreadPool)
		m_pThreadPool->Stop();

	SAFE_DELETE(m_pThreadPool);
	delete mChatManager;
	delete mPlayerMgr;
	delete mServerMgr;
}

void CChatServer::onMainLoop()
{
	if (m_serverCheckTimer.CheckTimer())
	{
		if (m_pWorldClient->GetState() == ISocket::SSF_DEAD)
		{
			m_pWorldClient->Restart();
		}

		getChatMgr()->Update();
	}
}

void CChatServer::onConfigChange(void* param)
{
	char* fileName = (char*)param;

	//重新载入配置文件
	if (0 == _stricmp(fileName, MONITOR_SENSWORDFILE))
	{
		mChatManager->readFile(MONITOR_SENSWORDFILE, ChatBanInfo::SENS);
	}
	else if (0 == _stricmp(fileName, MONITOR_SUSWORDFILE))
	{
		mChatManager->readFile(MONITOR_SUSWORDFILE, ChatBanInfo::SUSP);
	}

}

void CChatServer::onTimer(void* param)
{
	void* pData = (void*)param;
}

#include <WinSock2.h>

void main(int argc, char* argv[])
{
	NetLibInit netInit;

	ServerCloseMonitor monitor(g_runMode);
	CCfgBuilder::ParseCfgBuilder(argc, argv);
	CChatServer::getInstance()->init("ChatServer");

	ConfigChangeMonitor configMonitor;

	//设置监控目录
	std::vector<std::string> files;
	files.push_back(MONITOR_SENSWORDFILE);
	files.push_back(MONITOR_SUSWORDFILE);
	configMonitor.Start(SERVER->getWorkQueue(), files);

	CChatServer::getInstance()->main(g_runMode);
	CChatServer::getInstance()->shutdown();

	delete CChatServer::getInstance();
	return;
}