#ifndef _ACCOUNTHANDLER_H_
#define _ACCOUNTHANDLER_H_

#include <string>
#include <hash_map>
#include <set>
#include <memory>
#include <time.h>
#include "Common/TimerStateMachine.h"
#include "Common/PlayerStructEx.h"
#include "base/memPool.h"

#include <WinSock2.h>
#include <windows.h>
#include "WorldServer.h"
#include "CommLib/SimpleTimer.h"
#include "Common/AccountSMData.h"
#include "Common/Common.h"

typedef std::tr1::shared_ptr<SimplePlayerData>  PlayerSimpleDataRef;

#define PLAYER_TIMEOUT 400
#define QUERYACCOUNT_TIMEOUT 10*60 //10分钟

struct stPacketHead;
class  Base::BitStream;
class Player;

typedef std::tr1::shared_ptr<Player> AccountRef;
enum AccountStats
{
	ACCOUNT_BAND = BIT(0),//封号
	ACCOUNT_BAND_CHAT = BIT(1),//禁言
};

/************************************************************************/
/* 帐号和帐号对应玩家的封装
/************************************************************************/
class Player : public AccountSMData
{
    typedef AccountSMData Parent;
public:
	typedef std::vector<std::pair<S32, U32>> PAIRVEC;
	typedef PAIRVEC::iterator PAIRVECITR;
	//需要异步执行的,被设置成下面相应的标志;
	enum PlayerEvent
	{
		EVENT_WAIT_PLAYERDATA	= BIT(0),	//发送玩家数据

		//当前正在等待Zone踢线返回
		EVENT_WAIT_KICKPLAYER	= BIT(1),

		//等待验证码
		EVENT_WAIT_CREATEPLAYER = BIT(2),	//创建玩家
		EVENT_WAIT_ENTERGAME	= BIT(3),	//等待登陆游戏
		EVENT_TRANSPORT			= BIT(4),	//正在传送

		//客户端掉线
		EVENT_CLIENTLOST		= BIT(5),
        EVENT_LOGOUTED          = BIT(6),
	};

    //帐号的状态,这是状态机
    enum Status
    {
        STATUS_IDEL              = 0, //空闲状态
		STATUS_LOGIN,				  //帐号登录
        STATUS_IN_SELECT,             //角色选择
        STATUS_IN_GAME,               //在游戏世界里,包括传送(切线和地图传送)
        STATUS_LOGOUT,				  //帐号退出,会在一定时间后切换到offline状态.并删除帐号数据
		STATUS_OFFLINE,

        STATUS_COUNT,
    };

public:
    Player();
	~Player(void);

	Player &operator=(const Player& Other)
    {
        LastTime		  = (int)time(NULL);
        isGM			  = Other.isGM;
        isAdult			  = Other.isAdult;
        UID				  = Other.UID;
        AccountId		  = Other.AccountId;
        mSocketId		  = Other.mSocketId;
        CurPlayerPos	  = Other.CurPlayerPos;
        PlayerNum		  = Other.PlayerNum;
        status            = Other.status;
		loginTime		  = Other.loginTime;
		logoutTime		  = Other.logoutTime;
        m_isFirstLoginGame= Other.m_isFirstLoginGame;
		loginType		  = Other.loginType;

        dStrcpy(AccountName,sizeof(AccountName),Other.AccountName);
        memcpy(m_playerIdList, Other.m_playerIdList, sizeof(m_playerIdList));

		dStrcpy(loginIP,sizeof(loginIP),Other.loginIP);
	
       
        return *this;
    }
    
	int  GetState(void)		   {return m_status.GetState();}

	void SetState(int state)    
    {
        AccountSMData::SetState(state);
        m_status.SetState(state);
    }
    
    void    SetGateSocketId(U32 id)     {mSocketId = id;}
    U32     GetGateSocketId(void) const {return mSocketId;}
   
    void    Update(U32 curTime);
    
    bool    IsOffline(void)  const {return m_status.GetState() == STATUS_OFFLINE;}
	bool    IsLogout(void)   const {return m_status.GetState() == STATUS_LOGOUT;}
    bool    IsInGame(void)   const {return m_status.GetState() == STATUS_IN_GAME;}

    bool    Kick(void);

    //测试机器人目前暂定gmflg为200
    bool    IsRobot(void)  const {return (unsigned char)isGM == 200;}
    bool    IsGM(void)     const {return isGM != 0 && !IsRobot();}

	const char* GetPlayerName(void);
	const char* GetPlayerName(U32 playerId);

    U32     GetPlayerId(void) const {return m_playerId;}
	

	U32		GetPlayerCount(void) const;

	bool	VerifyUID(U32 id,int SocketHandle);

    bool    SetPlayerId(U32 playerId);

	//发送角色列表
	bool	SendPlayerList(void);
    int     CheckDeletePlayer(U32 playerId,bool& canDeleted,int& deleteTime);
	void    DeletePlayer(U32 playerId);
	int     CreatePlayer(U32 playerId);

    void    SendPlayerDeleteRespond(U32 playerId,int error,int deleteTime);

	void	ResetLostTimer(void);
	U32		GetLogoutTime(void);

    void    UpdateLogoutTime(void);
    void    UpdateAccountLogoutTime(void);
	void	ClearAllSpPlayerData();

    //重新载入玩家的角色列表,spThis是指向自己的智能指针.这个智能指针的引用计数不是自己管理的
    int     ReloadPlayerList(Players& playerList, AccountRef spThis);

    void    SendClientErrorCode(Base::BitStream& sendPacket);

    void    SendGateLoginRespond(int error);

    void    SetFirstLoginFlag(bool isFirst) {m_isFirstLoginGame = isFirst;}

    U8      GetLevel(void);

    //返回是否可以向玩家发送逻辑数据,需要在游戏里并且不在传送或断线重连状态中
    bool    CanSendLogicData(void);
	//----------------------------这里是排行榜数据------------------------------------------
	void ComeNewTopRank(int nType = -1);
	static void Export(struct lua_State* L);
	static Player getPlayer(U32 nPlayerId);

	bool isVaild();
	//----------------------------------------
public:
    //玩家登陆的gate
    int		mSocketId;
    int     m_newPlayerId;//新建角色id

	CreatePlayerInfo m_createInfo;
    bool    m_isFirstLoginGame;

	typedef void(Player::*EVENT_CALLBACK)(int,int,stPacketHead*,Base::BitStream*);
	CTimerStateMachine<Player,STATUS_COUNT,EVENT_CALLBACK> m_status;
private:
    //初始化状态
    void    __InitStatus(void);
    
    void    __OnStateChangedHandler(void);

	void	__OnEnterLoginHandler(void);
    void    __OnEnterLogoutHandler(void);

    void    __OnEnterGameHandler(void);
	void	__OnLeaveGameHandler(void);

	void    __OnEnterSelectHandler(void);
	void	__OnLeaveSelectHandler(void);

	//踢人计时器10(s)检测一次,如果长时间没有返回
	//则自动把玩家设置为离线状态
	SimpleTimer<10> m_kickTimer;

	//玩家掉线后5分钟切到离线状态
	SimpleTimer<30> m_lostTimer;

	//玩家的对象的保存时间
	SimpleTimer<60> m_offlineTimer;
};

#endif /*_ACCOUNTHANDLER_H_*/
