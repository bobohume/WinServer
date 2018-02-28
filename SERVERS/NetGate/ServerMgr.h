#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <string>
#include <hash_map>

#include "Base/Locker.h"

struct stServerInfo
{
    stServerInfo(void)
    {
        memset(this,0,sizeof(stServerInfo));
    }

	int ZoneId;			//地图编号
	int SocketId;		//连接句柄
	int Ip;
	int Port;
	int ConnectSeq;		//连接序号，用来判断是地图服务器断开重连了还是宕机了
};

struct stZoneInfo
{
    stZoneInfo(void)
    {
        memset(this,0,sizeof(stZoneInfo));
    }

	int ZoneId;			//地图服务器编号
	int MaxPlayerNum;	//最大玩家数量
	int ServicePort;    //服务端口号
	int Sequence;
};

struct stGoods;
struct stDiscount;
struct stRecommend;

class CServerManager
{
protected:
	typedef stdext::hash_map<int,stServerInfo*> HashSocketMap;
	typedef stdext::hash_map<int,stServerInfo*> HashZoneMap;
	typedef stdext::hash_map<int,stZoneInfo> HashZoneInfo;

	//typedef std::vector<stGoods*>		GOODSVEC;
	//typedef std::vector<stDiscount*>	DISCOUNTVEC;
	//typedef std::vector<stRecommend*>	RECOMMENDVEC;
	//GOODSVEC		mGoodsList;
	//DISCOUNTVEC		mDiscountList;
	//RECOMMENDVEC	mRecommendList;

	HashSocketMap	m_SocketMap;
	HashZoneMap		m_ZoneMap;

	HashZoneInfo	m_ZoneInfoList;

    //竞技场的相关配置，同步到ZONE
    char            m_arenaStartTime[256];
    int             m_openDay;
    int             m_closeDay;

	int				mDataAgentSocket;

	U8				m_Years5Assure;		//五年包养活动标志
	U8				m_GodGivePath;		//仙人指路活动标志
public:
	CMyCriticalSection m_cs;

public:
	CServerManager();
	~CServerManager();

	bool Initialize();
	void AddServerMap(stServerInfo &ServerInfo);
	void ReleaseServerMap(int SocketId,bool postEvent=false);
	stServerInfo *GetServerInfo(int ZoneId);
	stServerInfo *GetZoneInfo( int socketId );
	int GetZoneSocketId(int ZoneId);
	int GetDataAgentSocket();
	void SetDataAgentSocket( int socketId );

	void UnpackZoneData(Base::BitStream *);
	bool GetZoneInfo(int ZoneId,stZoneInfo &);
    void GetArenaInfo(std::string& startTime,int& openDay,int& closeDay);

	stServerInfo* GetZoneInfo(int SocketId, int ZoneId);
	void PackZoneMgrList(Base::BitStream &);
	void RemoveZoneFromGate(int SocketId, int ZoneId);

	U8 GetYears5Assure()	{ return m_Years5Assure;}
	U8 GetGodGivePath()		{ return m_GodGivePath;}

	//void AddGoods(stGoods*);
	//void AddDiscount(stDiscount*);
	//void AddRecommend(stRecommend*);
	//void ClearGoods();
	//void ClearDiscount();
	//void ClearRecommend();
	//bool IsNoGoods() { return mGoodsList.empty();}
	//bool IsNoDiscount() { return mDiscountList.empty();}
	//bool IsNoRecommend() { return mRecommendList.empty();}
	//void SendGZBillingResponse(int SocketHandle, int type);
};

#endif