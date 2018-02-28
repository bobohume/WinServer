#include "Base/Locker.h"
#include "Channel.h"

CChannel::CChannel(UINT nChannelID, char cMessageType,const char* strChannelName)
{
	m_nChannelID     = nChannelID;
	m_cMessageType   = cMessageType;
	m_strChannelName = strChannelName;
}

CChannel::~CChannel()
{
}

UINT CChannel::GetId()
{
	return m_nChannelID;
}

const char* CChannel::GetName()
{
	return m_strChannelName.c_str();
}

char CChannel::GetMessageType()
{
	return m_cMessageType;
}

void CChannel::AddPlayer( int nPlayerId )
{
	m_playerList.insert( nPlayerId );
}

void CChannel::RemovePlayer( int nPlayerId )
{
	m_playerList.erase( nPlayerId );
}

void CChannel::GetPlayerList(std::vector<int>& playerList )
{
	stdext::hash_set<int>::iterator iter;

	for( iter = m_playerList.begin(); iter != m_playerList.end(); iter++ )
	{
		playerList.push_back( *iter );
	}
}

UINT CChannelManager::m_nChannelIDSeed = 1;

CChannelManager::CChannelManager()
{
}

CChannelManager::~CChannelManager()
{
    OLD_DO_LOCK(m_cs);

	RemoveAllChannel();
}

void CChannelManager::GetPlayerList(U32 channelId,std::vector<int>& playerList)
{
    OLD_DO_LOCK(m_cs);

    CChannel* pChannel = GetChannel(channelId);

    if (0 == pChannel)
        return;

    pChannel->GetPlayerList(playerList);
}

U32 CChannelManager::GetChannelId(int nPlayerId, char cMessageType)
{
    OLD_DO_LOCK(m_cs);

    CChannel* pChannel = GetChannel(nPlayerId,cMessageType);

    if (0 == pChannel)
        return 0;
    
    return pChannel->GetId();
}

U32 CChannelManager::GetChannelId(int channelId)
{
    OLD_DO_LOCK(m_cs);

    CChannel* pChannel = GetChannel(channelId);

    if (0 == pChannel)
        return 0;
    
    return pChannel->GetId();
}   

U32 CChannelManager::RegisterChannel(char cMessageType,const char* strChannelName )
{
	// 大规模消息不能创建频道
	if( cMessageType < CHAT_MSG_TYPE_RACE )
		return 0;

	OLD_DO_LOCK(m_cs);

	UINT nId = BuildChannelID();
    UnregisterChannel(nId);

	CChannel* pChannel = new CChannel(nId,cMessageType,strChannelName);
	m_hmChannelMap[nId] = pChannel;
	return nId;
}

void CChannelManager::UnregisterChannel(UINT nChannelID)
{
	OLD_DO_LOCK(m_cs);

	CHANNEL_MAP::iterator iter = m_hmChannelMap.find(nChannelID);

	if(iter == m_hmChannelMap.end())
		return;
    
    delete iter->second;
	m_hmChannelMap.erase(iter);
}

void CChannelManager::RemoveAllChannel()
{
	OLD_DO_LOCK(m_cs);

	m_hmPlayerChannelMap.clear();

    for (CHANNEL_MAP::iterator iter = m_hmChannelMap.begin(); iter != m_hmChannelMap.end(); ++iter)
    {
        delete iter->second;
    }

    m_hmChannelMap.clear();
}

UINT CChannelManager::BuildChannelID()
{
	UINT id = m_nChannelIDSeed++;

    if (0 == id)
    {
        id++;
    }

    return id;
}

CChannel* CChannelManager::GetChannel( UINT nChannelID )
{
	OLD_DO_LOCK(m_cs);

	CHANNEL_MAP::iterator iter = m_hmChannelMap.find(nChannelID);

	if( iter == m_hmChannelMap.end())
		return 0;

	return iter->second;
}

CChannel* CChannelManager::GetChannel( int nPlayerId, char cMessageType )
{
	// 对于大规模消息来说，没有意义
	if( cMessageType < CHAT_MSG_TYPE_RACE )
		return 0;

	__int64 v = nPlayerId;
	v = ( v << 8 ) | cMessageType;
    
    OLD_DO_LOCK(m_cs);

    PlayerChannelMap::iterator iter = m_hmPlayerChannelMap.find(v);

    if (iter == m_hmPlayerChannelMap.end())
        return 0;

	return GetChannel(iter->second);
}

void CChannelManager::AddPlayer( int nPlayerId, int nChannelId, bool bLink )
{
	OLD_DO_LOCK(m_cs);

	CChannel* pChannel = GetChannel(nChannelId);

	if(0 == pChannel)
		return;

	pChannel->AddPlayer(nPlayerId);

	if(!bLink)
		return;

	__int64 v = nPlayerId;
	v = ( v << 8 ) | pChannel->GetMessageType();

	m_hmPlayerChannelMap[v] = pChannel->GetId();
}

// bClearLink 为一个标志， 此标志不清除可以方便玩家快速加回原来的频道
void CChannelManager::RemovePlayer( int nPlayerId, int nChannelId, bool bClearLink )
{
	OLD_DO_LOCK(m_cs);

	CChannel* pChannel = GetChannel(nChannelId);

	if(0 == pChannel)
		return;

	pChannel->RemovePlayer( nPlayerId );

	if( !bClearLink )
		return;

	__int64 v = nPlayerId;
	v = ( v << 8 ) | pChannel->GetMessageType();
	m_hmPlayerChannelMap.erase( v );
}

bool CChannel::HasPlayer( int nPlayreId )
{
	return m_playerList.find( nPlayreId ) != m_playerList.end();
}
