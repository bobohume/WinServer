//#define DEFAULT_DO_LOCK
//
//#include "SavePlayerJob.h"
//
//#include "wintcp/AsyncSocket.h"
//#include "TeamManager.h"
//#include "Event.h"
//#include "PlayerMgr.h"
//#include "Common/PlayerStructEx.h"
//#include "WorldServer.h"
//#include "EventProcess.h"
//#include "base/WorkQueue.h"
//
//bool CSavePlayerJob::TimeProcess( bool bExit )
//{
//	SERVER->GetWorkQueue()->PostEvent(1,0,0,0,WQ_TIMER);
//
//	return true;
//}
//
//void CSavePlayerJob::PostPlayerData( unsigned long UID, stPlayerStruct* pPlayerData )
//{
//	OLD_DO_LOCK( m_cs );
//	
//	int nPlayerId = pPlayerData->BaseData.PlayerId;
//
//	int idx = allocPlayerStruct();
//	if( idx == -1 )
//	{
//		g_Log.WriteWarn( "Save Player List Full !" );
//		return ;
//	}
//
//	memcpy( &mPlayers[idx], pPlayerData, sizeof( stPlayerStruct ) );
//
//	m_playerList.push_back( idx );
//}
//
//void CSavePlayerJob::SetDataAgentSocketHandle( int nPlayerId, int nSocket )
//{
//	OLD_DO_LOCK( m_cs );
//
//	m_dataAgentMap[nPlayerId] = nSocket;
//}
//
//bool CSavePlayerJob::Update()
//{
//	OLD_DO_LOCK( m_cs );
//
//	if( m_playerList.begin() == m_playerList.end() )
//		return false;
//
//	while( m_playerList.begin() != m_playerList.end() )
//	{
//		int idx = m_playerList.front();
//		stPlayerStruct* pPlayerData = &mPlayers[idx];
//		int nPlayerId = pPlayerData->BaseData.PlayerId;
//		int SID = pPlayerData->BaseData.SaveUID;
//
//		SERVER->GetPlayerManager()->SavePlayerData( pPlayerData );
//
//		freePlayerStruct( idx );
//
//		m_playerList.pop_front();
//
//		CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM );
//		Base::BitStream SendPacket( buf.get(), MAX_PACKET_SIZE );
//		stPacketHead* pSendHead = IPacket::BuildPacketHead( SendPacket, DATAAGENT_WORLD_NotifyPlayerDataAck, nPlayerId, SERVICE_DATAAGENT, SID );
//		pSendHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );
//		SERVER->GetServerSocket()->Send( m_dataAgentMap[nPlayerId], SendPacket );
//
//		m_dataAgentMap.erase( nPlayerId );
//	}
//
//	return true;
//}
//
//int CSavePlayerJob::allocPlayerStruct()
//{
//	if( mFreeStack.size() == 0 )
//		return -1;
//
//	int idx = mFreeStack.top();
//	mFreeStack.pop();
//
//	return idx;
//}
//
//void CSavePlayerJob::freePlayerStruct( int index )
//{
//	mFreeStack.push( index );
//}
//
//CSavePlayerJob::CSavePlayerJob()
//{
//	for( int i = 0; i < MAX_SAVE_PLAYER_COUNT; ++i )
//	{
//		mFreeStack.push( i );
//	}
//}
