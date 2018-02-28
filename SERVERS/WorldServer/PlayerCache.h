//#ifndef PLAYER_CACHE_H
//#define PLAYER_CACHE_H
//
//#include "PlayerOperator.h"
//
//#include <hash_map>
//#include <deque>
//#include "base/Locker.h"
//#include "Common/slist.h"
//#include "Common/BackWorker.h"
//
//class CPlayerCache : private CPlayerOperator
//{
//public:
//	CPlayerCache();
//	virtual ~CPlayerCache();
//
//	void showStatus();
//
//	PlayerDataRef loadPlayer(  int playerId ); 
//	PlayerDataRef loadPlayer( std::string playerName );
//
//	int savePlayerData( PlayerDataRef pPlayerData );
//	//int savePlayerData( stPlayerStruct* pPlayerData );
//
//	//---------------------------------------------------------------
//	// 刷新数据库缓冲区
//	//---------------------------------------------------------------
//	void flush();
//
//	int getSaveListCount();
//
//	//---------------------------------------------------------------
//	// 处理数据
//	//---------------------------------------------------------------
//	void processTick();
//	
//private:
//#if defined(_UNITTEST)||defined(_NTJ_UNITTEST)
//public:
//#endif
//	void updatePlayer( PlayerDataRef pPlayerData );
//
//	static void WorkThread( void* pParam );
//	void saveToDB( PlayerDataRef pPlayerData );
//	void doSave( PlayerDataRef* player );
//	typedef SList< PlayerDataRef > PlayerDataList;
//	typedef stdext::hash_map< int, PlayerDataList::NodeRef > HashPlayerIdMap;
//	typedef stdext::hash_map< std::string, PlayerDataList::NodeRef > HashPlayerNameMap;
//
//	typedef stdext::hash_map< int, int > PlayerIdSet;
//	typedef stdext::hash_map< int, PlayerDataRef > PlayerSetID;
//	typedef stdext::hash_map< std::string, PlayerDataRef > PlayerSetName;
//
//	PlayerDataList		mPlayerDataList;
//	PlayerDataList		mSavePlayerDataList;
//
//	HashPlayerIdMap		mPlayerIdMap;
//	HashPlayerNameMap	mPlayerNameMap;
//
//	CMyCriticalSection  mSaveLock;
//	CMyCriticalSection  mLoadLock;
//	PlayerIdSet			mDeleteSet;
//	PlayerSetID			mSaveSetID;
//	PlayerSetName		mSaveSetName;
//
//	typedef CPlayerOperator Parent;
//
//	HANDLE				mThreadHandle;
//	bool				mIsEnd;
//	CBackWorker			mWorker;
//
//	static const int	CacheSize = 1024;
//
//	int					mLastCacheCheckTime;
//	int					mHitCount;
//	int					mVisitCount;
//
//	float				mHitPrecent;
//};
//
//#endif