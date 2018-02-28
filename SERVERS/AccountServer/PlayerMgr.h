#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <string>
#include <hash_map>
#include <Time.h>

#include "Base/Locker.h"
#include "Common/PacketType.h"
#include "DBLayer/Data/TBLAccount.h"

struct stSocketInfo
{
	int AccountId;
	int LoginIP;
};

struct stAccountInfo
{
	int LastTime;
	T_UID UID;
	int socketId;
	int	AccountId;
	int flag;
	char AccountName[ACCOUNT_NAME_LENGTH];
	int GMFlag;
	unsigned int status;
	int logoutTime;
	int dTotalOnlineTime;
	int dTotalOfflineTime;

	stAccountInfo()
	{
		LastTime	 = (int)time(NULL);
		UID			 = 0;
		AccountId	 = 0;
		socketId	 = 0;
		AccountName[0] = 0;
		flag = 0;
	}
};

class CPlayerManager
{
protected:
	typedef stdext::hash_map<int,stSocketInfo> HashSocketMap;
	typedef stdext::hash_map<int,stAccountInfo> HashAccountMap;
    typedef stdext::hash_map<int,std::string>   IPMAP;
	typedef stdext::hash_map<int, std::string> HashIdCodeMap;
	typedef std::list<int>  IdCodeList;

	HashSocketMap  m_SocketMap;
	HashAccountMap m_AccountMap;
    IPMAP          m_ips;//socketid对应ip

#ifdef _SXZ_UNITTEST
public:
#endif

	static int m_uidSeed;
public:
	CMyCriticalSection m_cs;

public:
	CPlayerManager();
	~CPlayerManager();

	void TimeProcess(U32 bExit);
    
    void AddIPMap(int SocketId,const char* ip);
    bool GetIp(int SocketId,std::string& ip);
    
	//玩家连接管理
	bool AddSocketMap(int SocketId,const char *Ip);
	bool ReleaseSocketMap(int SocketId,bool postEvent=false);
	stSocketInfo *GetSocketMap(int SocketId);

	//玩家信息管理
	stAccountInfo *AddAccountMap(int SocketId,int AccountId);
	bool		  ReleaseAccountMap(int AccountId);
	stAccountInfo *GetAccountMap(int AccountId);

#ifdef _SXZ_UNITTEST
	void ClearAll()
	{
		m_SocketMap.clear();
		m_AccountMap.clear();
		m_ips.clear();;
	}

#endif
};

#endif