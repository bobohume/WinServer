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
class dtServerSocket;
class CAsyncSocket2;
class CWorkQueue;
class CTimerMgr;
class CPlayerManager;

extern int g_nopass9;

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
	static CWorkQueue *			m_pWorkQueue;

	bool                     m_loginFlag;
	bool						m_Inited;
	dtServerSocket	*			m_pUserService;
	CAsyncSocket2	*			m_pWorldClient;
	//CAsyncSocket2	*			m_pRemoteAccountClient;
	CPlayerManager *			m_pPlayerManager;
	DataBase*					m_ActorDB;
    DBThreadManager*            m_pCommonDBManager;
    VirtualConsole              m_console;

	typedef std::list< std::pair< int, std::string> > IdCodeList;
	IdCodeList mIdCodeList;
	CMyCriticalSection m_idCodeCs;

private:
	SERVER_CLASS_NAME();

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
	bool		   IsNoPass9();
	int            GetLoginFlag()     {      return m_loginFlag;     } 
	void            SetLoginFlag( bool flag )  { m_loginFlag = flag; }
	CAsyncSocket2	*GetWorldSocket()	{		return m_pWorldClient;	}
	//CAsyncSocket2	*GetRemoteAccountSocket()	{		return m_pRemoteAccountClient;	}
	dtServerSocket	*GetServerSocket()	{		return m_pUserService;	}
	CPlayerManager  *GetPlayerManager()	{		return m_pPlayerManager;}

	static DataBase* GetActorDB() { return GetInstance()->m_ActorDB;}
	static CWorkQueue		*GetWorkQueue()		{		return m_pWorkQueue;	}
	static int				EventProcess(LPVOID Param);
#ifdef _SXZ_UNITTEST // add this method to expose m_Inited in unit test
	bool IsInitialized() {		return m_Inited;	}
#endif
    
    //普通逻辑的DB异步线程
    DBThreadManager*    GetCommonDBManager(void) {return m_pCommonDBManager;}

    int mRemoteId;
	int mAreaId;

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
