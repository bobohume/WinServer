#ifndef NET_GATE_H
#define NET_GATE_H

#include "Common/DataAgentHelper.h"
#include "Commlib/SimpleTimer.h"

#include <windows.h>

#define SERVER_CLASS_NAME CNetGate

class CScript;
class dtServerSocket;
class CDBConnPool;
class CAsyncSocket2;
class CWorkQueue;
class CTimerMgr;
class CPlayerManager;
//class CServerManager;
class CUDPServer;
struct WorkQueueItemStruct;
struct stPacketHead;

class CMemoryShare;

extern std::string UserCount;
extern std::string MaxUserCount;
extern std::string UserNetIP;
extern std::string UserNetIP2;
extern std::string UserNetPort;
extern std::string WorldServerIP;
extern std::string WorldServerPort;
extern std::string RemoteCount;
extern std::string MaxRemoteCount;
extern std::string RemoteNetIP;
extern std::string RemoteNetPort;
extern std::string DataAgentIP;
extern std::string DataAgentPort;

extern std::string NetLineId;
extern std::string NetGateId;
extern std::string ServerGroup;

#define SERVER		SERVER_CLASS_NAME::GetInstance()
#define DATABASE	SERVER_CLASS_NAME::GetDBConnPool()
#define MEMPOOL		CMemPool::GetInstance()

class SERVER_CLASS_NAME
{
	static SERVER_CLASS_NAME *	m_pInstance;
	static CScript *			m_pScript;
	static CDBConnPool *		m_pDBConnPool;
	static CWorkQueue *			m_pWorkQueue;
	static CWorkQueue *			m_pUserWorkQueue;

	HANDLE m_StartEvent;

	bool						m_Inited;
	dtServerSocket	*			m_pServerService;
	dtServerSocket	*			m_pUserService;
	CAsyncSocket2	*			m_pWorldClient;
	CAsyncSocket2	*			m_pChatClient;
	CUDPServer		*			m_pUDPServerSide;
	CUDPServer		*			m_pUDPClientSide;
	CPlayerManager *			m_pPlayerManager;
	//CServerManager *			m_pServerManager;

private:
#ifdef _NTJ_UNITTEST_PRESS
public:
#endif
	SERVER_CLASS_NAME();
#ifdef _NTJ_UNITTEST_PRESS
private:
#endif
	bool InstallBreakHandlers();
	bool InitializeDatabase();

	static void OnBreak(int);

public:
	~SERVER_CLASS_NAME();

	static SERVER_CLASS_NAME	*GetInstance()
	{
		if(!m_pInstance)
			m_pInstance = new SERVER_CLASS_NAME;

		return m_pInstance;
	}

	CAsyncSocket2	*GetWorldSocket()	{		return m_pWorldClient;	}	
	CAsyncSocket2	*GetChatSocket()	{		return m_pChatClient;	}
	dtServerSocket	*GetServerSocket()	{		return m_pServerService;}
	dtServerSocket	*GetUserSocket()	{		return m_pUserService;	}
	CPlayerManager  *GetPlayerManager()	{		return m_pPlayerManager;}
	//CServerManager  *GetServerManager()	{		return m_pServerManager;}

	CUDPServer		*GetUDPZoneSide()	{		return m_pUDPServerSide;}




	static CDBConnPool		*GetDBConnPool()	{		return m_pDBConnPool;	}
	static CWorkQueue		*GetWorkQueue()		{		return m_pWorkQueue;	}
	static CWorkQueue		*GetUserWorkQueue()	{		return m_pUserWorkQueue;}
	static int				EventProcess(LPVOID Param);

	void SendToDataAgent( WorkQueueItemStruct * pItem, stPacketHead* pHead );

	static int				UserEventProcess(LPVOID Param);
	int GateId;
	int LineId;

	bool Initialize();
#ifdef _NTJ_UNITTEST_PRESS
	void StartServiceA();
#else
	void StartService();
#endif
	void StopService();
	void DenyService();
	void Maintenance();

	void OnServerStart();

	void OnServerStarted();
	void ShowMessage();

    SimpleTimer<600> m_PacketLogTimer;
	SimpleTimer<30> m_TimeTraceTimer;
};

#endif
