#pragma once

#include <process.h>

template< int _NetMsgCount, typename _Ty >
class CNetClient : public CNetEventHandler< _NetMsgCount, _Ty >
{
public:
	typedef CNetClient< _NetMsgCount, _Ty > Parent;

	enum StateTypes
	{
		IDLE,
		CONNECT,
		DISCONNECT,
		IN_WORK
	};
 
	StateTypes getState()
	{
		return mState;
	}

	static const int DefaultTimeout = 30000;	// 超时时间 30秒

	CNetClient()
	{
		mState = IDLE;

		mTimeout = DefaultTimeout;

#ifndef TCP_SERVER
		CNetLib::Init();
#endif

		mClientSocket = socket( AF_INET, SOCK_DGRAM, 0 );

		if( mClientSocket == INVALID_SOCKET )
		{
			throw std::exception( "无法创建Socket" );
		}

		SOCKADDR_IN		addr;
		memset( &addr, 0, sizeof( SOCKADDR_IN ) );
		addr.sin_family = AF_INET;

		if( bind( mClientSocket, (SOCKADDR *)&addr, sizeof( addr ) ) == SOCKET_ERROR )
		{
			throw std::exception( "无法绑定到制定到网卡" );
		}

		mIoHandle = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, NULL, 1 );
		
		mSendThread = (HANDLE)_beginthread( SendThread, 0, this );
		mWorkThreadHandle = (HANDLE)_beginthread( WorkThread, 0, this );
		mTickThreadHandle = (HANDLE)_beginthread( TickThread, 0, this );
	}

	virtual ~CNetClient()
	{
		// force to shutdown the work thread
		TerminateThread( mWorkThreadHandle, 0 );
		TerminateThread( mTickThreadHandle, 0 );
		TerminateThread( mSendThread, 0 );
		CloseHandle( mIoHandle );

#ifndef TCP_SERVER
		CNetLib::Shutdown();
#endif
	}

	// set remote server
	void setRemoteServer( std::string ipAddress, int port )
	{
		memset( &mRemoteAddr, 0, sizeof( SOCKADDR_IN ) );
		mRemoteAddr.sin_family = AF_INET;
		mRemoteAddr.sin_port = htons( port );
		mRemoteAddr.sin_addr.S_un.S_addr = inet_addr( ipAddress.c_str() );
	}
	
	// connect to a remote server
	void connect( std::string ipAddress, int port )
	{
		setRemoteServer( ipAddress, port );
		mState = CONNECT;
		char protocol = CNetSession::CONNECT;
		mLastTickTime = ::GetTickCount();
		sendPacket( &protocol, 1 );
	}

	// disconnect from the remote server
	void disconnect()
	{
		mState = DISCONNECT;
		char protocol = CNetSession::DISCONNECT;
		sendPacket( &protocol, 1 );	
	}

	// send tick packet
	void doTick()
	{
		char protocol;

		if( mState == CONNECT )
			protocol = CNetSession::CONNECT;
		else if( mState == IN_WORK )
			protocol = CNetSession::TICK;
		else if( mState == DISCONNECT )
			protocol = CNetSession::DISCONNECT;

		sendPacket( &protocol, 1 );
	}

	virtual void onConnected()		{;}
	virtual void onDisconnected()	{;}

	bool sendPacket( CPacketStream& packetStream )
	{
		return sendPacket( (char*)packetStream.getBuffer(), packetStream.getPosition() );
	}

	bool sendPacket( char* buf, int length )
	{
		char* pBuf = new char[length];

		memcpy( pBuf, buf, length );

		::PostQueuedCompletionStatus( mIoHandle, length, (ULONG_PTR)pBuf, NULL );

		return true;
		//return SOCKET_ERROR != sendto( mClientSocket, buf, length, 0, (SOCKADDR*)&mRemoteAddr, sizeof( SOCKADDR ) );
	}
private:

	struct t_sendItem
	{
		char* buf;
		int length;
	};

	static void TickThread( void* param )
	{
		CNetClient* pThis = ( CNetClient* )param;
		
		while( 1 )
		{
			::Sleep( 1000 );

			pThis->doTick();


			if( ::GetTickCount() - pThis->mLastTickTime > pThis->mTimeout )
			{
				if( pThis->mState == CONNECT  )
				{
					pThis->mState = IDLE;
					pThis->onTimeout();
				}

				if( pThis->mState == IN_WORK )
				{
					pThis->mState = IDLE;
					pThis->onDisconnected();
				}
			}

		}
	}

	static void SendThread( void* param )
	{
		CNetClient* pThis = ( CNetClient* )param;

		DWORD dwKey = 0;
		DWORD dwSize = 0;
		char* pKey = 0;
		LPOVERLAPPED Over;
		while( 1 )
		{
			::GetQueuedCompletionStatus( pThis->mIoHandle, &dwSize, (PULONG_PTR)&pKey, &Over, -1 );

			if( dwSize > 0 && pKey != NULL )
			{
				sendto( pThis->mClientSocket, pKey, dwSize, 0, (SOCKADDR*)&pThis->mRemoteAddr, sizeof( SOCKADDR ) );
				
				delete[] pKey;
			}
			else
			{
				break;
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
		CNetClient* pThis = ( CNetClient* )param;

		char protocol;
		int len = sizeof( addr );

		while( 1 )
		{
			length = recvfrom( pThis->mClientSocket, buf, t_IPacket::MAX_PACKET_SIZE, 0, ( SOCKADDR* )&addr, &len );

			if( length )
			{
				CPacketStream packetStream( buf, t_IPacket::MAX_PACKET_SIZE );
				packetStream >> protocol;

				if( protocol == CNetSession::CONNECT && pThis->mState == CONNECT )
				{
					pThis->onConnected();
					pThis->mState = IN_WORK;
					continue;
				}

				if( protocol == CNetSession::DISCONNECT && pThis->mState == DISCONNECT )
				{
					pThis->onDisconnected();
					pThis->mState = IDLE;
					continue;
				}

				if( protocol == CNetSession::TICK && pThis->mState == IN_WORK )
				{
					pThis->mLastTickTime = ::GetTickCount();
				}

				if( protocol != CNetSession::LOGIC )
					continue;
				
				packetStream >> packetHeader;

				pThis->onEvent( packetHeader.Message, CNetSession::SERVER_ID_HANDLE, &packetHeader, &packetStream );
			}
		}	
	}

	virtual void onTimeout() 
	{
		;  // do nothing
	}

private:
	SOCKET			mClientSocket;
	HANDLE			mWorkThreadHandle;
	HANDLE			mTickThreadHandle;
	SOCKADDR_IN		mServerAddr;
	SOCKADDR_IN		mRemoteAddr;
	int				mState;

	HANDLE			mIoHandle;
	HANDLE			mSendThread;

	DWORD			mLastTickTime;
	volatile DWORD	mTimeout;
} ;
