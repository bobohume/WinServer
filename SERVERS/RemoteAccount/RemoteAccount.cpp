
#include "RemoteAccount.h"
#include "Common\CfgBuilder.h"
#include "Common/UdpLog.h"
#include "CommLib/DBThreadManager.h"
#include "Common/OrgBase.h"
#include "Common/RALibDef.h"
#include "CommLib/redis.h"
#include "CommLib/format.h"
#include "BASE/md5.h"
#include "Common/CfgBuilder.h"

std::string ipAddress;
std::string port;
std::string maxClientCount;
std::string minClientCount;

std::string clientipAddress;
std::string clientport;

std::string remoteDBName;
std::string remoteDBIp;
std::string remoteDBUser;
std::string remoteDBPass;

std::string remoteBillDBName;
std::string remoteBillDBIp;
std::string remoteBillDBUser;
std::string remoteBillDBPass;

std::string RedisIp;
std::string RedisPort;
std::string RedisPassword;

std::string IFM_SERVICE_NAME;

unsigned int workThreadCount;

U32 g_runMode = 0;


class ClientPacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

protected:
    bool HandlePacket(const char *pInData,int InSize)
    {
        const stPacketHead *pHead = reinterpret_cast<const stPacketHead *>(pInData);
        const char *pData = pInData + GetHeadSize();

        if(GetMessageCodeSimple("MSG_CONNECT") == pHead->Message)
        {
            CMemGuard Buffer(64 MEM_GUARD_PARAM);
            Base::BitStream SendPacket(Buffer.get(),64);
            stPacketHead *pSendHead = BuildPacketHead(SendPacket,"MSG_CONNECTED");

            m_loginStatus = MSG_CONNECTED;

            SendPacket.writeInt(m_pSocket->GetClientId(),Base::Bit32);
            pSendHead->PacketSize = SendPacket.getPosition()-GetHeadSize();
            Send(SendPacket,PT_SYS);
 
            return true;
        }

        return Parent::HandlePacket(pInData,InSize);
    }

	bool HandleGamePacket(stPacketHead *pHead,int DataSize)
    {
        char *pData = (char*)pHead + GetHeadSize();
        Base::BitStream recv(pData,DataSize - GetHeadSize());
        
        switch(pHead->Message)
        {
        /*case RA_MSG_KICK_ACCOUNT:
            {
                char accountName[256] = {0};
                recv.readString(accountName,sizeof(accountName));
                
                int svrId = recv.readInt(Base::Bit32);

                //向world服务器发送踢人请求
                char buf[256];
                Base::BitStream sendPacket(buf,sizeof(buf));

                stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,RW_KICKACCONT,1);
                
                sendPacket.writeString(accountName,sizeof(accountName));
                sendPacket.writeInt(svrId,Base::Bit32);
                sendPacket.writeInt(m_pSocket->GetClientId(),Base::Bit32);
                sendPacket.writeInt(pHead->Id,Base::Bit32);

                pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
                CRemoteAccount::getInstance()->getServerSocket()->Send(sendPacket);
            }*/
            break;
        };

        return true;
    }
};

#define MD5_KEY "OHE5YJWTN3N8NVEF"
class PurchaseProcedure : public ThreadProcedure
{
public:
	PurchaseProcedure()
	{
		m_isEnded = false;
		m_pDBConn = SERVER->getDB()->Lock();
		LoadDB();
	}

	~PurchaseProcedure(void)
	{
		m_isEnded = true;
	}

	void LoadDB() 
	{
		m_pDBConn->SQLExt("SELECT id, buy_type, output_id, output_count, price from  TBL_RECHARGE");
		while (m_pDBConn->More())
		{
			stOrder stInfo;
			stInfo.id = m_pDBConn->GetInt();
			stInfo.buytype = m_pDBConn->GetInt();
			stInfo.itemid = m_pDBConn->GetInt();
			stInfo.itemcount = m_pDBConn->GetInt();
			stInfo.price = atoi(m_pDBConn->GetString());
			m_OrderMap.insert(std::unordered_map<int, stOrder>::value_type(stInfo.id, stInfo));
		}
	}

	virtual int run(void)
	{
		while (!m_isEnded)
		{
			try
			{
				CRedisDB redisDB(SERVER->GetRedisPool());
				std::vector<std::string> CmdVec;
				std::vector<std::string> KeyVec;

				auto GetCmd = [&](std::string redisTable) {
					//recharge_order
					redisDB.Command("select 2");
					redisDB.Command(fmt::sprintf("hkeys %s", redisTable.c_str()), CmdVec);
					for (auto itr : CmdVec)
					{
						KeyVec.push_back(itr);
					}
				};

				auto ParseCmd = [&](std::string redisTable) {
					for (auto itr : KeyVec)
					{
						std::string cmd = redisDB.Command(fmt::sprintf("hget %s %s", redisTable.c_str(), itr.c_str()));
						std::string sKey = cmd;
						size_t iSize = sKey.find("&");
						if (iSize == std::string::npos)
							continue;

						std::string sId = sKey.substr(0, iSize);
						std::string sTempStr = sKey.substr(iSize + 1);
						iSize = sTempStr.find("&");
						if (iSize == std::string::npos)
							continue;

						std::string sPlayerId = sTempStr.substr(0, iSize);
						std::string sMD5 = sTempStr.substr(iSize + 1);
						std::string sSpe = "&";
						std::string sOldMD5 = itr.c_str() + sSpe + sId + sSpe + sPlayerId + sSpe + MD5_KEY;
						std::string sNewMd5 = GetMD5_32(sOldMD5.c_str(), sOldMD5.length());
						//sNewMd5 = (char*)GetMD5(sOldMD5.c_str(), sOldMD5.length());
						if (sMD5 != sNewMd5) {
							redisDB.Command(fmt::sprintf("hdel %s %s", redisTable.c_str(), itr.c_str()));
							continue;
						}

						S32 nId = atoi(sId.c_str());
						S32 nPlayer = atoi(sPlayerId.c_str());
						if (nId == 0 || nPlayer == 0)
							continue;

						auto itr1 = m_OrderMap.find(nId);
						if (itr1 == m_OrderMap.end())
							continue;

						if (redisTable == "recharge_order") {
							redisDB.Command(fmt::sprintf("hdel %s %s", redisTable.c_str(), itr.c_str()));
							m_pDBConn->SQL(fmt::sprintf("INSERT INTO TBL_PURCHASE([UID], [Order], [Status])VALUES('%s', '%s', 0)", itr.c_str(), cmd.c_str()).c_str());
							m_pDBConn->Exec();

							//群发充值消息
							MAKE_PACKET(sendPacket, "RW_RECHARGE_RESPONSE", nPlayer, SERVICE_WORLDSERVER);
							itr1->second.WriteData(&sendPacket);
							sendPacket.writeString(itr, 50);
							sendPacketHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();
							std::vector<std::pair<int, int> > sockets;
							SERVER->GetWorldSockets(sockets);
							for (auto itr2 : sockets)
							{
								if (itr2.second > 0)
									SERVER->getServerSocket()->Send(itr2.second, sendPacket);
							}
						}
						else if (redisTable == "recharge_order1")
						{
							redisDB.Command(fmt::sprintf("hdel %s %s", redisTable.c_str(), itr.c_str()));
							//群发充值消息
							MAKE_PACKET(sendPacket, "RW_RECHARGE1_RESPONSE", nPlayer, SERVICE_WORLDSERVER);
							itr1->second.WriteData(&sendPacket);
							sendPacket.writeString(itr, 50);
							sendPacketHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();
							std::vector<std::pair<int, int> > sockets;
							SERVER->GetWorldSockets(sockets);
							for (auto itr2 : sockets)
							{
								if (itr2.second > 0)
									SERVER->getServerSocket()->Send(itr2.second, sendPacket);
							}
						}
					}
				};


				GetCmd("recharge_order");
				ParseCmd("recharge_order");
				CmdVec.clear();
				KeyVec.clear();
				GetCmd("recharge_order1");
				ParseCmd("recharge_order1");
			}
			catch (...)
			{
				//m_isEnded = true;
			}

			Sleep(2000);
		}

		m_isEnded = true;
		return 0;
	}

	void Stop(void) { m_isEnded = true; }
private:
	bool m_isEnded;
	CDBConn* m_pDBConn;
	std::unordered_map<int ,stOrder> m_OrderMap;
};

class CodeProcedure : public ThreadProcedure
{
public:
	CodeProcedure()
	{
		m_isEnded = false;
		m_pDBConn = SERVER->getDB()->Lock();
	}

	~CodeProcedure(void)
	{
		m_isEnded = true;
	}

	virtual int run(void)
	{
		while (!m_isEnded)
		{
			try
			{
				CRedisDB redisDB(SERVER->GetRedisPool());
				std::vector<std::string> CmdVec;
				std::vector<std::string> KeyVec;
				redisDB.Command("select 2");
				redisDB.Command("hkeys exchange_order", CmdVec);
				for (auto itr : CmdVec)
				{
					KeyVec.push_back(itr);
				}

				for (auto itr : KeyVec)
				{
					std::string cmd = redisDB.Command(fmt::sprintf("hget exchange_order %s", itr.c_str()));
					std::string sKey = cmd;
					size_t iSize = sKey.find("&");
					if (iSize == std::string::npos)
						continue;

					std::string sPlayerId = sKey.substr(0, iSize);
					std::string sTemStr = sKey.substr(iSize + 1);
					iSize = sTemStr.find("&");
					if (iSize == std::string::npos)
						continue;

					std::string sAward = sTemStr.substr(0, iSize);
					std::string sContent = sAward;
					std::string sMD5 = sTemStr.substr(iSize + 1);
					std::vector<S32> ItemIdVec, ItemNumVec;
					char* tokenPtr = strtok((char*)sAward.c_str(), "|");
					while (tokenPtr != NULL)
					{
						std::string sTempStr = tokenPtr;
						iSize = sTempStr.find(":");
						if (iSize == std::string::npos)
							continue;

						std::string sItemId  = sTempStr.substr(0, iSize);
						std::string sItemNum = sTempStr.substr(iSize + 1);
						ItemIdVec.push_back(atoi(sItemId.c_str()));
						ItemNumVec.push_back(atoi(sItemNum.c_str()));
						tokenPtr = strtok(NULL, "|");
					}

					std::string sSpe = "&";
					std::string sOldMD5 = itr.c_str() + sSpe + sPlayerId + sSpe + sContent + sSpe + MD5_KEY;
					std::string sNewMd5 = GetMD5_32(sOldMD5.c_str(), sOldMD5.length());
					//sNewMd5 = (char*)GetMD5(sOldMD5.c_str(), sOldMD5.length());
					if (sMD5 != sNewMd5) {
						redisDB.Command(fmt::sprintf("hdel exchange_order %s", itr.c_str()));
						continue;
					}

					S32 nPlayer = atoi(sPlayerId.c_str());
					if (nPlayer == 0)
						continue;

					redisDB.Command(fmt::sprintf("hdel exchange_order %s", itr.c_str()));
					m_pDBConn->SQL(fmt::sprintf("INSERT INTO TBL_CodeHistory([UID], [Order], [Status])VALUES('%s', '%s', 0)", itr.c_str(), cmd.c_str()).c_str());
					m_pDBConn->Exec();

					//群发充值消息
					MAKE_PACKET(sendPacket, "RW_CODE_RESPONSE", nPlayer, SERVICE_WORLDSERVER);
					sendPacket.writeInt(ItemIdVec.size(), Base::Bit32);
					auto nSlot = 0;
					for (auto itr = ItemIdVec.begin(); itr != ItemIdVec.end(); ++itr,++nSlot) {
						sendPacket.writeInt(*itr, Base::Bit32);
						sendPacket.writeInt(ItemNumVec[nSlot], Base::Bit32);
					}
					sendPacket.writeString(itr, 50);
					sendPacketHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();
					std::vector<std::pair<int, int> > sockets;
					SERVER->GetWorldSockets(sockets);
					for (auto itr2 : sockets)
					{
						if (itr2.second > 0)
							SERVER->getServerSocket()->Send(itr2.second, sendPacket);
					}
				}
			}
			catch (...)
			{
				//m_isEnded = true;
			}

			Sleep(2000);
		}

		m_isEnded = true;
		return 0;
	}

	void Stop(void) { m_isEnded = true; }
private:
	bool m_isEnded;
	CDBConn* m_pDBConn;
	std::unordered_map<int, stOrder> m_OrderMap;
};

CRemoteAccount::CRemoteAccount()
    :mPlayerManager(NULL)/*, m_StartEvent(NULL)*/, mRemoteID(1), mAreaId(1)
{
    m_pThreadPool    = 0;
	m_pCurlPool		 = 0;
    m_pGoldWorkQueue = 0;

	m_pDBManager		= NULL;
	m_pBillDBManager	= NULL;
    m_pRAService        = 0;
	m_pChangeThread		= NULL;
	m_pCodeThread		= NULL;

	m_300Timer.Start();
	m_600Timer.Start();

	GetPrivateProfileStringA("webservice", "userinfo_url", "", m_webUserInfoUrl, sizeof(m_webUserInfoUrl), GAME_PASS9INI_FILE);
	GetPrivateProfileStringA("webservice", "alipay_url", "", m_webAlipayUrl, sizeof(m_webAlipayUrl), GAME_PASS9INI_FILE);

	GetPrivateProfileStringA("webservice", "serialgift_url", "", m_serialGiftUrl, sizeof(m_serialGiftUrl), GAME_PASS9INI_FILE);
	GetPrivateProfileStringA("webservice", "serialgift_key", "", m_serialGiftKey, sizeof(m_serialGiftKey), GAME_PASS9INI_FILE);
}

CRemoteAccount::~CRemoteAccount()
{
	onShutdown();
}

void CRemoteAccount::_loadParam( t_server_param &param )
{
	param.ipAddr = ipAddress.c_str();
	param.port = atoi( port.c_str() );
	param.param.MaxClients = atoi( maxClientCount.c_str() );
	param.param.MinClients = atoi( minClientCount.c_str() );
	param.workThreadCount = workThreadCount;
}

void CRemoteAccount::ShowMessage()
{
	g_Log.WriteLog("**********************************************************");
	g_Log.WriteLog("\tRemoteServer Version:\t%s", G_PVERSION);
	//g_Log.WriteLog("\tUserCount\t\t\t%s", minClientCount.c_str());
	//g_Log.WriteLog("\tMaxUserCount\t\t\t%s", maxClientCount.c_str());
	g_Log.WriteLog("\tRemoteCountIP(LAN)\t%s:%s", ipAddress.c_str(), port.c_str());
	g_Log.WriteLog("\tAccountDBServer(LAN):\t%s", remoteDBIp.c_str());
	g_Log.WriteLog("\tAccountDBName:\t\t%s", remoteDBName.c_str());
	g_Log.WriteLog("\tBillingDBServer(LAN):\t%s", remoteBillDBIp.c_str());
	g_Log.WriteLog("\tBillingDBName:\t\t%s", remoteBillDBName.c_str());
	g_Log.WriteLog("\tIFM_SERVICE_NAME:\t%s", IFM_SERVICE_NAME.c_str());
	g_Log.WriteLog("\tRemoteID:\t%d", mRemoteID);
	g_Log.WriteLog("\tOperation:\t%s", g_worldServerInfo.operation);
	g_Log.WriteLog("**********************************************************");
}

bool CRemoteAccount::onInit()
{
	ConfigLog("RemoteAccountLog");	

	mPlayerManager   = new CPlayerManager();
	//mOpActivity      = new COpActivity();
	m_pThreadPool    = new ThreadPool(1,1);
	m_pCurlPool		 = new ThreadPool(3,1);
    //m_pGoldWorkQueue = new CWorkQueue();

	minClientCount = "50";
	maxClientCount = "100";
	CFG_BUIILDER->Get("RemoteAccount_LANAddress", ':', ipAddress, port);
    CFG_BUIILDER->Get("RemoteAccount_ClientAddress", ':', clientipAddress, clientport);

	remoteDBIp		= CFG_BUIILDER->Get("RemoteAccountDB_LANIP");
	remoteDBName	= CFG_BUIILDER->Get("RemoteAccountDB_Name");
	remoteDBUser	= CFG_BUIILDER->Get("RemoteAccountDB_UserId");
	remoteDBPass	= CFG_BUIILDER->Get("RemoteAccountDB_Password");

	RedisIp = CFG_BUIILDER->Get("RedisIp");
	RedisPort = CFG_BUIILDER->Get("RedisPort");
	RedisPassword = CFG_BUIILDER->Get("RedisPassword");

	//remoteBillDBIp		= CFG_BUIILDER->Get("RemoteBillingDB_LANIP");
	//remoteBillDBName	= CFG_BUIILDER->Get("RemoteBillingDB_Name");
	//remoteBillDBUser	= CFG_BUIILDER->Get("RemoteBillingDB_UserId");
	//remoteBillDBPass	= CFG_BUIILDER->Get("RemoteBillingDB_Password");

	IFM_SERVICE_NAME = CFG_BUIILDER->Get("IFM_SERVICE_NAME");
	std::string remoteID = CFG_BUIILDER->Get("RemoteID");
	if (remoteID=="" || !(mRemoteID=atoi(remoteID.c_str())))
	{
		mRemoteID = 1;
		g_Log.WriteError("RemoteID configure error.");
	}
	std::string operation = CFG_BUIILDER->Get("Operation");
	if (operation == "")
		g_Log.WriteError("Operation configure error.");
	else
		strcpy_s(g_worldServerInfo.operation, sizeof(g_worldServerInfo.operation), operation.c_str());

	//根据CPU数计算实际线程数
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	workThreadCount = siSysInfo.dwNumberOfProcessors * 2 + 1;

	SetConsole("remote", G_PVERSION, ipAddress, port);
	ShowMessage();

	g_Log.WriteLog("正在初始化数据库连接...");

	//初始化数据库连接
	if(!InitializeDatabase())
		return false;
 
	t_server_param param;
	_loadParam(param);
	
	mWorkQueue->Initialize( _eventProcess, 1,"_eventProcess", 100);
    //m_pGoldWorkQueue->Initialize( _eventProcess,1,"_eventProcess" );
    
	strcpy(param.param.Name,"RemoteServer");
	mServer->Initialize( param.ipAddr, param.port, &param.param );
	mServer->Start();

    stServerParam raParam;
    raParam.MinClients = 10;
    raParam.MaxClients = 10;
    strcpy_s(raParam.Name,64, "ClientService");

    m_pRAService =	new CommonServer<ClientPacketProcess>;

	m_pRAService->Initialize(clientipAddress.c_str(),atoi( clientport.c_str() ),&raParam);
    m_pRAService->Start();

	m_pDBManager = new DBThreadManager;
	m_pDBManager->Start(&mDB, 1, DB_CONTEXT_NOMRAL);

	m_pBillDBManager = new DBThreadManager;
	m_pBillDBManager->Start(&mDB, 1, DB_CONTEXT_NOMRAL);

	PurchaseProcedure* pPurchase = new PurchaseProcedure();
	m_pChangeThread = CThread::Create(pPurchase);
	m_pChangeThread->Resume();

	CodeProcedure* pCode= new CodeProcedure();
	m_pCodeThread = CThread::Create(pCode);
	m_pCodeThread->Resume();

	//mOpActivity->getNetBarInfo();

    if (!m_pThreadPool->Start())
		return false;
	if(!m_pCurlPool->Start())
		return false;

	return true;
}

void CRemoteAccount::onShutdown()
{
	SAFE_DELETE(mPlayerManager);

	if (m_pDBManager)
	{
		m_pDBManager->Stop();
		SAFE_DELETE(m_pDBManager);
	}
	if (m_pBillDBManager)
	{
		m_pBillDBManager->Stop();
		SAFE_DELETE(m_pBillDBManager);
	}

    if (0 != m_pThreadPool)
		m_pThreadPool->Stop();

	if(0 != m_pCurlPool)
		m_pCurlPool->Stop();

    if (0 != m_pGoldWorkQueue)
        m_pGoldWorkQueue->Stop();

	SAFE_DELETE(m_pThreadPool);
	SAFE_DELETE(m_pCurlPool);
    SAFE_DELETE(m_pGoldWorkQueue);
    SAFE_DELETE(m_pRAService);
	SAFE_DELETE(m_pChangeThread);
	SAFE_DELETE(m_pCodeThread);
}

void CRemoteAccount::onMainLoop()
{
    /*if (m_300Timer.CheckTimer())
	{
		getOpActivity()->getNetBarInfo();
	}

	if(m_600Timer.CheckTimer())
	{
		getOpActivity()->checkPromAccount();
	}*/
}

void CRemoteAccount::onTimer(void* param)
{
	void* pData = (void*)param;
}

DBThreadManager* CRemoteAccount::GetDBManager()
{
	JUST_ASSERT(m_pDBManager == NULL);
	return m_pDBManager;
}

DBThreadManager* CRemoteAccount::GetBillDBManager()
{
	JUST_ASSERT(m_pBillDBManager == NULL);
	return m_pBillDBManager;
}

bool CRemoteAccount::InitializeDatabase()
{
	int nCount = workThreadCount + 1;
	bool ret = mDB.Open( nCount, remoteDBIp.c_str(), remoteDBUser.c_str(),
			remoteDBPass.c_str(), remoteDBName.c_str() );
	if(!ret)
	{
		g_Log.WriteFocus("Error:: %s数据库初始化失败!", remoteDBName.c_str());
		return false;
	}
	else
	{
		g_Log.WriteLog("%s数据库初始化成功!", remoteDBName.c_str());


		m_RedisPool = new CRedisConnPool();
		if (!m_RedisPool->Open(5, RedisIp.c_str(), atoi(RedisPort.c_str()), RedisPassword.c_str()))
		{
			g_Log.WriteFocus("Warning:: %s redis初始化失败!", remoteDBIp.c_str());
			return false;
		}

		return true;
	}
}

void CRemoteAccount::onDisconnected(int socketId)
{
    OLD_DO_LOCK(m_cs);

    for (WORLD_SOCKET_MAP::iterator iter = m_sockets.begin(); iter != m_sockets.end(); ++iter)
    {
        if (iter->second == socketId)
        {
            m_sockets.erase(iter);
            return;
        }
    }
}

void CRemoteAccount::AddWorldServer(int areaId,int socketId)
{
    OLD_DO_LOCK(m_cs);

	mAreaId = areaId;
    m_sockets[areaId] = socketId;
}

int  CRemoteAccount::GetWorldSocket(int areaId)
{
    OLD_DO_LOCK(m_cs);

    WORLD_SOCKET_MAP::iterator iter = m_sockets.find(areaId);

    if (iter == m_sockets.end())
        return -1;

    return iter->second;
}

void CRemoteAccount::GetWorldSockets(std::vector<std::pair<int,int> >& sockets)
{
    OLD_DO_LOCK(m_cs);

    for (WORLD_SOCKET_MAP::iterator iter = m_sockets.begin(); iter != m_sockets.end(); ++iter)
    {
        sockets.push_back(std::make_pair(iter->first,iter->second));
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
	HeapSetInformation(NULL,HeapEnableTerminationOnCorruption,NULL,0);

	::CoInitialize(0);
	
	ServerCloseMonitor monitor(g_runMode);
	CCfgBuilder::ParseCfgBuilder(argc, argv);
	CRemoteAccount::getInstance()->init( "RemoteAccount");
	CRemoteAccount::getInstance()->main(g_runMode);
	CRemoteAccount::getInstance()->shutdown();	

	delete CRemoteAccount::getInstance();

    ::CoUninitialize();
	return 0;
}
