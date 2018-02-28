#ifndef __PLAYER_MGR_H__
#define __PLAYER_MGR_H__

#include <string>
#include <hash_map>
#include "Common/ChatBase.h"
#include <time.h>
#include "base/locker.h"

class ChatPlayer
{
public:
    ChatPlayer(void)
    {
        m_lastUpdateTime = time(0);
        m_isOnline       = false;
    }

    bool IsLogin(void) const {return m_isOnline;}

    time_t         m_lastUpdateTime;
    bool           m_isOnline;
    ChatPlayerInfo m_playerInfo;
};

struct SendInfo
{
    SendInfo(void)
    {
        accountId = 0;
        lineId    = 0;
        socketId  = 0;
    }

    U32 accountId;
    U32 lineId;
    int socketId;
};

class CPlayerMgr
{
public:
    std::string getPlayerName(U32 playerId);
	std::string getAccountName(U32 playerId);
	std::string getPlayerIp(U32 playerId);

    U32 getAccountId(U32 playerId);
    U32 getPlayerId(const char* name);
    int getSocketId(U32 playerId);

    SendInfo GetSendInfo(U32 playerId);
    bool GetPlayerInfo(U32 playerId,ChatPlayerInfo& info);

	void addPlayer(const ChatPlayerInfo& info);
	void removePlayer(U32 playerId);
    void RemoveAccount(U32 accountId,U32 newPlayerId);
    
    void Update(void);
protected:
    ChatPlayer*     GetPlayer(U32 playerId);
    ChatPlayer*     GetPlayer(const char* name);
    ChatPlayerInfo* GetPlayerInfo(U32 playerId);

    CMyCriticalSection m_cs;

	typedef stdext::hash_map<U32,ChatPlayer*> HashPlayerAccountMap;
	typedef stdext::hash_map<std::string,U32> HashNamePlayerMap;
    typedef stdext::hash_map<U32,U32>         ACCOUNTMAP;

	HashPlayerAccountMap m_players;
    ACCOUNTMAP           m_accounts;
	HashNamePlayerMap	 m_playerNames;
};

#endif /*__PLAYER_MGR_H__*/