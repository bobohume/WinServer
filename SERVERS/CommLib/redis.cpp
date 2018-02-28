#include "redis.h"

//CRedis* m_gRedisMgr = new CRedis();

CRedisException::CRedisException()
{
}

CRedisException::~CRedisException()
{
}

CRedisConn::CRedisConn(CRedisConnPool *pool) :m_pPool(pool), m_pHandle(NULL), m_Port(0)
{
	memset(m_Ip, 0, sizeof(m_Ip));
}

CRedisConn::~CRedisConn()
{
	if(m_pHandle)
		redisFree(m_pHandle);
}

void CRedisConn::Detach()
{
	if (m_pPool)
	{
		m_pPool->Unlock(this);
	}
}

std::string CRedisConn::Command(std::string cmd)
{
	std::string str;
	if (!m_pHandle)
		return "";

	redisReply *pReply = (redisReply *)redisCommand(m_pHandle, cmd.c_str());
	if (!pReply) {
		g_Log.WriteError("Redis do Command [%s] error", cmd.c_str());
		return "";
	}

	str += ParseRedis(pReply);
	freeReplyObject(pReply);
	return str;
}

void CRedisConn::Command(std::string cmd, std::vector<std::string>& CmdVec)
{
	std::string str;
	if (!m_pHandle)
		return;

	redisReply *pReply = (redisReply *)redisCommand(m_pHandle, cmd.c_str());
	if (!pReply) {
		g_Log.WriteError("Redis do Command [%s] error", cmd.c_str());
		return;
	}
	ParseRedis(pReply, CmdVec);
	freeReplyObject(pReply);
}

template<class T>
int CRedisConn::get1(const std::string &Key, T Val)
{
	std::string cmd = "get ";
	cmd += Key;
	if (!m_pHandle)
		return 0;

	redisReply *pReply = (redisReply *)redisCommand(m_pHandle, cmd.c_str());
	if (!pReply) {
		g_Log.WriteError("Redis do Command [%s] error", cmd.c_str());
		return 0;
	}

	if (pReply->type == REDIS_REPLY_ERROR)
	{
		freeReplyObject(pReply);
		return 0;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		memcpy(Val, pReply->str, pReply->len);
	}

	return pReply->len;
}

template<class T>
void CRedisConn::set1(const std::string &Key, T Val, int Len)
{
	std::vector<std::string> vtKey = { Key };
	std::vector<T> vtVal = { Val };
	std::vector<int> vtLen = { Len };
	set2(vtKey, vtVal, vtLen);
}

template<class T>
void CRedisConn::set2(const std::vector<std::string> &vtKey, const std::vector<T> vtVal, const std::vector<int> &vtLen)
{
	if (vtKey.size() != vtVal.size() || vtKey.size() != vtLen.size())
	{
		throw std::runtime_error("Redis error");
	}

	std::vector<const char *> argv(vtKey.size() + vtVal.size() + 1);
	std::vector<size_t> argvlen(vtKey.size() + vtVal.size() + 1);
	int j = 0;

	static char msetcmd[] = "set";
	argv[j] = msetcmd;
	argvlen[j] = sizeof(msetcmd) - 1;
	++j;

	for (int i = 0; i < vtKey.size(); ++i)
	{
		argvlen[j] = vtKey[i].length();
		argv[j] = vtKey[i].c_str();
		j++;

		argvlen[j] = vtLen[i];
		argv[j] = (const char *)vtVal[i];
		j++;
	}

	void *r = redisCommandArgv(m_pHandle, argv.size(), &(argv[0]), &(argvlen[0]));
	if (!r)
		throw std::runtime_error("Redis error");
	freeReplyObject(r);
}

void CRedisConn::SetAddress(const char* Ip, U16 Port)
{
	dStrcpy(m_Ip, 32, Ip);
	m_Port = Port;
}

void CRedisConn::TimeProcess(U32 bExit)
{
	if (m_Timer.CheckTimer() && Command("PING") != "PONG")
	{
		DisConnect();
		Connect(m_Ip, m_Port, 500);
	}
}

bool CRedisConn::Connect(const char* Ip, U16 Port, U32 timeout)
{
	m_Timer.Start();
	SetAddress(Ip, Port);
	struct timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = timeout * 1000;
	m_pHandle = redisConnectWithTimeout(Ip, Port, tv);
	if (m_pHandle->err)
	{
		g_Log.WriteError("Connect to redisServer [%s] faield:%s\n", Ip, m_pHandle->errstr);
		redisFree(m_pHandle);
		m_pHandle = NULL;
		return false;
	}

	//Command("auth 123456");
	return true;
}

void CRedisConn::DisConnect()
{
	if (m_pHandle)
	{
		redisFree(m_pHandle);
		m_pHandle = NULL;
	}
}

std::string CRedisConn::ParseRedis(redisReply *pReply)
{
	if (pReply->type == REDIS_REPLY_ERROR)
		return "Error";
	else if (pReply->type == REDIS_REPLY_STRING)
		return pReply->str;
	else if (pReply->type == REDIS_REPLY_ARRAY)
	{
		std::string str;
		for (auto i = 0; i < pReply->elements; ++i)
		{
			if (pReply->element[i])
			{
				str += ParseRedis(pReply->element[i]);
				str += " ";
			}
		}
		return str;
	}
	else if (pReply->type == REDIS_REPLY_STATUS)
		return pReply->str;
	else if (pReply->type == REDIS_REPLY_INTEGER)
	{
		std::ostringstream ostr;
		ostr << pReply->integer;
		return ostr.str();
	}
	else if (pReply->type == REDIS_REPLY_NIL)
	{
		return "nil";
	}
}

void  CRedisConn::ParseRedis(redisReply *pReply, std::vector<std::string>& CmdVec)
{
	if (pReply->type == REDIS_REPLY_ERROR)
	{
	}
	else if (pReply->type == REDIS_REPLY_STRING)
	{
		CmdVec.push_back(pReply->str);
	}
	else if (pReply->type == REDIS_REPLY_ARRAY)
	{
		for (auto i = 0; i < pReply->elements; ++i)
		{
			if (pReply->element[i])
			{
				std::string str = ParseRedis(pReply->element[i]);
				CmdVec.push_back(str);
			}
		}
	}
	else if (pReply->type == REDIS_REPLY_STATUS)
	{
		CmdVec.push_back(pReply->str);
	}
	else if (pReply->type == REDIS_REPLY_INTEGER)
	{
		std::ostringstream ostr;
		ostr << pReply->integer;
		CmdVec.push_back(ostr.str());
	}
	else if (pReply->type == REDIS_REPLY_NIL)
	{
	}
}

CRedisConnPool::CRedisConnPool()
{
}

CRedisConnPool::~CRedisConnPool()
{
	Close();
}

bool CRedisConnPool::Open(int nConnCount, const char* Ip, S32 Port, const char* Pass)
{
	if (!nConnCount)
		return false;

	m_nConnCount = nConnCount;

	strcpy(m_Ip, Ip);
	m_Port = Port;

	try
	{
		CRedisConn *pObj;
		for (int i = 0; i < m_nConnCount; i++)
		{
			pObj = new CRedisConn(this);
			pObj->Connect(Ip, Port, 1000);
			std::string str = "auth ";
			str += Pass;
			pObj->Command(str.c_str());
			m_pFreeList.push_back(pObj);
		}
	}
	catch (CRedisException& e)
	{
		Close();
		return false;
	}
	catch (...)
	{
		Close();
		return false;
	}
	return true;
}

void CRedisConnPool::Close()
{
	CRedisConn *pObj;

	CLocker lock(m_csFreeList);

	while (m_pFreeList.size() > 0)
	{
		pObj = m_pFreeList.front();
		delete pObj;
		m_pFreeList.pop_front();
	}
}

CRedisConn * CRedisConnPool::Lock()
{
	DWORD dwTotal = 0;
	CLocker lock(m_csFreeList, false);
	while (dwTotal < 30)		//连接池获取连接超时30秒
	{
		lock.Lock();

		if (m_pFreeList.size() > 0)
		{
			CRedisConn* pConn = m_pFreeList.front();
			m_pFreeList.pop_front();
			lock.Unlock();
			pConn->TimeProcess(0);
			return pConn;
		}
		lock.Unlock();

		Sleep(1000);		//等待1秒
		dwTotal++;
	}
	return NULL;
}

bool CRedisConnPool::Unlock(CRedisConn * pConn)
{
	CLocker lock(m_csFreeList);

#ifdef _DEBUG
	for (std::list<CRedisConn *>::iterator iter = m_pFreeList.begin(); iter != m_pFreeList.end(); ++iter)
	{
		if (*iter == pConn)
		{
			assert(0);
		}
	}
#endif

	m_pFreeList.push_front(pConn);
	return true;
}


CRedisDB::CRedisDB(CRedisConnPool* db)
{
	if (db)
	{
		mRedisConn = db->Lock();
		m_isNeedDetach = true;
	}
	else
	{
		mRedisConn = NULL;
		m_isNeedDetach = false;
	}
}

CRedisDB::~CRedisDB()
{
	if (mRedisConn && m_isNeedDetach)
	{
		mRedisConn->Detach();
		mRedisConn = 0;
	}
}

void CRedisDB::AttachConn(CRedisConn* conn)
{
	if (0 == conn)
	{
		assert(0 != conn);
		return;
	}

	assert(0 == mRedisConn);
	mRedisConn = conn;
}

CRedisConn* CRedisDB::GetConn()
{
	if (mRedisConn == NULL)
	{
		throw CRedisException();
	}

	return mRedisConn;
}

std::string CRedisDB::Command(std::string cmd)
{
	if (GetConn())
		return GetConn()->Command(cmd);

	return "";
}

void CRedisDB::Command(std::string cmd, std::vector<std::string>& VecCmd)
{
	if (GetConn())
		GetConn()->Command(cmd, VecCmd);
}

IRedisUnit::IRedisUnit()
{
}

IRedisUnit::~IRedisUnit()
{
}


CRedisUnitMgr::CRedisUnitMgr()
{
}

CRedisUnitMgr::~CRedisUnitMgr()
{
	m_RedisUnitMap.clear();
}

CRedisUnitMgr* CRedisUnitMgr::Instance()
{
	static CRedisUnitMgr s_LocalMgr;
	return &s_LocalMgr;
}

void CRedisUnitMgr::RegisterUnit(std::string sName, IRedisUnit* pRedisUnit)
{
	m_RedisUnitMap.insert(REDISUNIT_MAP::value_type(sName, pRedisUnit));
}

IRedisUnit* CRedisUnitMgr::GetUnit(std::string sName)
{
	auto itr = m_RedisUnitMap.find(sName);
	if (itr != m_RedisUnitMap.end())
	{
		return itr->second;
	}

	return NULL;
}

//-----------------------for example----------------------------//
/*CRedisDB conn(SERVER->GetRedisPool());
HashCmd<stPlayerBaseInfo> RedisHan(&conn);
str = RedisHan.hlen(nPlayerID);*/
//-------------------------------------------------------------//