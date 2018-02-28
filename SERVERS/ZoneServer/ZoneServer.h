#ifndef ACCOUNT_SERVER_H
#define ACCOUNT_SERVER_H

#include <windows.h>
#include <assert.h>
#include <string>
#include <list>
#include "Base\Locker.h"
#include "CommLib/DBThreadManager.h"

#include "CommLib/VirtualConsole.h"

#define SERVER_CLASS_NAME CAccountServer


#define MONITOR_CONFIGFILE "../SXZ_SERVER.CFG"

class CScript;
class DataBase;
class CAsyncSocket2;
class CWorkQueue;
class CTimerMgr;
class CPlayerManager;

#define SERVER		SERVER_CLASS_NAME::GetInstance()
#define MEMPOOL		CMemPool::GetInstance()

enum
{
	LOGIN_COUNT_PRE_MINUTE = 1000,
};

class SERVER_CLASS_NAME
{
	HANDLE m_StartEvent;
	static SERVER_CLASS_NAME *	m_pInstance;
	static CScript *			m_pScript;

	bool						m_Inited;
	CAsyncSocket2	*			m_pGateClient;
	CTimerMgr *					m_pTimeMgr;
	CWorkQueue*					m_pWorkQueue;
	CPlayerManager *			m_pPlayerManager;
    VirtualConsole              m_console;

private:
	SERVER_CLASS_NAME();

	bool InstallBreakHandlers();

	static void OnBreak(int);

public:
	~SERVER_CLASS_NAME();

	static int	EventProcess(LPVOID Param);

	static SERVER_CLASS_NAME	*GetInstance()
	{
		if(!m_pInstance)
			m_pInstance = new SERVER_CLASS_NAME;

		return m_pInstance;
	}
	CAsyncSocket2	*GetGateSocket()	{		return m_pGateClient;	}
	CPlayerManager  *GetPlayerManager()	{		return m_pPlayerManager;}
	CWorkQueue		*GetWorkQueue()		{		return m_pWorkQueue; }

#ifdef _SXZ_UNITTEST // add this method to expose m_Inited in unit test
	bool IsInitialized() {		return m_Inited;	}
#endif
    

	bool Initialize();
	void StartService();
	void StopService();
	void DenyService();
	void Maintenance();

	void OnServerStart();

	void OnServerStarted();
	void ShowMessage();
};

#endif
