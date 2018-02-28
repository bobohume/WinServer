#ifdef NTJ_SERVER
#include "Gameplay\ServerGameplayState.h"
#include "NetWork\ServerPacketProcess.h"
#endif

#ifdef NTJ_CLIENT
#include "Gameplay/ClientGameplayState.h"
#include "NetWork/UserPacketProcess.h"
#endif

#include "SyncObject.h"

#ifdef WORLDSERVER
#include "WorldServer\ServerMgr.h"
#endif

#if defined( NTJ_SERVER ) || defined( NTJ_CLIENT )
#include "NetWork\EventCommon.h"
#endif

SyncManager SyncManager::instance;

VOID CALLBACK sync_apc( ULONG_PTR dwParam )
{

}

void SyncObject::packUpdate( Base::BitStream& stream/*, SyncMask syncMask*/ )
{
	stream.writeInt(mSyncMask,32);

	if( mSyncMask & InitMask )
	{
		stream.writeInt( mSyncObjectId, 32 );
	}
}

void SyncObject::unpackUpdate( Base::BitStream& stream )
{
	mSyncMask = stream.readInt( 32 );

	if( mSyncMask & InitMask  )	// InitMask
	{
		mSyncObjectId = stream.readInt( 32 );
	}
}

void SyncObject::registerObject()
{
	SyncManager::getInstance()->registerObject( *this );

	onAdd();
}

void SyncObject::unregisterObject()
{
	onRemove();

	SyncManager::getInstance()->unregisterObject( *this );

	delete this;
}

void SyncObject::setSyncMask( SyncMask syncMask )
{   
    //这里貌似不是线程安全
	mSyncMask |= syncMask;

#ifdef WORLDSERVER
	SyncManager::getInstance()->setSyncMask( this, syncMask );
#endif
}

void SyncObject::clearSyncMask()
{
	mSyncMask = 0;

#ifdef WORLDSERVER
	SyncManager::getInstance()->clearSyncMask( this );
#endif
}

//SyncException::SyncException( std::string text ) : mText( text )
//{
//#ifdef WORLDSERVER
//	char buf[256];
//	sprintf_s( buf, sizeof( buf ), "SyncException : %s \n\r", text.c_str() );
//	g_Log.WriteError( buf );
//
//	OutputDebugStringA( buf );
//#endif
//}

//SyncException::~SyncException()
//{
//
//}

SyncManager::SyncManager()
#ifdef WORLDSERVER
: ILockable( Lock::Sync )
#endif
{
#ifdef WORLDSERVER
	mSyncObjIdSeed = 0;
	registerEvent( SYNC_EVENT, &SyncManager::OnSyncEvent );

	mWorkThread.WorkMethod.hook( this, &SyncManager::syncThread );
#endif

#if defined( NTJ_SERVER ) || defined( NTJ_CLIENT )
	gEventMethod[SYNC_EVENT] = &SyncManager::onReceive;
#endif
}

void SyncManager::registerObject( SyncObject& object )
{
#ifdef WORLDSERVER
	OLD_DO_LOCK( m_cs );

	object.SetSyncID( mSyncObjIdSeed++ );
#endif 

	SyncObjectMap::iterator it;
	it = mSyncObjectMap.find( object.GetSyncID() );
	if( it != mSyncObjectMap.end() )
	{
		throw std::exception("SyncManager : registerObject : object has already been registered !");
		return ;
	}

	mSyncObjectMap[object.GetSyncID()] = &object;
}

void SyncManager::unregisterObject( SyncObject& object )
{
#ifdef WORLDSERVER
	OLD_DO_LOCK( m_cs );
#endif

	SyncObjectMap::iterator it;
	it = mSyncObjectMap.find( object.GetSyncID() );
	if( it == mSyncObjectMap.end() )
	{
		throw std::exception("SyncManager : unregisterObject : object has not been registered !");
		return ;
	}

	mSyncObjectMap.erase( it );


#ifdef WORLDSERVER
	for each( std::pair<int, ClientSocketRef> p in mScopeMap[&object] )
	{
		ClientSocketRef client = p.second;
		client->mObjectSet.erase( &object );

		CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM );
		Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );
		stPacketHead* pHead = IPacket::BuildPacketHead( sendPacket, SYNC_EVENT );
		pHead->SrcZoneId = 2;
		pHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		client->send( sendPacket );
	}

	mScopeMap.erase( &object );
#endif
}

SyncObject* SyncManager::getObject( int id )
{
#ifdef WORLDSERVER
	OLD_DO_LOCK( m_cs );
#endif
	SyncObjectMap::iterator it;

	it = mSyncObjectMap.find( id );
	if( it != mSyncObjectMap.end() )
		return it->second;

	return NULL;
}

#if defined( NTJ_SERVER ) || defined( NTJ_CLIENT )
void SyncManager::onReceive( stPacketHead* pHead, Base::BitStream& stream )
{
	switch( pHead->SrcZoneId )
	{
	case 0:	// 处理GHOST
		{
			char className[255];
			stream.readString( className, 255 );
			int id = stream.readInt( 32 );

			SyncObject* obj = getObject( id );
			if( obj == NULL )
			{
				CTI::TypeInfo::TypeInfoMap::iterator it;
				it = CTI::TypeInfo::getTypeInfoMap().find( className );
				if( it == CTI::TypeInfo::getTypeInfoMap().end() )
					break;

				CTI::TypeInfo* info = it->second;
				obj = dynamic_cast< SyncObject* >( info->createObject() );
				if( obj == NULL )
					break;
				obj->SetSyncID( id );
				obj->unpackUpdate( stream );
				obj->registerObject();
			}
			else
			{
				obj->unpackUpdate( stream );
			}
			obj->process( mWorldConn );
		}
		break;
	case 1: // 处理EVENT
		{
			char className[255];
			stream.readString( className, 255 );

			CTI::TypeInfo::TypeInfoMap::iterator it;
			it = CTI::TypeInfo::getTypeInfoMap().find( className );
			if( it == CTI::TypeInfo::getTypeInfoMap().end() )
				break;

			CTI::TypeInfo* info = it->second;
			SyncEvent* obj = dynamic_cast< SyncEvent* >( info->createObject() );
			if( obj == NULL )
				break;

			obj->unpackData( stream );
			obj->process( mWorldConn );
		}
		break;
	case 2:	// ghost 删除
		{
			int id = stream.readInt( 32 );
			SyncObject* obj = getObject( id );
			if( obj )
				obj->unregisterObject();
		}
		break;
	}
}


void SyncManager::WorldConnection::postEvent( SyncEvent& event )
{
	CMemGuard buf(MAX_PACKET_SIZE MEM_GUARD_PARAM);
	Base::BitStream sendPakcet( buf.get(), MAX_PACKET_SIZE );

	stPacketHead* pHead = IPacket::BuildPacketHead( sendPakcet, SYNC_EVENT, 0, SERVICE_WORLDSERVER );


	sendPakcet.writeString( event.getTypeInfo()->mClassName.c_str() );

	event.packData( sendPakcet );

	pHead->PacketSize = sendPakcet.getPosition() - sizeof( stPacketHead );

#ifdef NTJ_CLIENT
	pHead->Id = g_ClientGameplayState->getAccountId();
	UserPacketProcess* pProcess = g_ClientGameplayState->GetPacketProcess();
	if( pProcess )
		pProcess->Send( sendPakcet );
#endif
	
#ifdef NTJ_SERVER
	pHead->SrcZoneId = g_ServerGameplayState->getZoneId();
	ServerPacketProcess* pProcess = g_ServerGameplayState->GetPacketProcess();
	if( pProcess )
		pProcess->Send( sendPakcet );
#endif
}
#endif

#ifdef WORLDSERVER
void SyncManager::setSyncMask( SyncObject* obj, SyncMask mask )
{
	OLD_DO_LOCK( m_cs );

	for each( std::pair<int, ClientSocketRef> p in mScopeMap[obj] )
	{
		ClientSocketRef client = p.second;
		client->mObjectSet[obj] |= mask;
	}

	QueueUserAPC(sync_apc,mWorkThread.getHandle(),0);
}

void SyncManager::clearSyncMask( SyncObject* obj )
{
	OLD_DO_LOCK( m_cs );

	for each( std::pair<int, ClientSocketRef> p in mScopeMap[obj] )
	{
		ClientSocketRef client = p.second;
		client->mObjectSet[obj] = 0;
	}
}

// 处理Event消息
void SyncManager::OnSyncEvent( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	char className[255];
	Packet->readString( className, 255 );

	CTI::TypeInfo::TypeInfoMap::iterator it;
	it = CTI::TypeInfo::getTypeInfoMap().find( className );
	if( it == CTI::TypeInfo::getTypeInfoMap().end() )
		return ;

	CTI::TypeInfo* info = it->second;
	SyncEvent* obj = dynamic_cast< SyncEvent* >( info->createObject() );
	if( obj == NULL )
		return ;

	obj->unpackData( *Packet );
	if( pHead->SrcZoneId != 0 )
	{
		ZoneSocketRef pSocket = SERVER->GetServerManager()->GetZone_SyncSocket( SERVER->GetServerManager()->GetLineId( SocketHandle ), pHead->SrcZoneId );
		if( pSocket )
		{
			obj->process( *pSocket );
		}
	}
	else
	{	

	}
}



void SyncManager::syncThread( CTI::Thread* thread )
{
	std::list< SyncObject* > objList;

	while( 1 )
	{
		SleepEx( -1, true );

		ScopeMap::iterator it;

		m_cs.Lock();

		objList.clear();

		for( it = mScopeMap.begin(); it != mScopeMap.end(); it++ )
		{
			objList.push_back( it->first );
		}

		m_cs.Unlock();

		for each( SyncObject* obj in objList )
		{
			if( obj->getSyncMask() != 0 )
			{
				sendObject( obj );

				obj->clearSyncMask();
			}
		}
	}
}

void SyncManager::addClient( ClientSocketRef client )
{
	if( !client )
		return ;

	OLD_DO_LOCK( m_cs );
	mClientSet[client->mId] = client;
}

void SyncManager::removeClient( ClientSocketRef client )
{
	OLD_DO_LOCK( m_cs );

	if( !client )
		return ;

	clientSocket::ObjectSet::iterator it;

	for( it = client->mObjectSet.begin(); it != client->mObjectSet.end(); it++ )
	{
		mScopeMap[it->first].erase( client->mId );
	}

	mClientSet.erase( client->mId );
}

void SyncManager::addScope( ClientSocketRef client, SyncObject* obj )
{
	OLD_DO_LOCK( m_cs ); 

	if( !client || !obj )
		return ;

	client->mObjectSet[obj] = SyncMaskFull;
	
	ClientSet::iterator itr = mScopeMap[obj].find(client->mId);
	if ( itr == mScopeMap[obj].end() )
	{
		mScopeMap[obj][client->mId] = client;
	}

	// 此处触发更新
	QueueUserAPC(sync_apc,mWorkThread.getHandle(),0);
}

//void SyncManager::removeScope( clientSocket* client, SyncObject* obj )
//{
//	OLD_DO_LOCK( m_cs );
//	client->mObjectSet.erase( obj );
//
//	mScopeMap[obj].erase( client );
//}

void SyncManager::sendObject( SyncObject* obj )
{
	if( !obj )
		return ;

	std::list< ClientSocketRef > clientList;

	{
		OLD_DO_LOCK( m_cs );

		for each( std::pair<int, ClientSocketRef> p in mScopeMap[obj] )
		{
			ClientSocketRef client = p.second;
			clientList.push_back( client );
		}
	}

	for each( ClientSocketRef client in clientList )
	{
		CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM );
		Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );
		stPacketHead* pHead = IPacket::BuildPacketHead( sendPacket, SYNC_EVENT );
		pHead->SrcZoneId = 0;
		sendPacket.writeString( obj->getTypeInfo()->mClassName.c_str() );
		sendPacket.writeInt( obj->GetSyncID(), 32 );
		obj->packUpdate( sendPacket/*, client->mObjectSet[obj]*/ );

		pHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );

		client->send( sendPacket );
	}
}

void SyncManager::scopeToAllZone( SyncObject& obj )
{
	OLD_DO_LOCK( m_cs );

	std::list< ZoneSocketRef > socketList;
	SERVER->GetServerManager()->GetAllZone_SyncSocket( socketList );

	for each( ZoneSocketRef client in socketList )
	{
		addScope( client, &obj );
	}
}

void SyncManager::scopeAllObjects( ClientSocketRef client )
{
	OLD_DO_LOCK( m_cs );

	for each( const std::pair< int, SyncObject* >& objs in mSyncObjectMap )
	{
		addScope( client, objs.second );
	}
}

void SyncManager::clientSocket::postEvent( SyncEvent& event )
{
	CMemGuard buf(MAX_PACKET_SIZE MEM_GUARD_PARAM);
	Base::BitStream sendPakcet( buf.get(), MAX_PACKET_SIZE );

	stPacketHead* pHead = IPacket::BuildPacketHead( sendPakcet, SYNC_EVENT );
	
	pHead->SrcZoneId = 1;

	sendPakcet.writeString( event.getTypeInfo()->mClassName.c_str() );

	event.packData( sendPakcet );

	pHead->PacketSize = sendPakcet.getPosition() - sizeof( stPacketHead );
	send( sendPakcet );
}

volatile LONG SyncManager::clientSocket::mIdSeed = 100000;


void SyncManager::playerSocket::send( Base::BitStream& stream )
{
	 stPacketHead* pHead = ( stPacketHead* )stream.getBuffer();
	 pHead->DestServerType = SERVICE_CLIENT;
	 pHead->Id = accountId;

	 SERVER->GetServerSocket()->Send( socketId, stream );
}

void SyncManager::zoneSocket::send( Base::BitStream& stream )
{
	stPacketHead* pHead = (stPacketHead* )stream.getBuffer();
	pHead->DestServerType = SERVICE_ZONESERVER;
	pHead->DestZoneId = zoneId;

	SERVER->GetServerSocket()->Send( socketId, stream );
}
#endif
