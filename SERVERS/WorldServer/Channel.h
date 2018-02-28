#ifndef CHAT_CHANNEL_H
#define CHAT_CHANNEL_H

#include <string>
#include <sstream>
#include <list>
#include <hash_map>
#include <hash_set>

#include "base/Locker.h"
#include "Common/ChatBase.h"
#include "Event.h"

#include "LockCommon.h"
#include <memory>

class CChannelManager;

class CChannel
{
    friend CChannelManager;
#ifdef _NTJ_UNITTEST
public:
#else
protected:
#endif
    void GetPlayerList(std::vector<int>& playerList);

    UINT GetId();
    const char* GetName();
    char GetMessageType();
    bool HasPlayer( int nPlayreId );

    CChannel() {}
    CChannel( UINT nChannelID, char cMessageType,const char* strChannelName );
    virtual ~CChannel();

    void AddPlayer(int nPlayerId);
    void RemovePlayer(int nPlayerId);

    unsigned int m_nChannelID;			//ID
    char    	 m_cMessageType;		//消息类型
    std::string  m_strChannelName;		//名称

    stdext::hash_set<int> m_playerList;
};

class CChannelManager
{
public:
	CChannelManager();
	virtual ~CChannelManager();

    void GetPlayerList(U32 channelId,std::vector<int>& playerList);
	U32  GetChannelId(int nPlayerId, char cMessageType);
    U32  GetChannelId(int channelId);

	U32  RegisterChannel(char cMessageType,const char* strChannelName);
	void UnregisterChannel( UINT nChannelID );

	void AddPlayer(int nPlayerId, int nChannelId, bool bLink = true);
	void RemovePlayer(int nPlayerId, int nChannelId, bool bClearLink = true);

	void RemoveAllChannel();
#ifndef _NTJ_UNITTEST
protected:
#endif
    CChannel* GetChannel( UINT nChannelID );
	CChannel* GetChannel( int nPlayerId, char cMessageType );

    static UINT BuildChannelID();

    CMyCriticalSection m_cs;

	typedef stdext::hash_map<int,CChannel*> CHANNEL_MAP;
    CHANNEL_MAP m_hmChannelMap;

	typedef stdext::hash_map<__int64, int> PlayerChannelMap;
	PlayerChannelMap m_hmPlayerChannelMap;

	static UINT m_nChannelIDSeed;
};

#endif