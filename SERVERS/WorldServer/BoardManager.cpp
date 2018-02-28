/*
#include "BoardManager.h"
#include "DBLayer\Data\TBLBoard.h"

#include "DB_BoardList.h"
#include "DB_ReadBoard.h"
#include "DB_SaveBoard.h"
#include "DB_DeleteBoard.h"
#include "DB_BoardAddHot1.h"
#include "DB_BoardAddHot2.h"

CBoardManager::CBoardManager()
{
	registerEvent( CLIENT_WORLD_BoardListRequest, &CBoardManager::HandleClientListRequest );
	registerEvent( CLIENT_WORLD_BoardLoadRequest, &CBoardManager::HandleClientLoadRequest );
	registerEvent( CLIENT_WORLD_BoardSaveRequest, &CBoardManager::HandleClientSaveRequest );
	registerEvent( CLIENT_WORLD_BoardDelRequest,  &CBoardManager::HandleClientDeleteRequest );
	registerEvent( CLIENT_WORLD_BoardHot1Request, &CBoardManager::HandleClientAddHot1 );
	registerEvent( CLIENT_WORLD_BoardHot2Request, &CBoardManager::HandleClientAddHot2 );
}

CBoardManager::~CBoardManager()
{

}

void CBoardManager::HandleClientAddHot1( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	DO_LOCK( Lock::Board );

	int playerId = pHead->Id;
	int id       = pHead->DestZoneId;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

	if(NULL == pAccount)
		return;

	DBError err = DBERR_NONE;

	if( mPlayerHotMap[playerId].find( id ) != mPlayerHotMap[playerId].end() )
	{
		err = DBERR_PLAYER_TOOMANY;
	}
	else
	{
		mPlayerHotMap[playerId].insert( id );
	}

	DB_BoardAddHot1* pDBHandle = new DB_BoardAddHot1;

    pDBHandle->m_socketHandle = pAccount->GetGateSocketId();
    pDBHandle->m_accountId	  = pAccount->GetAccountId();
    pDBHandle->m_err	      = err;
    pDBHandle->m_id           = id;

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}

void CBoardManager::HandleClientAddHot2( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	DO_LOCK( Lock::Board );

	int playerId = pHead->Id;
	int id       = pHead->DestZoneId;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

	if(NULL == pAccount)
		return;

	DBError err = DBERR_NONE;

	if( mPlayerHotMap[playerId].find( id ) != mPlayerHotMap[playerId].end() )
	{
		err = DBERR_PLAYER_TOOMANY;
	}
	else
	{
		mPlayerHotMap[playerId].insert( id );
	}

    DB_BoardAddHot2* pDBHandle = new DB_BoardAddHot2;

    pDBHandle->m_socketHandle = pAccount->GetGateSocketId();
    pDBHandle->m_accountId	  = pAccount->GetAccountId();
    pDBHandle->m_err	      = err;
    pDBHandle->m_id           = id;

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}

void CBoardManager::HandleClientListRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	int playerId = pHead->Id;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

	if(NULL == pAccount)
		return;

	int pageNo    = pHead->DestZoneId;
	int pageCount = pHead->SrcZoneId;
	int type      = pHead->LineServerId;

    DB_BoardList* pDBHandle = new DB_BoardList;

    pDBHandle->m_socketHandle = pAccount->GetGateSocketId();
    pDBHandle->m_accountId	  = pAccount->GetAccountId();
    pDBHandle->m_pageNo	      = pageNo;
    pDBHandle->m_pageCount    = pageCount;
    pDBHandle->m_type         = type;

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}

void CBoardManager::HandleClientDeleteRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	int playerId = pHead->Id;
	int id       = pHead->DestZoneId;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

	if(NULL == pAccount)
		return;

	mPlayerTimeMap[playerId] = ::GetTickCount();

    DB_DeleteBoard* pDBHandle = new DB_DeleteBoard;

    pDBHandle->m_socketHandle = SocketHandle;
    pDBHandle->m_accountId	  = pAccount->GetAccountId();
    pDBHandle->m_playerId	  = playerId;
    pDBHandle->m_id           = id;

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}

void CBoardManager::HandleClientSaveRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	int playerId = pHead->Id;
	int isFirst  = pHead->DestZoneId;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

    if(!pAccount)
        return;

    bool isAllowed = true;

	if( isFirst != 0 )
	{
		PlayerTimeMap::iterator it = mPlayerTimeMap.find( playerId );
		if( it != mPlayerTimeMap.end() )
		{
			int time = it->second;
			if( time != 0 && ::GetTickCount() - time < 24 * 3600 * 1000 )
			{
				isAllowed = false;
			}
		}
	}

	DBError err = DBERR_NONE;

    if( !isAllowed )
		err = DBERR_PLAYER_TOOMANY;

    DB_SaveBoard* pDBHandle = new DB_SaveBoard;

    pDBHandle->m_socketHandle = pAccount->GetGateSocketId();
    pDBHandle->m_accountId	  = pAccount->AccountId;
    pDBHandle->m_err          = err;

    if (isAllowed)
    {
        pDBHandle->m_msg.readData(Packet); 
    }

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}

void CBoardManager::HandleClientLoadRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	int playerId     = pHead->Id;
	int destPlayerId = pHead->DestZoneId;
	int type         = pHead->SrcZoneId;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

	if( NULL == pAccount )
		return;

	DB_ReadBoard* pDBHandle = new DB_ReadBoard;

    pDBHandle->m_socketHandle = pAccount->GetGateSocketId();
    pDBHandle->m_accountId	  = pAccount->AccountId;
    pDBHandle->m_destPlayerId = destPlayerId;
    pDBHandle->m_type         = type;

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}
*/