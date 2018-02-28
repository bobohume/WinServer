#include "DBLib\dbPool.h"
#include "AchievementMgr.h"
#include "WorldServer.h"
#include "PlayerMgr.h"
#include "wintcp/dtServerSocket.h"
#include "ServerMgr.h"
#include "Common/AchievementBase.h"
#include "DBLayer/Data/TBLAchievement.h"
#include "CopymapManager.h"

CAchManager::CAchManager() : ILockable( Lock::Achievement )
{
	//registerEvent( ZONE_WORLD_QueryAchievementRequest,	&CAchManager::HandleZoneAchievementRequest );
	//registerEvent( ZONE_WORLD_QueryAchRecordRequest,	&CAchManager::HandleZoneAchRecordRequest );
	registerEvent( ZONE_WORLD_SaveAchievemtnRequest,	&CAchManager::HandleZoneSaveAchDataRequest );
	registerEvent( ZONE_WORLD_SaveAchRecordRequest,		&CAchManager::HandleZoneSaveAchRecordRequest );
}

CAchManager::~CAchManager()
{
}

//向ZoneServer发送角色成就数据
void CAchManager::SendAchievementInfo(int nPlayerId)
{
	stAchievement ach;
	ach.nPlayerId = nPlayerId;

	CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM);
	Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );
	int ZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(nPlayerId);
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, ZONE_WORLD_QueryAchievementResponse, nPlayerId, SERVICE_ZONESERVER, ZoneId );

	TBLAchievement tb_achlist(SERVER_CLASS_NAME::GetActorDB());
	std::vector<std::pair<int, int> > achlist;
	DBError err = tb_achlist.loadlist( ach.nPlayerId, achlist);
	if(err != DBERR_NONE)
	{
		g_Log.WriteError("Query playerid = %d achievement failure.", ach.nPlayerId);
		return;
	}

	ach.Build((int)achlist.size());
	for(int i = 0; i < ach.nCount; i++)
	{
		ach.nIdList[i]		= achlist[i].first;
		ach.nUpTimeList[i]	= achlist[i].second;
	}

	ach.PackData(&sendPacket);

	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer( nPlayerId );

	if(!pAccount)
		return;

	int LineId = pAccount->GetLastLineId();
	stServerInfo* pServer = SERVER->GetServerManager()->GetGateServerByZoneId( LineId, ZoneId );
	if( !pServer )
		return ;
	SERVER->GetServerSocket()->Send( pServer->SocketId, sendPacket );
}

//向ZoneServer发送角色成就数据
void CAchManager::SendAchRecInfo(int nPlayerId)
{
	stAchRecord ach;
	ach.nPlayerId = nPlayerId;

	CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM);
	Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );
	int ZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(nPlayerId);
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, ZONE_WORLD_QueryAchRecordResponse, nPlayerId, SERVICE_ZONESERVER, ZoneId );

	TBLAchRecord tb_achlist(SERVER_CLASS_NAME::GetActorDB());
	std::vector<std::pair<int, int> > achlist;
	DBError err = tb_achlist.loadlist( ach.nPlayerId, achlist);
	if(err != DBERR_NONE)
	{
		g_Log.WriteError("Query playerid = %d achievement record failure.", ach.nPlayerId);
	}

	ach.Build((int)achlist.size());
	for(int i = 0; i < ach.nCount; i++)
	{
		ach.nIdList[i]		= achlist[i].first;
		ach.nCountList[i]	= achlist[i].second;
	}

	ach.PackData(&sendPacket);

	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer( nPlayerId );
	if(!pAccount)
		return;

	int LineId = pAccount->GetLastLineId();
	
	stServerInfo* pServer = SERVER->GetServerManager()->GetGateServerByZoneId( LineId, ZoneId );
	if( !pServer )
		return ;
	SERVER->GetServerSocket()->Send( pServer->SocketId, sendPacket );
}

//// ----------------------------------------------------------------------------
//// 处理来自ZoneServer请求查询角色成就的消息
//void CAchManager::HandleZoneAchievementRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
//{
//	DO_LOCK( Lock::Achievement );
//
//	stAchievement ach;
//	ach.nPlayerId = Packet->readInt(Base::Bit32);
//
//	CMemGuard buf( MAX_PACKET_SIZE );
//	Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );
//	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, ZONE_WORLD_QueryAchievementResponse, pHead->Id, SERVICE_ZONESERVER, pHead->SrcZoneId );
//
//	TBLAchievement tb_achlist(SERVER_CLASS_NAME::GetActorDB());
//	std::vector<std::pair<int, int> > achlist;
//	DBError err = tb_achlist.loadlist( ach.nPlayerId, achlist);
//	if(err != DBERR_NONE)
//	{
//		g_Log.WriteError("Query playerid = %d achievement failure.", ach.nPlayerId);
//	}
//
//	ach.Build((int)achlist.size());
//	for(int i = 0; i < ach.nCount; i++)
//	{
//		ach.nIdList[i]		= achlist[i].first;
//		ach.nUpTimeList[i]	= achlist[i].second;
//	}
//
//	ach.PackData(Packet);
//
//	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
//
//	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket );
//}

// ----------------------------------------------------------------------------
// 处理来自ZoneServer保存角色成就的请求
void CAchManager::HandleZoneSaveAchDataRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	DO_LOCK( Lock::Achievement );
	U32 id		= Packet->readInt(Base::Bit32);
	U32 uptime	= Packet->readInt(Base::Bit32);
	U32 credit	= Packet->readInt(Base::Bit32);
	TBLAchievement tb_achlist(SERVER_CLASS_NAME::GetActorDB());	
	tb_achlist.save(pHead->Id, id, uptime, credit);
}

//// ----------------------------------------------------------------------------
//// 处理来自ZoneServer查询角色成就记录的请求
//void CAchManager::HandleZoneAchRecordRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
//{
//	DO_LOCK( Lock::Achievement );
//
//	stAchRecord ach;
//	ach.nPlayerId = Packet->readInt(Base::Bit32);
//
//	CMemGuard buf( MAX_PACKET_SIZE );
//	Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );
//	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, ZONE_WORLD_QueryAchRecordResponse, pHead->Id, SERVICE_ZONESERVER, pHead->SrcZoneId );
//
//	TBLAchRecord tb_achlist(SERVER_CLASS_NAME::GetActorDB());
//	std::vector<std::pair<int, int> > achlist;
//	DBError err = tb_achlist.loadlist( ach.nPlayerId, achlist);
//	if(err != DBERR_NONE)
//	{
//		g_Log.WriteError("Query playerid = %d achievement record failure.", ach.nPlayerId);
//	}
//
//	ach.Build((int)achlist.size());
//	for(int i = 0; i < ach.nCount; i++)
//	{
//		ach.nIdList[i]		= achlist[i].first;
//		ach.nCountList[i]	= achlist[i].second;
//	}
//
//	ach.PackData(Packet);
//
//	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
//
//	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket );
//}

// ----------------------------------------------------------------------------
// 处理来自ZoneServer保存角色成就记录的请求
void CAchManager::HandleZoneSaveAchRecordRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	DO_LOCK( Lock::Achievement );
	U32 id		= Packet->readInt(Base::Bit32);
	U32 count	= Packet->readInt(Base::Bit32);
	TBLAchRecord tb_achlist(SERVER_CLASS_NAME::GetActorDB());	
	tb_achlist.save( pHead->Id, id, count);
}