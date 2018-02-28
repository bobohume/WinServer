#ifndef SOCIAL_MANAGER_H
#define SOCIAL_MANAGER_H

#include <map>
#include <deque>
#include "base/Locker.h"
#include "LockCommon.h"
#include "ManagerBase.h"
#include "AccountHandler.h"
#include "Common/SocialBase.h"

class CDBConnPool;
class CDBConn;

class CSocialManager : public CManagerBase< CSocialManager>
{
public:
    CSocialManager(CDBConnPool* pConnPool);
    ~CSocialManager(void);

    void ProcessEvents(int event,Base::BitStream& recv);
    void OnTimer();
    void OnQuit();
#ifdef _NTJ_UNITTEST
public:
#else
protected:
#endif
	typedef std::deque< int >       PlayerList;
	typedef stdext::hash_set<int>   QuitList;
	typedef std::map< int, U8 >     PlayerStatusMap;
	typedef std::map< int, SocialItemMap > SocialMap;

	enum enSex
	{
		SEX_BOTH	= 0,			//无要求
		SEX_MALE,					//男性
		SEX_FEMALE,					//女性
	};

// 操作部分
protected:
	
	void             updateInfo                ();
	void             onPlayerEnter             (int playerId);                                        // 载入社会关系
	void             onPlayerQuit              (int playerId);                                        // 保存社会关系
	void			 sendSocial				   (int playerId,bool bFlag = true);                      // 发送当前玩家社会关系
	void             destoryAllLink            (int playerId);                                        // 删除所有关联关系(用于玩家删号)
	void             cleanupSocialCache        (int playerId,int destPlayerId);                       // 清除两个玩家缓存内容
	void             changePlayerStatus        (int playerId,PlayerStatusType::Type status);          // 变更玩家状态	
    bool             updateDb();
protected:
	enSocialError    makeLink                  (int playerId1,int playerId2,SocialType::Type type);   // 建立
	enSocialError    destoryLink               (int playerId1,int playerId2);                         // 删除

	int              addFriendValue            (int playerId1,int playerId2,int value);           // 增加好友度
	int              addFriendValueV1          (int playerId1,int playerId2,int value);           // 组队打怪加好友度
	int              addFriendValueV2          (int playerId1,int playerId2,int value);           // 组队做任务加好友度
	int              queryFriendValue          (int playerId1,int playerId2);                     // 查询好友度(好感度)
	bool             setFriendValue            (int playerId1,int playerId2,int value);           // 设置好友度（好感度）

	bool             buddyFriend               (int playerId1,int playerId2);                     // 互为好友
	bool             hasFriend                 (int playerId1,int playerId2);                     // 存在好友
	bool             isPlayerBlackListed       (int playerId1,int playerId2);                     // playerId2是否在playerId1黑名单中

	void			 TimeProcess			   (U32 bExit);
	bool             readDb                    (int playerId);
	bool             saveDb                    (int playerId);
	bool             saveDb                    (int playerId,const stSocialItem& info);
	bool             updateDb                  (int playerId,const stSocialItem& info);
	bool             updateDb                  (int playerId,const stSocialItem& info,int destType);
	bool             deleteSocial              (int playerId,int destPlayer);

	bool             isDataLoading             (int playerId);                // 是否加载了数据
	void             RemoveLotList             (int playerId,int sex = 0);    // 移除有缘人

	stSocialInfo     buildInfo                 (const stSocialItem& item,SimplePlayerData* data);      // 构建具体信息
	void             notifyPlayerStatusChange  (int playerId,PlayerStatusType::Type status);                 // 上线下通知
	
	enSocialError    onNewAddPlayer            (int playerId1,int playerId2,SocialType::Type type);    // 新增一项
	bool             hasFriend                 (const SocialItemMap& itemMap,int destPlayerId);        // 存在好友系
	bool             buddyFriend               (const SocialItemMap& itemMap,int destPlayerId);        // 互为好友

	void             sendFriendInfo            (int playerId,int destPlayerId);                        // 发送player消息到zoneServer
    
    void NotifyPlayerOffline (int playerId, stSocialItem& socialItem);
	void NotifyPlayerOnline  (int playerId, stSocialItem& socialItem);
// 消息部分处理
protected:
	DECLARE_EVENT_FUNCTION(HandleMakeRequest);                            // 建立关系
	DECLARE_EVENT_FUNCTION(HandleDestoryRequest);                         // 解除关系
	DECLARE_EVENT_FUNCTION(HandleWorldSocialInfoRequest);             // 请求社会关系表
	/*
	DECLARE_EVENT_FUNCTION(HandleZoneWorldFindFriendRequest);             // 查询好友信息
	DECLARE_EVENT_FUNCTION(HandleZoneWorldPlayerStatusChangeRequest);     // 更改状态
	DECLARE_EVENT_FUNCTION(HandleZoneWorldAddFriendValue);                // 加好友度
	DECLARE_EVENT_FUNCTION(HandleZoneWorldChangeLinkReuquest);            // 修改关系
	DECLARE_EVENT_FUNCTION(HandleZoneWorldFindPlayerRequest);             // 查找玩家
	DECLARE_EVENT_FUNCTION(HandleZoneAddFriendRequest);                   // 请求玩家加自己
	DECLARE_EVENT_FUNCTION(HandleZoneUseItemAddFriendValueRequest);       // 使用道具加好友度
	DECLARE_EVENT_FUNCTION(HandleZonePlayerListAddFriendRequest);         // 批量玩家加好友度
	DECLARE_EVENT_FUNCTION(HandleZoneNotFriendClearRequest);              // 非好友批量删除

	DECLARE_EVENT_FUNCTION(HandleZoneWorldMasterSocialRequest);           // 查询玩家师徒数据
	DECLARE_EVENT_FUNCTION(HandleZoneWorldPlayerMasterSaveRequest);       // 保存出师玩家
	DECLARE_EVENT_FUNCTION(HandleZoneQueryPrenticeListRequest);           // 查询已出师徒弟例表


	//命运有缘人
	//要从全部在线角色中挑选一部分合适的玩家,然后随机选择其中的一位跟特定玩家配对
	//重点:	(1)如何确定玩家在线
	//		(2)如何确定合适的玩(高等级玩家按队列先后)
	DECLARE_EVENT_FUNCTION( HandleZoneWorldLotRequest );
	DECLARE_EVENT_FUNCTION( HandleZoneWorldAddLotRequest );
	DECLARE_EVENT_FUNCTION( HandleZoneWorldRemoveLotRequest );*/

	void FindFriendRequest(Base::BitStream* Packet);

#ifdef _NTJ_UNITTEST
	public:
#else
protected:
#endif

	SocialMap            mSocialMap;          // 社会关系
	PlayerStatusMap      mPlayerStatusMap;    // 玩家状态

	LimitMap             mFriendValueMap;     // 好友度更新
	LimitMap	         mLimitTeam;          // 组队加好友度限制
	LimitMap	         mLimitMission;       // 任务加好友度限制

	QuitList             mQuitList;           // 玩家离线

	PlayerList	         mMaleList;           // 男情有缘人
	PlayerList           mFemaleList;         // 女性有缘人

	int                  mCurrentDay;         // 当前天

	SimpleTimer<60>      m_60Timer;
	CDBConn* m_pDBConn;
} ;

#endif /* SOCIAL_MANAGER_H */