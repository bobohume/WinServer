#pragma once

#include "Common/ServerFramwork.h"
#include "wintcp/AsyncSocket.h"
//#include "Common/LogHelper.h"
#include "PlayerManager.h"
#include "dblib/dbLib.h"
#include "CommLib/SimpleTimer.h"

#include "CommLib/ThreadPool.h"
#include "CommLib/PacketFactoryManager.h"
#include "PollAccount.h"

#define UPDATETIME	300         //更新vip网吧ip时间 5分钟

enum DBContext
{
	DB_CONTEXT_LOGIC,  
	DB_CONTEXT_NOMRAL,
};

class CPlayerManager;
class DBThreadManager;
class CRedisConnPool;

class CRemoteAccount : public CServerFramework< CRemoteAccount >
{
public:
	CRemoteAccount();
	~CRemoteAccount();
	void _loadParam( t_server_param &param );
	void ShowMessage();
	bool onInit();
	void onShutdown();
	void onMainLoop();
	void onTimer(void* param);

	CDBConnPool* getDB() { return &mDB; }
	CDBConnPool* getBillDB() { return &mBillDB;}
	CPlayerManager* getPlayerManager() { return mPlayerManager; }
	int GetRemoteID() { return mRemoteID; }
	char* getUserInfoUrl() { return m_webUserInfoUrl; }
	char* getAlipayUrl() { return m_webAlipayUrl; }
	char* getSerialGiftUrl() { return m_serialGiftUrl; }
	char* getSerialGiftKey() { return m_serialGiftKey; }

	DBThreadManager* GetDBManager();
	DBThreadManager* GetBillDBManager();
    
    ThreadPool* GetThreadPool(void) {return m_pThreadPool;}
	ThreadPool* GetCurlPool()		{return m_pCurlPool; }

    virtual bool HandleGamePacket(stPacketHead *pHead,int iSize)
	{
        /*if (WA_DRAWGOLD_REQUEST == pHead->Message ||
            WA_QUERYACCOUNTGOLD_REQUEST == pHead->Message)
        {
            if (0 != m_pGoldWorkQueue)
            {
                m_pGoldWorkQueue->PostEvent(GetSocket()->GetClientId(),pHead,iSize,true);
                return true;
            }
        }*/

        getInstance()->getWorkQueue()->PostEvent(GetSocket()->GetClientId(),pHead,iSize,true);
		return true;
	}

	virtual bool onLogic(void* param)
	{
		bool bNeedRemove = true;
		ThreadBase* pDBHandle = (ThreadBase*)param;

		_MY_TRY
		{
			U32 uret = pDBHandle->Execute(DB_CONTEXT_LOGIC, 0, 0, 0);
			if(uret == PACKET_NOTREMOVE)
				bNeedRemove = false;
		}
		_MY_CATCH
		{
			g_Log.WriteError("WQ_LOGIC 执行异常");
		}

		if(bNeedRemove)
			SAFE_REMOVE_PACKET(pDBHandle);

		return true;
	}
    
    dtServerSocket* GetRAService(void) {return m_pRAService;}

    virtual void onDisconnected(int socketId);

    void AddWorldServer(int areaId,int socketId);
    int  GetWorldSocket(int areaId);
	int  GetAreaaId() {return mAreaId; };
    void GetWorldSockets(std::vector<std::pair<int,int> >& sockets);
	CRedisConnPool* GetRedisPool() { return m_RedisPool; }
protected:
	bool InitializeDatabase();
	CPlayerManager* mPlayerManager;
	//HANDLE m_StartEvent;
	CDBConnPool mDB;			//SXZ_ACCOUNTDB
	CDBConnPool mBillDB;		//SXZ_BILLINGDB
	int mRemoteID;

    dtServerSocket* m_pRAService;
    
    //给元宝操作1条独立的线程,防止多线程引起的包顺序导致DB操作的错误
    CWorkQueue* m_pGoldWorkQueue;
    
    //remote account的多线程池,用来处理webservice等操作
	//暂时负责内购
	ThreadPool* m_pThreadPool;
	//负责内购处理curl占用的资源
	ThreadPool* m_pCurlPool;

	CThread*    m_pChangeThread;
	CThread*	m_pCodeThread;
	CRedisConnPool*	m_RedisPool;

	DBThreadManager* m_pDBManager;
	DBThreadManager* m_pBillDBManager;

	char m_webUserInfoUrl[URL_STRING_LENGTH];
	char m_webAlipayUrl[URL_STRING_LENGTH];

	char m_serialGiftUrl[URL_STRING_LENGTH];
	char m_serialGiftKey[PASSWORD_LENGTH];

	SimpleTimer<300>  m_300Timer;
	SimpleTimer<600>  m_600Timer;

    CMyCriticalSection m_cs;

    typedef stdext::hash_map<int,int> WORLD_SOCKET_MAP;
    WORLD_SOCKET_MAP m_sockets;
	int mAreaId;
};

#define SERVER CRemoteAccount::getInstance()
