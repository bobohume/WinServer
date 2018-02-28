#pragma once

// CNetManager 命令目标

#include "Packet.h"
#include "NetSession.h"

#include <hash_map>
#include <exception>
#include "netlib_core.h"


template< int _NetMsgCount, typename _Ty, typename _Session >
class CNetServer : public CNetEventHandler< _NetMsgCount, _Ty >
{
public:
	CNetServer( int nPort = 8511)
	{
#ifndef TCP_SERVER
		CNetLib::Init();
#endif

		mServerSocket = socket( AF_INET, SOCK_DGRAM, 0 );

		if( mServerSocket == INVALID_SOCKET )
		{
			throw std::exception( "创建SOCKET失败" );
		}

		SOCKADDR_IN		addr;
		memset( &addr, 0, sizeof( SOCKADDR_IN ) );
		addr.sin_family      = AF_INET;
		addr.sin_port        = htons(nPort);

		if( bind( mServerSocket, (SOCKADDR *)&addr, sizeof( addr ) ) == SOCKET_ERROR )
		{
			throw std::exception( "无法绑定到制定到网卡" );
		}

		mThreadHandle = (HANDLE)_beginthread( WorkThread, 0, this );

		//mMonitorHandle = (HANDLE)_beginthread( MonitorThread, 0, this );
	}

	virtual ~CNetServer()
	{
		// force to shutdown the work thread
		TerminateThread( mThreadHandle, 0 );

#ifndef TCP_SERVER
		CNetLib::Shutdown();
#endif

		SessionIdMap::iterator it;
		for( it = mSessionIdMap.begin(); it != mSessionIdMap.end(); it++ )
		{
			delete it->second;
		}

	}

public:
	void sendPacket( CNetSession::ID_HANDLE id, CPacketStream& packetStream )
	{
		sendPacket( id, (char*)packetStream.getBuffer(), packetStream.getPosition() );
	}

	void sendPacket( CNetSession::ID_HANDLE id, char* buf, int length )
	{
		SessionIdMap::iterator it;
		it = mSessionIdMap.find( id );

		if( it == mSessionIdMap.end() )
			return ;

		CNetSession* pSession = it->second;

		sendto( mServerSocket, buf, length, 0, (SOCKADDR*)pSession->getAddress(), sizeof( SOCKADDR ) );
	}
    
    //获取ip地址
    const char* GetRemoteIP(CNetSession::ID_HANDLE id)
    {
        SessionIdMap::iterator it;
        it = mSessionIdMap.find( id );

        if( it == mSessionIdMap.end() )
            return "";
        
        CNetSession* pSession = it->second;

        return inet_ntoa((in_addr)pSession->getAddress()->sin_addr);
    }
private:
	static _Ty* getEventHandler()
	{
		static _Ty	mEventHandler;
		return &mEventHandler;
	}

	SOCKET				mServerSocket;
	HANDLE				mThreadHandle;
	HANDLE				mMonitorHandle;

	typedef stdext::hash_map< __int64, CNetSession* > SessionMap;
	typedef stdext::hash_map< CNetSession::ID_HANDLE, CNetSession* > SessionIdMap;

	SessionMap			mSessionMap;
	SessionIdMap		mSessionIdMap;

private:
	static void doCheck( CNetServer* pThis )
	{
		SessionMap::iterator it;

		DWORD dwTime = ::GetTickCount();

		for( it = pThis->mSessionMap; it != pThis->mSessionMap.end(); it++ )
		{
			// 如果此连接30秒没有活动，主动断开
			if( dwTime - it->second > 3000 )
			{
				it->second.onTimeout();
				it->second.onDisconnect();

				it = pThis->mSessionMap.erase( it );
			}
		}
	}

	static void WorkThread( void* param )
	{
		char buf[t_IPacket::MAX_PACKET_SIZE];
		SOCKADDR_IN addr;
		memset( &addr, 0, sizeof( addr ) );
		int length;
		t_packetHeader packetHeader;
		CNetServer* pThis = ( CNetServer* )param;

		char protocol;
		int len = sizeof( addr );

		while( 1 )
		{
			length = recvfrom( pThis->mServerSocket, buf, t_IPacket::MAX_PACKET_SIZE, 0, ( SOCKADDR* )&addr, &len );
			
			if( length )
			{
				CPacketStream packetStream( buf, t_IPacket::MAX_PACKET_SIZE );
				packetStream >> protocol;

				// check the protocol
				__int64 key;
				key = (__int64)( addr.sin_addr.S_un.S_addr ) << 32 | addr.sin_port;

				SessionMap::iterator it =  pThis->mSessionMap.find( key );

				if( it == pThis->mSessionMap.end() )
				{
					if( protocol != CNetSession::CONNECT )
						continue;

					CNetSession* pNewSession = new _Session();
					pNewSession->onConnected();

					pThis->mSessionMap.insert( SessionMap::value_type( key, pNewSession ) );
					memcpy( pNewSession->getAddress(), &addr, sizeof( SOCKADDR_IN ) );

					pThis->mSessionIdMap[pNewSession->getId()] = pNewSession;

					// ack
					pThis->sendPacket( pNewSession->getId(), &protocol, 1 );
				}
				else
				{
					CNetSession* pSession = it->second;

					pSession->LastTickTime = ::GetTickCount();

					switch( protocol )
					{
					case CNetSession::DISCONNECT:
						
						pSession->onDisconnect();

						// ack
						pThis->sendPacket( pSession->getId(), &protocol, 1 );
						
						pThis->mSessionIdMap.erase( pSession->getId() );

						delete pSession;
						
						pThis->mSessionMap.erase( it );
						break;
					case CNetSession::CONNECT:		//ignore
						
						break;
					case CNetSession::TICK:
						// update the tick time
						pSession->onTick();

						// ack
						pThis->sendPacket( pSession->getId(), &protocol, 1 );
						break;
					case CNetSession::LOGIC:
						// handle net message
						packetStream >> packetHeader;

						if( packetHeader.Message >= 0 && packetHeader.Message < _NetMsgCount )
							pThis->onEvent( packetHeader.Message, pSession->getId(), &packetHeader, &packetStream );

						break;
					}
				}
			}
		}
	}
};


