#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <string>
#include <hash_map>
#include <hash_set>
#include <set>
#include <Time.h>

#include "Base/bitStream.h"
#include "Base/Locker.h"
#include "Common/PacketType.h"
#include "Common/PlayerStruct.h"
#include "Common/Tracable.h"
#include "Common/dbStruct.h"

#include "AccountHandler.h"
#include "LockCommon.h"
#include "PlayerCache.h"

#include "CommLib/DataCache.h"
#include "CommLib/SimpleTimer.h"
#include "CommLib/SimpleLocker.h"

class CDBOperator;

#define MAX_LINE_COUNT  3000

extern U32 g_maxLinePlayers;

//用于其他异步线程,来获取玩家的基本信息
class AnsyPlayerInfo
{
public:
    AnsyPlayerInfo(void)
    {
        memset(this,0,sizeof(AnsyPlayerInfo));
    }

    U32 playerId;
    U32 accountId;

    //登陆gate时socket和所在的线
    U32 socket;
    U32 gateLineId;

    bool isInGame;
    char gmFlag;
};

#define MAX_ACCOUNT_WRITE 100000

//缓冲服务器的帐号信息,并在线程里面输出到文件
struct AccoutWriteInfo
{
    char accountName[ACCOUNT_NAME_LENGTH];
    char playerName[MAX_NAME_LENGTH];
    U32  playerId;
    U32  accountId;
    int  state;
};

struct LoginUnit
{
	LoginUnit(U32 id,const char* n) : playerId(id),name(n ? n : "") {}

	std::string name;
	U32 playerId;
};

#define MAX_FAKE_PLAYER_COUNT 80000

#define MAX_LINE 100

/************************************************************************/
/* 管理当前所有的帐号
/************************************************************************/
class CPlayerManager
{
public:
	CPlayerManager();
	~CPlayerManager();

    //对于人物的简单结构这里预先载入一部分.按载入的时间来计算，如果超过
    //时间值则停止载入
    void LoadSimpePlayerDatas(int timeVal);

    //-----------------------------------------------------------------------------------------
    //提供给一些异步的线程去获取玩家的信息,这时都是通过值来返回,而不是对象引用的形式.
    bool AnsyGetPlayerData(int PlayerId,SimplePlayerData& data);
    bool AnsyGetPlayerData(const char* name,SimplePlayerData& data);

    bool AnsyGetOnlinePlayer(int playerId,AnsyPlayerInfo& data);
    bool AnsyGetOnlinePlayer(const char* name,AnsyPlayerInfo& data);
    
    /************************************************************************/
    /* 添加帐号管理
    /************************************************************************/
    ERROR_CODE AddAccount(Player& accountInfo, Players& playerList);

	void TimeProcess(U32 bExit);
    void Update(void);

	//查询玩家信息,都是返回简单结构
	PlayerSimpleDataRef GetPlayerData(int PlayerId);
	PlayerSimpleDataRef GetPlayerData(const char* name);
	std::string			GetPlayerName(int PlayerId);
	U32					GetFirstClass(U32 nPlayerID);
	U32					GetFamily(U32 nPlayerID);
	bool AddPlayerData(PlayerSimpleDataRef spData);

	bool IsPlayerOnline(int PlayerId);

	ERROR_CODE VerifyUID(T_UID UID,int AccountId);

	void SendSimplePacket( int nPlayerId, const char* cMessageType, int nDestZoneId = 0, int nSrcZoneId = 0 );

	int AddPlayerMap(AccountRef pPlayer,U32 playerId,const char* name);

	ERROR_CODE DeletePlayer(T_UID UID,int AccountId,int Playerid,bool isFromZone);

    //DB返回玩家删除成功
    void OnPlayerDeleted(U32 accountId,U32 playerId);

	ERROR_CODE CreatePlayer(T_UID UID,int AccountId,const char *PlayerName,int Sex,int Body,int Face,int Hair,int HairCol,U32 ItemId,int FaceImage,int Family,int ZoneId,int Playerid);
    
	void RemoveAccount( int nAccountId );
	void RemovePlayer(U32 playerId,const char* name);

	AccountRef GetOnlinePlayer(int nPlayerId);
	AccountRef GetOnlinePlayer(const char* playerName);
	AccountRef GetAccount( int AccountId );
    AccountRef GetAccount( const char* accountName);

	void writeAccountInfo();

	void SendSimpleInfoPacket( int nPlayerId, const char * cMessageType, int nDestZoneId, int nSrcZoneId = 0 );

	void RegisterChat();

    bool HasOnlineAccount(U32 accountId);
    void AddOnlineAccount(U32 accountId);
    void DelOnlineAccount(U32 accountId);
    U32  GetOnlineAccountCount(void) const;
    
    void SetCreatedPlayerCount(U32 count);
    U32  GetCreatedPlayerCount(void);
   
    //清除玩家对象上已加载的player数据
    bool ClearPlayerData(U32 accountId);
	//-----------玩家操作---------------
	bool CanAddGold(int PlayerId, S32 Gold);
	bool AddGold(int PlayerId, S32 Gold, enOpType logType = LOG_GOLD_BUY);
	bool SetFace(int PlayerId, S32 Val);
	bool AddDrawGold(int PlayerId, S32 Gold);
	bool SetVipLv(int PlayerId, U32 VipLv);
	//----------------------------------
public:
    void OnNewDay();
    void UpdateLastDayTime();

	//玩家的基本信息
	typedef DataCache<PlayerSimpleDataRef,100000,1800> SPLAYER_CACHE;
	SPLAYER_CACHE m_simplePlayerDatas;
	
	//定时(s)向帐号服务器发送在线人数
	SimpleTimer<30>   m_30Timer;
	SimpleTimer<60>   m_60Timer;

    //已创建的玩家数目
    U32 m_createdPlayerCount;

    typedef stdext::hash_map<int,AccountRef>			 HashAccountMap;
	typedef stdext::hash_map<std::string,AccountRef>	 HashPlayerNameMap;
    typedef stdext::hash_set<U32>                        OnlineAccountMap;
    typedef stdext::hash_map<std::string,int>            HashAccountNameMap;

	HashPlayerNameMap       m_PlayerNameMap;			//登录的PlayerName为键值
	HashAccountMap          m_PlayerMap;				//登录的PlayerId为键值
	HashAccountMap          m_AccountMap;			    //登录的帐号为键值
    HashAccountNameMap      m_accountNameMap;			//名字对应accountId
    
    //记录所有在角色选择或游戏内的帐号
    //用来统计当前的在线玩家数
    OnlineAccountMap m_onlineAccounts;

    volatile time_t m_lastUpdateTime;

    //统计进入神仙传的角色次数
    U32 m_playerEnterInterval;
    
    //把需要写文件的帐号信息全部放在这里，假设最大值为10W个帐号
    //然后这个结构被放到线程里面输出到文件
    bool m_canWriteAccout;
    AccoutWriteInfo m_accountWriteInfos[MAX_ACCOUNT_WRITE];
};

#endif
