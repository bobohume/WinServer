#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <string>
#include <hash_map>
#include <list>

#include "Base/Locker.h"
#include "LockCommon.h"
#include "Common/ZoneSetting.h"
#include "commlib/simpleTimer.h"
#include "commlib/TimeRange.h"

#define MAX_IPSET 5				//网络提供商配置

struct stServerInfo
{
    stServerInfo(void)
    {
        memset(this,0,sizeof(stServerInfo));
    }

	int Type;			//服务类型编号
	int LineId;			//服务线路编号
	int GateId;			//服务网关编号
	int Ip[MAX_IPSET];	//服务IP
	int Port;			//服务端口
	int SocketId;		//连接句柄
};

struct stServerZone
{
	stServerInfo gateInfo;
	int ZoneIp;
	int ZonePort;
	int ConnectSeq;
	int Socket;
};

class CServerManager
{
public:
	CServerManager();
	~CServerManager();

    void Update(void);

	bool Initialize();
	void AddServerMap(stServerInfo &ServerInfo);
	void ReleaseServerMap(int SocketId,bool postEvent=false);

    int  GetServerSocket(int ServerType,int ServerLineId=0,int ServerId=0);
    int  GetGateSocket(int playerId);//client
    bool GetGateInfoBySocket(int SocketId,U32& lineId,U32& gateId);
	
	void PackGateData(Base::BitStream &,U8 netProvider);

	int  GetLineId(int SocketId);
	
	stWorldSetting& GetWorldSetting() { return m_WorldSetting;}

	void GetAllGates( std::list<stServerInfo*>& serverInfoList );
	void GetAllGates( std::list<stServerInfo*>& serverInfoList, int nLineId );

    U32  GetNormalLineCount(void);
    
    U32  GetMaxPlayerCount(void);

	void GetAllLines(std::vector<int>& lines);

    bool RandGateAddress(int line,int& ip,int& port,U8 netProvider);
    
    //服务器开服时间的控制
    void SetOpenTime(time_t openTime);
    time_t GetOpenTime(void) {return m_openTime;}
    void AddLoginTimeRange(const TimeRange& range);
    
    //返回是否服务器已经开服
    bool IsOpenTime(time_t curTime);

    //如果服务未开服,是否允许登录
    bool IsLoginTime(time_t curTime);

    bool HasDataAgent(void) const;
	bool readWorldSetting();

#ifndef _NTJ_UNITTEST
protected:
#endif
    //bool readMapConfig();

    stServerInfo* GetServerInfo(int ServerType,int ServerLineId = 0,int ServerId = 0);
    
	typedef stdext::hash_map<int,stServerInfo*>  HashSocketMap;//服务器之间通信信息
	typedef stdext::hash_map<int,HashSocketMap>  HashLineGateMap;
	typedef stdext::hash_map<int, stServerInfo > HashDataAgentMap;

    CMyCriticalSection m_cs;
	
	HashSocketMap	m_SocketMap;//所有sokcket队列
	HashLineGateMap m_LineGateMap;//gate_socket队列
	stServerInfo*	m_pAccountInfo;
	stServerInfo*	m_pChatServerInfo;
	stServerInfo*	m_pWorldDBInfo;
	stWorldSetting  m_WorldSetting;//全服配置文件
	HashDataAgentMap m_DataAgentMap;

    SimpleTimer<600> m_PacketLogTimer;
	SimpleTimer<15>  m_TimeTraceTimer;

    time_t m_openTime;
    std::vector<TimeRange> m_loginTimeRange;
};

#endif /*SERVER_MANAGER_H*/
