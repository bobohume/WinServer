// AccountServer.cpp : 定义控制台应用程序的入口点。
//

#include <signal.h>
#include "base/WorkQueue.h"

#include "DBLib/dbPool.h"
#include "Common/Script.h"
#include "Common/CommonServer.h"
#include "Common/CommonClient.h"
#include "Common/TimerMgr.h"
#include "Common/DumpHelper.h"
#include "Common/OrgBase.h"

#include "ZoneServer.h"
#include "ServerPacketProcess.h"
//#include "RemoteAccountPacketProcess.h"
#include "PlayerMgr.h"
#include "common/cfgbuilder.h"
#include "Common/UdpLog.h"
#include "DBLayer/Data/DataBase.h"
#include "UIDGen/UIDGen.h"
#include "CommLib/ServerCloseMonitor.h"
#include "CommLib/ConfigChangeMonitor.h"
#include "Timer/timeSignal.h"
//#include "Common/CheckCodeCache.h"

static TimeSignal<U32> s_TimeSignal;

SERVER_CLASS_NAME *	SERVER_CLASS_NAME::m_pInstance		=	NULL;
CScript *			SERVER_CLASS_NAME::m_pScript		=	NULL;

std::string	GateIp;//网关地址
std::string GatePort;//网关端口号

U32 g_runMode = 0;

SERVER_CLASS_NAME::SERVER_CLASS_NAME()
{
	m_Inited			=	false;
	m_pScript			=	NULL;
	m_pGateClient		=	NULL;
	m_pPlayerManager	=	NULL;
	m_pTimeMgr			=	NULL;
	m_pWorkQueue		=	NULL;
	m_StartEvent		= INVALID_HANDLE_VALUE;

	ISocket::InitNetLib();
}

SERVER_CLASS_NAME::~SERVER_CLASS_NAME()
{
	if(m_pScript)				delete m_pScript;
	if(m_pTimeMgr)				delete m_pTimeMgr;
	if(m_pPlayerManager)		delete m_pPlayerManager;
	if(m_pGateClient)			delete m_pGateClient;
	if(m_pWorkQueue)			delete m_pWorkQueue;

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

void SERVER_CLASS_NAME::ShowMessage()
{
	g_Log.WriteLog("**********************************************************");
	g_Log.WriteLog("\tZoneServer Version:\t%s",G_PVERSION);	
	g_Log.WriteLog("**********************************************************");
}

void SERVER_CLASS_NAME::OnServerStarted()
{
	g_Log.WriteLog("*****************帐号服务已启动****************************");
}

bool SERVER_CLASS_NAME::Initialize()
{
	HeapSetInformation(NULL,HeapEnableTerminationOnCorruption,NULL,0);

	if( !IsUIDMatch() )
    {
        g_Log.WriteError("机器未被授权运行此程序");
		return false;
    }

	if(m_Inited)
		return true;

	if(!InstallBreakHandlers())
		return false;

	//设置日志文件名
	ConfigLog("ZoneLog");

	GateIp			= CFG_BUIILDER->GetMatchHostIP();
	GatePort		= CFG_BUIILDER->GetByHostIP("NetGate_LANPort");

	//执行初始化脚本
	m_pScript = CScript::GetInstance();
	m_pScript->Initialize();

	ShowMessage();

	m_pWorkQueue->Initialize(EventProcess,1,"EventProcess");

	//连接世界服务器
	stAsyncParam aParam;
	strcpy_s(aParam.Name, 64, "GateClient");
	m_pGateClient		=	new CommonClient<ServerPacketProcess>;
	m_pGateClient->Initialize( GateIp.c_str(), atoi( GatePort.c_str() ),&aParam );
	m_pGateClient->SetConnectType(ISocket::SERVER_CONNECT);
   
	m_pTimeMgr = new CTimerMgr(/*60 * */1000);
	m_pPlayerManager = new CPlayerManager;
	m_pTimeMgr->AddObject(m_pPlayerManager);
	m_pTimeMgr->AddObject(&CVersion::GetInstance());
    m_pTimeMgr->Start();

	m_Inited = true;
	return m_Inited;
}

void SERVER_CLASS_NAME::StartService()
{
	if(m_pGateClient) 
    {
        m_pGateClient->Start();
    }
}

int SERVER_CLASS_NAME::EventProcess(LPVOID Param)
{
	WorkQueueItemStruct *pItem = (WorkQueueItemStruct*)Param;

	switch(pItem->opCode)
	{
	case WQ_TIMER:
		{
			char ip[20];
			memcpy(ip, pItem->Buffer, pItem->size);
			ip[pItem->size] = '\0';
			SERVER->GetPlayerManager()->AddIPMap(pItem->Id, ip);
		}
		break;
	case WQ_PACKET:
		{

		}
		break;
	}

	return false;
}

void SERVER_CLASS_NAME::OnServerStart()
{
	if(m_pGateClient)
	{
		if(m_pGateClient->GetState() == ISocket::SSF_CONNECTED)
		{
			char Buf[64];
			Base::BitStream sendPacket( Buf, 64 );
			stPacketHead* pHead = IPacket::BuildPacketHead( sendPacket, ACCOUNT_REMOTE_PlayerCountRequest, 0, SERVICE_REMOTESERVER);
			pHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
			SERVER->GetGateSocket()->Send(sendPacket);
		}
	}
}


void SERVER_CLASS_NAME::Maintenance()
{
    g_runMode = 0;
	ServerCloseMonitor monitor(g_runMode);
	static int getInfoTime = 0;

	while(g_runMode != SERVER_RUN_REQUESTCLOSE)
	{
		if(m_pGateClient)
		{
			if(m_pGateClient->GetState() == ISocket::SSF_DEAD)
			{
				m_pGateClient->Restart();
			}
		}

		int curTime = (int)_time32(NULL);
		if(curTime-getInfoTime > 60)
		{
			if(m_pGateClient->GetState() == ISocket::SSF_CONNECTED)
			{
				char Buf[64];
				Base::BitStream sendPacket( Buf, 64 );
				stPacketHead* pHead = IPacket::BuildPacketHead( sendPacket, ACCOUNT_REMOTE_PlayerCountRequest, 0, SERVICE_REMOTESERVER);
				pHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
				SERVER->GetGateSocket()->Send(sendPacket);

				getInfoTime = curTime;
			}
		}

		if(GetWorkQueue())
		{
			GetWorkQueue()->PostEvent(0,0,0,0,WQ_TIMER);
		}

        Sleep(1);
	}
}

void SERVER_CLASS_NAME::DenyService()
{

}

CWorkQueue* SERVER_CLASS_NAME::GetWorkQueue()		   
{
	IF_ASSERT(m_pWorkQueue == NULL)
		return NULL;
	return m_pWorkQueue;	
}


void SERVER_CLASS_NAME::StopService()
{
	if(m_pGateClient)		m_pGateClient->Stop();
    if(m_pTimeMgr)          m_pTimeMgr->RemoveAll();
	if(m_pWorkQueue)		m_pWorkQueue->Stop();

	if( m_StartEvent != INVALID_HANDLE_VALUE )
		CloseHandle( m_StartEvent );

	g_Log.WriteLog("帐号服务器正常退出");
}










