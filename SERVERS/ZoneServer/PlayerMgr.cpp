#include <winsock2.h>
#include <windows.h>
//#include "Common/CheckCodeCache.h"
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"
#include "ZoneServer.h"
#include "Common/MemGuard.h"
#include "wintcp/AsyncSocket.h"

int CPlayerManager::m_uidSeed = 1;

CPlayerManager::CPlayerManager()
{
}

CPlayerManager::~CPlayerManager()
{

}

void CPlayerManager::AddIPMap(int SocketId,const char* ip)
{
    if (0 == ip)
        return;

    CLocker lock(m_cs);
    m_ips[SocketId] = ip;
}

bool CPlayerManager::GetIp(int SocketId,std::string& ip)
{
    CLocker lock(m_cs);
    
    IPMAP::iterator iter = m_ips.find(SocketId);

    if (iter != m_ips.end())
    {
        ip = iter->second.c_str();
        return true;
    }
    else
    {
        ip = "0.0.0.0";
        return false;
    }
}   

bool CPlayerManager::AddSocketMap(int SocketId,const char *Ip)
{
	CLocker lock(m_cs);

	HashSocketMap::iterator it = m_SocketMap.find(SocketId);
	if(it == m_SocketMap.end())
	{
		stSocketInfo Sinfo;
		Sinfo.AccountId			= 0;
		Sinfo.LoginIP			= inet_addr(Ip);
		m_SocketMap[SocketId]	= Sinfo;
		return true;
	}

	return false;
}

bool CPlayerManager::ReleaseSocketMap(int SocketId,bool postEvent)
{
	CLocker lock(m_cs);

    m_ips.erase(SocketId);

	HashSocketMap::iterator sit = m_SocketMap.find(SocketId);
	if(sit == m_SocketMap.end())
		return true;

	stSocketInfo *psInfo = &sit->second;
	if(psInfo->AccountId)
	{
		HashAccountMap::iterator pit = m_AccountMap.find(psInfo->AccountId);
		if(pit != m_AccountMap.end())
		{
			stAccountInfo *ppInfo = &pit->second;

			// 删除SOCKET信息
			m_ips.erase( ppInfo->socketId );

			//下线就删除该玩家对象
			m_AccountMap.erase(pit);
		}
	}	

	m_SocketMap.erase(sit);

	//if(postEvent)
		//SERVER->GetServerSocket()->PostEvent(dtServerSocket::OP_RESTART,SocketId);

	return false;
}

stSocketInfo *CPlayerManager::GetSocketMap(int SocketId)
{
	HashSocketMap::iterator it = m_SocketMap.find(SocketId);
	if(it == m_SocketMap.end())
		return NULL;

	return &it->second;
}

stAccountInfo *CPlayerManager::AddAccountMap(int SocketId,int AccountId)
{
	CLocker lock(m_cs);
    
    IPMAP::iterator iter = m_ips.find(SocketId);

    if (iter != m_ips.end())
        AddSocketMap(SocketId,iter->second.c_str());
    else
        AddSocketMap(SocketId,"0.0.0.0");

	HashSocketMap::iterator sit = m_SocketMap.find(SocketId);
	if(sit == m_SocketMap.end())
		return NULL;

	stSocketInfo *psInfo = &sit->second;
	if(psInfo->AccountId && psInfo->AccountId!=AccountId)
		return NULL;

	psInfo->AccountId = AccountId;
	HashAccountMap::iterator pit = m_AccountMap.find(psInfo->AccountId);
	if(pit == m_AccountMap.end())
	{
		stAccountInfo pinfo;
		pinfo.LastTime = (int)time(NULL);
		pinfo.AccountId= AccountId;
		pinfo.socketId = SocketId;
		pinfo.UID	   = InterlockedIncrement((LONG *)&m_uidSeed);
		m_AccountMap[psInfo->AccountId] = pinfo;
		return &m_AccountMap[psInfo->AccountId];
	}

	return NULL;
}

bool CPlayerManager::ReleaseAccountMap(int AccountId)
{
	CLocker lock(m_cs);

	HashAccountMap::iterator pit = m_AccountMap.find(AccountId);
	if(pit == m_AccountMap.end())
		return true;
	
	stAccountInfo *ppInfo = &pit->second;
	m_SocketMap.erase(ppInfo->socketId);
	//if(ppInfo->socketId)
	//	SERVER->GetServerSocket()->PostEvent(dtServerSocket::OP_RESTART,ppInfo->socketId);
	m_AccountMap.erase(pit);
	return true;
}

stAccountInfo *CPlayerManager::GetAccountMap(int AccountId)
{
	HashAccountMap::iterator pit = m_AccountMap.find(AccountId);
	if(pit != m_AccountMap.end())
		return &pit->second;

	return NULL;
}

bool CPlayerManager::TimeProcess(bool bExit)
{
	//扫描所有的管理对象，超时的被认为是下线删除
	//int curTime = time(NULL);

	//CLocker lock(m_cs);
	//HashAccountMap::iterator pit = m_AccountMap.begin();
	//while(pit != m_AccountMap.end())
	//{
	//	stAccountInfo *ppInfo = &pit->second;
	//	if(curTime - ppInfo->LastTime > MAX_DROP_TIME)
	//	{
	//		m_AccountMap.erase(pit++);
	//	}
	//	else
	//		pit++;
	//}

	// 过滤验证码

	return true;
}