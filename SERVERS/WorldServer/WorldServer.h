#ifndef WORLD_SERVER_H
#define WORLD_SERVER_H

#include <WinSock2.h>
#include <windows.h>
#include "Event.h"
//#include "Common/LogHelper.h"
#include "CommLib/DBThreadManager.h"
#include "base/WorkQueue.h"
#include "CommLib/ThreadPool.h"
#include <Psapi.h>

#define MONITOR_CONFIGFILE      "../SXZ_SERVER.CFG"
#define MONITOR_COPYMAPDATA     "Data/CopymapRepository.dat"
#define MONITOR_BANNAME         "Data/BanNameRepository.dat"
#define MONITOR_BANWORD         "Data/BanWordRepository.dat"
#define MONITOR_COLLECTITEMDATA "Data/CollectItemRepository.dat"
#define MONITOR_ITEMDATA        "Data/ItemRepository.dat"
#define MONITOR_ORGDEPOTDATA    "Data/OrgDepotData.dat"
#define MONITOR_ORGBOSSDATA     "Data/OrgBossData.dat"
#define MONITOR_PLAYERNAME		"Data/Data_3_Name.dat"
#define MONITOR_SCRIPT_MAINLUA	"Script/main.lua"

#define SERVER_CLASS_NAME CWorldServer

struct stPacketHead;
class DataBase;
class CScript;
class dtServerSocket;
class CAsyncSocket2;
class CWorkQueue;
class CTimerMgr;
class CPlayerManager;
class CServerManager;
class CFriendManager;
class CSavePlayerJob;
class CMyLog;
class CSocialManager;
struct dbStruct;
class CAsyncSocket2;
class CLogHelper;
class CRedisConnPool;
#define SERVER		        SERVER_CLASS_NAME::GetInstance()
#define MEMPOOL		        CMemPool::GetInstance()

extern int g_nopass9;

class SERVER_CLASS_NAME
{
#ifdef _NTJ_UNITTEST
public:
#endif
	static SERVER_CLASS_NAME *	m_pInstance;
	static CScript *			m_pScript;
	static CWorkQueue *			m_pWorkQueue;
	static CWorkQueue*			m_pSocialWorkQueue;

	HANDLE	m_StartEvent;

	int							m_accountSocket;
	bool						m_Inited;
	dtServerSocket	*			m_pServerService;
	CPlayerManager *			m_pPlayerManager;
	CServerManager *			m_pServerManager;
	CSocialManager*				m_pSocialManager;
    CLogHelper*                 m_logHelper;

	DataBase*					m_ActorDB;
	DataBase*					m_BillingDB;
	DataBase*					m_LogDB;
	CRedisConnPool*				m_RedisPool;

	CAsyncSocket2*				m_pRemoteAccountClient;
    
    DBThreadManager*            m_pCommonDBManager;
    DBThreadManager*            m_pQueryDBManager;
    DBThreadManager*			m_pTopDBManager;
	DBThreadManager*			m_pMailDBManager;
	DBThreadManager*			m_pPlayerDBManager;
    CThread*                    m_pMonitorThread;
	CThread*                    m_pGMThread;

	ThreadPool*					m_pUncompressThread;
	CThread*					m_pMonitorMemoryThread;

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
	int            GetAccountSocket()	{		return m_accountSocket; }
	void           SetAccountSocket( int accountSocket )	{ m_accountSocket = accountSocket; }
	dtServerSocket		*GetServerSocket();
	CPlayerManager		*GetPlayerManager();
	CServerManager		*GetServerManager();
	CSocialManager		*GetSocialManager();
	//CZoneEventManager	*GetZoneEventManager();  
	CAsyncSocket2		*GetRemoteAccountSocket();
	static CWorkQueue	*GetWorkQueue();
	static CWorkQueue	*GetSocialWorkQueue();
    CLogHelper			*GetLog();

    //普通逻辑的DB异步线程
    DBThreadManager*    GetCommonDBManager(void) {return m_pCommonDBManager;}

    //一些比较耗时的查询异步操作
    DBThreadManager*    GetQueryDBManager(void) {return m_pQueryDBManager;}

    //返回排行榜线程
    DBThreadManager*	GetTopDBManager(void) {return m_pTopDBManager;}

	//操作元宝交易的线程，改为掉落和救援专用
	DBThreadManager*	GetMailDBManager(void) { return m_pMailDBManager;}

	DBThreadManager*	GetPlayerDBManager(void) { return m_pPlayerDBManager;}

	//排行榜，压缩完整版人物物品数据给客户端
	ThreadPool*  GetUncompressThread() { return m_pUncompressThread; }

    //void Log(dbStruct& log);

	static DataBase* GetActorDB() { return GetInstance()->m_ActorDB;}
	static DataBase* GetBillingDB() { return GetInstance()->m_BillingDB;}
	static DataBase* GetLogDB() { return GetInstance()->m_LogDB;}
	static CRedisConnPool* GetRedisPool() { return GetInstance()->m_RedisPool; }
	static int				EventProcess(LPVOID Param);

	bool Initialize();
	void InitData();
	void StartService();
	void StopService();
	void DenyService();
	void Maintenance();
    bool IsClosed(int runMode);

	int currentAreaId;
	int m_bIsQuit;
	// 测试用------------------------------------------
	int nTestChannel;

	void OnServerStarted();
	void ShowMessage();
};

enum OTHERWORKQUEUE_MSG
{
	OWQ_QueryFriendRequest,
	OWQ_EnterGame,
	OWQ_NotifyZone,
	OWQ_LeaveGame,
	OWQ_UpdateMemberInfo,
	OWQ_GMActorOrg,
	OWQ_GMModifyOrg,
	OWQ_GMOrgChat,
	OWQ_ChangeSocialState,
	OWQ_DistorySocialLink,
	OWQ_CleanupSocialCache,
	OWQ_NotifyTopRank,
	OWQ_SocialNotifyTopRank,
	OWQ_RegisterChat,
	OWQ_ZoneRegister,
	OWQ_GetFamilyTopRank,
	OWQ_ReloadData,
	OWQ_ChangeOnlineState,
	OWQ_NotifyLeagueTopRank,
};

#define MAKE_WORKQUEUE_PACKET(packet, bufSize) \
	char buf##packet[bufSize]; \
	Base::BitStream packet(buf##packet, bufSize);

void SEND_WORKQUEUE_PACKET(Base::BitStream& packet, WORKQUEUE_MESSGAE workQueue, OTHERWORKQUEUE_MSG msg);

#endif
