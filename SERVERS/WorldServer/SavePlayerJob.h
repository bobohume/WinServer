//#ifndef SAVE_PLAYER_JOB_H
//#define SAVE_PLAYER_JOB_H
//
//#include "Common/TimerMgr.h"
//#include "Common/PlayerStructEx.h"
//#include "base/Locker.h"
//#include <hash_map>
//#include <list>
//#include "LockCommon.h"
//#include <stack>
//
//class CSavePlayerJob
//	: public CTimeCriticalObject
//{
//public:
//	enum
//	{
//		MAX_SAVE_PLAYER_COUNT = 32
//	};
//
//	CSavePlayerJob();
//
//	bool TimeProcess( bool bExit );
//
//	bool Update();
//	void PostPlayerData( unsigned long UID, stPlayerStruct* pPlayerData );
//
//	void SetDataAgentSocketHandle( int nPlayerId, int nSocket );
//	CMyCriticalSection m_cs;
//
//	int allocPlayerStruct();
//	void freePlayerStruct( int index );
//
//private:
//	stdext::hash_map< int, unsigned long> m_playerUIDMap;	
//
//	std::list<int> m_playerList;
//
//	stdext::hash_map<int, int> m_dataAgentMap;
//
//	typedef std::stack<int> FreeStack;
//	stPlayerStruct mPlayers[MAX_SAVE_PLAYER_COUNT];
//	FreeStack mFreeStack;
//
//};
//
//
//#endif