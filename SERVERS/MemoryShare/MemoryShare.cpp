//#include <windows.h>
//
//struct MemoryShareAPI
//{
//	static void* create( const char* MemoryShareName, int size )
//	{
//		HANDLE hMemoryFile = OpenFileMappingA( FILE_MAP_ALL_ACCESS, FALSE, MemoryShareName );
//		if( hMemoryFile != NULL )
//			return NULL;
//
//		hMemoryFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, size, MemoryShareName );
//
//		if( hMemoryFile == INVALID_HANDLE_VALUE )
//			return NULL;
//
//		void* lpBuffer = (LPVOID)MapViewOfFile( hMemoryFile, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
//
//		if( lpBuffer == NULL )
//			return NULL;
//		
//		return lpBuffer;
//	}
//
//	static void free( const char* MemoryShareName )
//	{
//		HANDLE hMemoryFile = OpenFileMappingA( FILE_MAP_ALL_ACCESS, FALSE, MemoryShareName );
//
//		if( hMemoryFile == NULL )
//			return ;
//
//		CloseHandle( hMemoryFile );
//	}
//};
//
//#define MSAPI _cdecl
//
//struct MemoryShareCallUnit
//{
//	char	mName[256];
//	int		mBufferLength;
//	char	mBuffer[1024];
//	__int64 callId;
//
//	MemoryShareCallUnit()
//	{
//		memset( this, 0, sizeof( MemoryShareCallUnit ) );
//	}
//};
//
//struct MemoryShareCallResponse
//{
//	__int64 callId;
//	int ack;
//};
//
//template< typename _Ty >
//class MemoryShareCallManager
//{
//	typedef MSAPI int ( _Ty::*MemoryShareCall)( ... );	
//
//	typedef std::hash_map< __int64, LPVOID > FiberMap;
//	typedef stdext::hash_map< std::string, MemoryShareCall > CallMap;
//
//	static CallMap mCallMap;
//	static FiberMap mFiberMap;
//	static int mAckCode;
//	static HANDLE threadHandle;
//	static HANDLE mPipe;
//	static _Ty* mObj;
//	static const int BUFSIZE = 2048;
//public:
//	enum ModeType
//	{
//		Mode_Client,
//		Mode_Server,
//	};
//
//	static void init( ModeType modeType = Mode_Server )
//	{
//		// server mode
//		if( modeType == Mode_Server )
//		{
//			char namePipe[128];
//			sprintf_s( namePipe, "%s_pipe", typeid(_Ty).name );
//
//			mPipe = CreateNamedPipe( 
//				namePipe,				  // pipe name 
//				PIPE_ACCESS_DUPLEX,       // read/write access 
//				PIPE_TYPE_MESSAGE |       // message type pipe 
//				PIPE_READMODE_MESSAGE |   // message-read mode 
//				PIPE_WAIT,                // blocking mode 
//				PIPE_UNLIMITED_INSTANCES, // max. instances  
//				BUFSIZE,                  // output buffer size 
//				BUFSIZE,                  // input buffer size 
//				0,                        // client time-out 
//				NULL); 
//
//			threadHandle = (HANDLE)_beginthreadex( NULL, 0, callResponseWork, this, 0, NULL );
//		}
//		else
//		{
//			// client mode
//			
//
//		}
//	}
//
//	static void attachObject( _Ty* obj )
//	{
//		mObj = obj;
//	}
//
//	static void shutdown()
//	{
//		TerminateThread( threadHandle, 0 );
//	}
//
//	static int invoke( const char* name )
//	{
//		static __int64 callIdSeed = 0;
//		MemoryShareCallUnit unit;
//		unit.callId = ++callIdSeed;
//		strcpy_s( unit.mName, name );
//		postCall( unit );
//
//		int ackCode;
//		LPVOID fiber = ConvertThreadToFiber( NULL );
//
//		mFiberMap[unit.callId] = fiber;
//
//		// 切换至等待纤程
//		SwitchToFiber( getHandleFiber() );
//
//		ackCode = mAckCode;
//		
//		ConvertFiberToThread();
//
//		return ackCode;
//	}
//
//	static LPVOID getHandleFiber()
//	{
//		return mHandleFiber;
//	}
//
//private:
//	static void callResponseWork( void* param )
//	{
//		MemoryShareCallManager* pThis = static_cast< MemoryShareCallManager* >( param );
//
//		BOOL fSuccess; 
//
//		char buf[BUFSIZE];
//		MemoryShareCallResponse* ack;
//		MemoryShareCallUnit* unit;
//		DWORD cbBytesRead, cbReplyBytes, cbWritten; 
//
//		// 切换至纤程模式
//		mHandleFiber = ConvertThreadToFiber( NULL );
//
//		while( true )
//		{
//			// Read client requests from the pipe. 
//			fSuccess = ReadFile( 
//				mPipe,								// handle to pipe 
//				&buf,								// buffer to receive data 
//				BUFSIZE,	// size of buffer 
//				&cbBytesRead,						// number of bytes read 
//				NULL);								// not overlapped I/O 
//
//			if (! fSuccess || cbBytesRead == 0) 
//				break; 
//
//			if( buf[0] == 0 )
//			{
//				FiberMap::iterator it;
//
//				ack = (MemoryShareCallResponse*)&buf[1];
//				it = mFiberMap.find( ack->callId );
//				if( it == mFiberMap.end() )
//					continue;
//
//				LPVOID fiber = it->second;
//
//				mFiberMap.erase( it );
//
//				SwitchToFiber( fiber );
//			}
//			else
//			{
//				unit = (MemoryShareCallUnit*)&buf[1];
//
//				__asm
//				{
//					
//				}
//
//				MemoryShareCall theCall;
//				(mObj->*theCall)();
//			}
//
//		}
//
//		// 切换至线程模式
//		ConvertFiberToThread();
//	}
//
//private:
//	void postCall( const MemoryShareCallUnit& callUnit )
//	{
//		
//	}
//
//	static LPVOID mHandleFiber;
//};
//
//template< typename _Ty >
//typename MemoryShareCallManager<_Ty>::CallMap MemoryShareCallManager<_Ty>::mCallMap;
//
//template< typename _Ty >
//_Ty* MemoryShareCallManager<_Ty>::mObj = NULL;
//
//template< typename _Ty >
//HANDLE MemoryShareCallManager<_Ty>::mPipe;
//
//template< typename _Ty >
//int MemoryShareCallManager<_Ty>::mAckCode;
//
//template< typename _Ty >
//MemoryShareCallManager<_Ty>::FiberMap MemoryShareCallManager<_Ty>::mFiberMap;
//
//template< typename _Ty >
//LPVOID MemoryShareCallManager<_Ty>::mHandleFiber;
//
//template< typename _Ty >
//class MemorySharePool
//{
//public:
//	MemorySharePool( int size )
//	{
//		
//	}
//
//	virtual ~MemorySharePool()
//	{
//		
//	}
//};
//
//class MemoryShareManager
//{
//public:
//	enum Type
//	{
//		MS_PLAYER,
//		
//		MS_COUNT,
//	};
//
//	void init();
//	void check();
//	void shutdown();
//
//	static int MemoryShareTypeList[MS_COUNT];
//};
//
//int MemoryShareManager::MemoryShareTypeList[MemoryShareManager::MS_COUNT] = 
//	{
//		sizeof( stPlayerStruct ),
//	};
//
//void MemoryShareManager::init()
//{
//	
//}
//

void main()
{

}
