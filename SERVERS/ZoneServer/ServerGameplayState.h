//
// NTianJi Game Engine V1.0
//
//
// Name:
//		ServerGameplayState.h
//
// Abstract:
//

#pragma once

#include <time.h>
#include <hash_map>
#include "common/PacketType.h"
#include "common/PlayerStructEx.h"
#include <hash_set>
#include "Common/OrgBase.h"

#include "CommLib/ShareMemoryKey.h"
#include "CommLib/ShareMemoryUserPool.h"
#include "Common/Common.h"
#include "Timer/timeSignal.h"

#define SM_PLAYERCOUNT 3000
typedef SMDataPool<stPlayerStruct,U32,SM_PLAYERCOUNT> SM_PLAYER_POOL;
typedef SMDataPool<stPlayerStruct,U32,SM_PLAYERCOUNT>::POOL_DATA SM_PLAYER_DATA;

// ========================================================================================================================================
//	ServerGameplayState
// ========================================================================================================================================
///	Server gameplay state.
///
struct PlayerLoginBlock
{
	T_UID UID;
	U32	  Time;
	
	stPlayerStruct* pPlayerData;

	//sm数据会绑定上面的pPlayerData
	//后续对玩家数据的打包保存,都通过pSMPlayerData对象来操作
	SM_PLAYER_DATA* pSMPlayerData;

	PlayerLoginBlock()
	{
		memset( this, 0, sizeof( PlayerLoginBlock ) );
	}

	void FreeSM(SM_PLAYER_POOL* pool)
	{
		if (0 == pSMPlayerData)
			return;

		pool->Free(pSMPlayerData);
		pSMPlayerData = 0;
	}
};

class Player;
class ServerPacketProcess;
class CDataAgentHelper;
class CLogHelper;

typedef void* HANDLE;

class ServerGameplayState
{
private:
	U32				 mZoneId;
	U32				 mMaxPlayerNum;
	U32				 mAreaId;			//当前Zone所在的区服ID
	
	bool			 mInited;          //地图初始化标识
	U32				 mInitFlag;			//地图初始化标识
public:
	typedef stdext::hash_map<U32,PlayerLoginBlock> HashPlayerLoginBlock;
	typedef stdext::hash_map<U32,stPlayerStruct*>  HashPlayerSaveMap;
	typedef stdext::hash_map< int, int >		   HashPlayerSIDMap;
	typedef stdext::hash_map<U32,Player *>		   HashPlayer;
	typedef stdext::hash_map<U32,U32>			   HashAccount;
	typedef Vector<Player*>							PlayerVec;


private:

	HashPlayerLoginBlock	m_PlayerLoginDataTimeout;  //Ray: 这里登记的数据会超时检查
	HashPlayerLoginBlock	m_PlayerLoginData;
	HashPlayerSaveMap		m_PlayerSaveMap;

	HashAccount				m_AccountMap;
	HashPlayer				m_PlayerManager;
	HashPlayerSIDMap		m_PlayerSIDMap;

	//CDataAgentHelper*		m_pDataAgentHelper;
    CLogHelper*             m_pLogHelper;
	stdext::hash_set<int>	m_copyMapSet;

public:
	ServerGameplayState										();
	~ServerGameplayState									();

	void			 Initialize								();
	void			 preShutdown							();
	void			 Shutdown								();
    bool             InitSM();

	void			 CheckPlayerQuit						( int delta );
	void			 AddPlayerQuit						    ( int playerId, int type );
	void			 RemovePlayerQuit					    ( int playerId );

	void			 TimeLoop								();
	U32				 getZoneId								()	const						{return mZoneId;}

	void			 setAreaId								(int AreaId)					{mAreaId = AreaId;}
	U32				 getAreaId								()								{return mAreaId;}


	void			 setMaxPlayerNum						(U32 MaxPlayerNum)				{mMaxPlayerNum=MaxPlayerNum;}
	U32				 getMaxPlayerNum						()								{return mMaxPlayerNum;}


    ZoneLogicData*  GetPlayerZoneData                       (T_UID UID,int PlayerId);

	//玩家登陆数据管理
	void			 FilterPlayerLoginDataTimeout			(int curtime);
	void			 RemovePlayerLoginDataTimeout			(int PlayerId);

	bool			 AddPlayerLoginData						(T_UID UID, stPlayerStruct*);
	void			 AddPlayerLoginData						(PlayerLoginBlock &block );
	stPlayerStruct	*GetPlayerLoginData						(T_UID UID,int PlayerId);
	PlayerLoginBlock DetachPlayerLoginData					(int PlayerId);
	bool			 DeletePlayerLoginData					(int PlayerId);
	void			 SavePlayerData(U32 PlayerId,bool isLastSave = false);
	bool			 LoadPlayerDataFromSM(U32 PlayerId,stPlayerStruct* pData);
	bool			 IsFirstLoginData						(int PlayerId);
	bool			 IsFirstLogin							(int PlayerId);
	void			 SetPlayerSID							( int playerId, int SID ) { m_PlayerSIDMap[playerId] = SID; }
	int				 GetPlayerSID							( int playerId ) {return m_PlayerSIDMap[playerId];}
	void			 AddInPlayerManager						(Player *);
	void			 DeleteFromPlayerManager				(Player *);
	void			 addBattleField							(Player *);
	void			 removeBattleField						(Player *);
	U32				 getBattleFieldPlayerCount				(U32 layerId, U32 battleFieldForce);
	S32				 getBattleFieldPlayer					(U32 layerId, U32 battleFieldForce, U32 index);
	void			 sendInitialEvent						(Player *player);
	void			 sendQuitZoneStatisticsEvent			(Player *player);
	void			 updateZoneStatistics					(S32 delta);
	void			 onEndBattlefield						(S32 layer, S32 winForce);
	void			 onEndArena								(S32 layer, S32 winForce);
	void			 checkArenaResult						(Player *player);

	Player *		 GetPlayer								(int PlayerId);
	Player *		GetPlayerByAccount						(int accountId);

public:
	stPlayerStruct* GetPlayerLoginData						( int playerId );
	stPlayerStruct* GetPlayerSaveData( U32 playerId );
	void AddPlayerSaveData( stPlayerStruct* pPlayer );
	void RemovePlayerSaveData( U32 playerId );
	int GetPlayerCount();
	const HashPlayer& getHashPlayer() { return m_PlayerManager;}
	
	typedef stdext::hash_map< int, std::pair<int, int> > PlayerQuitMap;
	PlayerQuitMap mPlayerQuitMap;

	bool			m_bIsRobotTest;
	TimeSignal<U32> m_TimeSignal;
private:
	SM_PLAYER_POOL  m_smPool;
};

// ========================================================================================================================================
//	Global Variables
// ========================================================================================================================================
extern ServerGameplayState* g_ServerGameplayState;
extern U32	g_nZoneId;
