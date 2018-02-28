#ifndef __CHAT_SERVER_H__
#define __CHAT_SERVER_H__

#include "Common/ServerFramwork.h"
#include "WINTCP/AsyncSocket.h"
#include "Commlib/simpleTimer.h"
#include "CommLib/ThreadPool.h"


class CChatManager;
class CPlayerMgr;
class CServerMgr;

#define  MONITOR_SENSWORDFILE     "./SensWords.ini"
#define  MONITOR_SUSWORDFILE     "./SusWords.ini"

enum DBContext
{
	DB_CONTEXT_LOGIC,
	DB_CONTEXT_NOMRAL,
};

class CChatServer : public CServerFramework< CChatServer >
{
public:
	CChatServer();
	bool onInit();
	void onShutdown();
	void onMainLoop();
	void onConfigChange(void* param);
	void onTimer(void* param);

	CChatManager*	getChatMgr()			{ return mChatManager;	}
	CPlayerMgr*		getPlayerMgr()			{ return mPlayerMgr;	}
	CServerMgr*		getServerMgr()			{ return mServerMgr;	}
	CAsyncSocket2*	getWorldSocket()		{ return m_pWorldClient;}
	ThreadPool*		GetThreadPool()			{ return m_pThreadPool; }
	std::string		GetChatLogUrl()			{ return m_ChatLog_Url; }
	void ShowMessage();
	void _loadParam( t_server_param &param );

	int currentAreaId;
#ifndef _SXZ_UNITTEST
private:
#endif
	CChatManager*	mChatManager;
	CPlayerMgr*		mPlayerMgr;
	CServerMgr*		mServerMgr;
	CAsyncSocket2*	m_pWorldClient;
	ThreadPool*		m_pThreadPool;
	std::string		m_ChatLog_Url;
    SimpleTimer<5>  m_serverCheckTimer;	
};

#define SERVER (CChatServer::getInstance())

#endif /*__CHAT_SERVER_H__*/