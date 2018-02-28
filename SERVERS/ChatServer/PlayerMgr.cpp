#include <winsock2.h>
#include "PlayerMgr.h"
#include "ChatServer.h"
#include "ServerMgr.h"
#include "WorldServer\LockCommon.h"

std::string CPlayerMgr::getPlayerName(U32 playerId)
{
	OLD_DO_LOCK(m_cs);

	ChatPlayerInfo* pPlayer = GetPlayerInfo(playerId);

	if (0 == pPlayer)
		return "";

	return pPlayer->playerName;
}

std::string CPlayerMgr::getAccountName(U32 playerId)
{
	OLD_DO_LOCK(m_cs);

	ChatPlayerInfo* pPlayer = GetPlayerInfo(playerId);

	if (0 == pPlayer)
		return "";

	return pPlayer->accountName;
}

std::string CPlayerMgr::getPlayerIp(U32 playerId)
{
	OLD_DO_LOCK(m_cs);

	ChatPlayerInfo* pPlayer = GetPlayerInfo(playerId);

	if (0 == pPlayer)
		return "";

	return pPlayer->loginIp;
}

U32 CPlayerMgr::getAccountId(U32 playerId)
{
	OLD_DO_LOCK(m_cs);

	ChatPlayerInfo* pPlayer = GetPlayerInfo(playerId);

	if (0 == pPlayer)
		return 0;

	return pPlayer->accountId;
}

U32 CPlayerMgr::getPlayerId(const char* name)
{
	OLD_DO_LOCK(m_cs);

	ChatPlayer* pPlayer = GetPlayer(name);

	if (0 == pPlayer)
		return 0;

	return pPlayer->m_playerInfo.playerId;
}

int CPlayerMgr::getSocketId(U32 playerId)
{
	OLD_DO_LOCK(m_cs);

	ChatPlayerInfo* pPlayer = GetPlayerInfo(playerId);

	if (0 == pPlayer)
		return 0;

	return SERVER->getServerMgr()->GetServer(pPlayer->lineId, pPlayer->gateId);
}

SendInfo CPlayerMgr::GetSendInfo(U32 playerId)
{
	OLD_DO_LOCK(m_cs);

	SendInfo rtnInfo;

	ChatPlayerInfo* pPlayer = GetPlayerInfo(playerId);

	if (0 == pPlayer)
		return rtnInfo;

	rtnInfo.accountId = pPlayer->accountId;
	rtnInfo.lineId = pPlayer->lineId;
	rtnInfo.socketId = SERVER->getServerMgr()->GetServer(pPlayer->lineId, pPlayer->gateId);

	return rtnInfo;
}

bool CPlayerMgr::GetPlayerInfo(U32 playerId, ChatPlayerInfo& info)
{
	OLD_DO_LOCK(m_cs);

	ChatPlayerInfo* pPlayer = GetPlayerInfo(playerId);

	if (0 == pPlayer)
		return false;

	info = *pPlayer;
	return true;
}

ChatPlayer* CPlayerMgr::GetPlayer(U32 playerId)
{
	OLD_DO_LOCK(m_cs);

	HashPlayerAccountMap::iterator iter = m_players.find(playerId);

	if (iter == m_players.end())
	{
		return 0;
	}

	return iter->second;
}

ChatPlayer* CPlayerMgr::GetPlayer(const char* name)
{
	OLD_DO_LOCK(m_cs);

	HashNamePlayerMap::iterator iter = m_playerNames.find(name);

	if (iter == m_playerNames.end())
		return 0;

	return GetPlayer(iter->second);
}

ChatPlayerInfo* CPlayerMgr::GetPlayerInfo(U32 playerId)
{
	OLD_DO_LOCK(m_cs);

	HashPlayerAccountMap::iterator iter = m_players.find(playerId);

	if (iter == m_players.end())
	{
		return 0;
	}

	return &iter->second->m_playerInfo;
}

void CPlayerMgr::addPlayer(const ChatPlayerInfo& info)
{
	OLD_DO_LOCK(m_cs);

	HashPlayerAccountMap::iterator iter = m_players.find(info.playerId);

	ChatPlayer* player = 0;

	if (iter == m_players.end())
	{
		player = new ChatPlayer;

		if (0 == player)
			return;

		m_players[info.playerId] = player;
		m_playerNames[info.playerName] = info.playerId;
		m_accounts[info.accountId] = info.playerId;
	}
	else
	{
		player = iter->second;
	}

	player->m_playerInfo = info;
	player->m_lastUpdateTime = time(0);
}

void CPlayerMgr::removePlayer(U32 playerId)
{
	OLD_DO_LOCK(m_cs);

	HashPlayerAccountMap::iterator iter = m_players.find(playerId);

	if (iter == m_players.end())
		return;

	ChatPlayer* player = iter->second;

	if (0 != player)
	{
		m_playerNames.erase(player->m_playerInfo.playerName);
		delete player;
	}

	m_players.erase(iter);
}

void CPlayerMgr::RemoveAccount(U32 accountId, U32 newPlayerId)
{
	OLD_DO_LOCK(m_cs);

	ACCOUNTMAP::iterator iter = m_accounts.find(accountId);

	if (iter == m_accounts.end())
		return;

	if (iter->second == newPlayerId)
		return;

	removePlayer(iter->second);
	m_accounts.erase(iter);
}

//在玩家进入游戏时,会发送当前玩家的注册信息到聊天服务器
ServerEventFunction(CChatServer, HandleAddPlayer, "WORLD_CHAT_AddPlayer")
{
	ChatPlayerInfo info;
	pPack->readBits(8 * sizeof(info), &info);

	int PlayerId = info.playerId;

	if (0 == PlayerId)
		return false;

	//移除当前帐号的其他角色
	SERVER->getPlayerMgr()->RemoveAccount(info.accountId, PlayerId);
	SERVER->getPlayerMgr()->addPlayer(info);
	return true;
}

//玩家在登出gate后,会移除玩家的信息
ServerEventFunction(CChatServer, HandleRemovePlayer, "WORLD_CHAT_RemovePlayer")
{
	SERVER->getPlayerMgr()->RemoveAccount(phead->Id, 0);
	return true;
}
