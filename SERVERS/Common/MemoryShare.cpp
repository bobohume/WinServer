#include "MemoryShare.h"
#include <process.h>
#include "base/memPool.h"

CMemoryShare::CMemoryShare( LPCTSTR lpLocalName, LPCTSTR lpRemoteName, bool bIsServer, LPCTSTR lpDataName, int nDataBufferSize )
{
	lstrcpy( this->lpRemoteName, lpRemoteName );
	this->m_bIsServer = bIsServer;
	m_nBufferSize = nDataBufferSize;

	SetErrorCode( MEMORY_SHARE_ERROR_NONE );

	m_hLocalHandle = CreateEvent( NULL, TRUE, FALSE, lpLocalName );
	CheckError( m_hLocalHandle, MEMORY_SHARE_ERROR_INVALID_HANDLE );

	// 打开共享内存
	m_hMemoryFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, lpDataName );

	// 如果存在就创建一块共享内存
	if( m_hMemoryFile == NULL )
		m_hMemoryFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, nDataBufferSize * 2, lpDataName ); // 创建双倍大小，以防万一
	CheckError( m_hMemoryFile, MEMORY_SHARE_ERROR_FILE );

	m_lpBuffer = (LPVOID)MapViewOfFile( m_hMemoryFile, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	CheckError( m_lpBuffer, MEMORY_SHARE_ERROR_MEMORY );

	m_hRemoteHandle = NULL;

	SetErrorCode( MEMORY_SHARE_ERROR_INVALID_REMOTE );
	//CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)WaitRemoteThread, this, 0, NULL );

	m_hThread = (HANDLE)_beginthread( WaitRemoteThread, NULL, this );
	m_hSendThread = NULL;
}

CMemoryShare::~CMemoryShare()
{
	Cleanup();
}

void CMemoryShare::Cleanup()
{
	TerminateThread( m_hThread, 0 );
	TerminateThread( m_hSendThread, 0 );

	UnmapViewOfFile( m_lpBuffer );
	CloseHandle( m_hMemoryFile );
}

char* CMemoryShare::GetBuffer()
{
	return (char*)m_lpBuffer;
}


int CMemoryShare::GetLastErrorCode()
{
	return m_nLastError;
}

void CMemoryShare::SetErrorCode( int nErrorCode )
{
	m_nLastError = MEMORY_SHARE_ERROR_NONE;
}

void CMemoryShare::NotiyRemote()
{
	if( GetLastErrorCode() == MEMORY_SHARE_ERROR_NONE )
		SetEvent( m_hRemoteHandle );
}

void CMemoryShare::WaitRemoteThread( LPVOID lpParam )
{
	CMemoryShare* pThis = (CMemoryShare*)lpParam;

	while( pThis->m_hRemoteHandle == NULL )
	{
		pThis->m_hRemoteHandle = OpenEvent( EVENT_ALL_ACCESS, FALSE, pThis->lpRemoteName );
		Sleep( 1000 ); // 一休哥。。。休息，休息一会儿。。。
	}

	pThis->SetErrorCode( MEMORY_SHARE_ERROR_NONE );

	// 如果是服务器主动向客户端通知
	if( pThis->m_bIsServer )
		pThis->NotiyRemote();
	else
		//CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)SendThread, pThis, 0, NULL );
		pThis->m_hSendThread = (HANDLE)_beginthread( SendThread, NULL, pThis );

}

void CMemoryShare::Read( char* lpBuffer, int* pSize /* = MEMORY_SHARE_MAX_DATABUFFER_SIZE */ )
{
	if( GetLastErrorCode() != MEMORY_SHARE_ERROR_NONE )
		return ;

	unsigned short wSize;
	memcpy( &wSize, GetBuffer(), 2 );
	memcpy( lpBuffer, GetBuffer() + 2, wSize );
	
	if( pSize )
		*pSize = wSize;
}

void CMemoryShare::Send( char* lpBuffer, int nSize )
{
	CLocker locker( m_cs );

	stSendItem si;
	si.nSize = nSize;
	char* pBuffer = (char*)CMemPool::GetInstance()->Alloc( nSize );
	memcpy( pBuffer, lpBuffer, nSize );
	si.pBuf = pBuffer;

	m_sendList.push_back( si );
}

void CMemoryShare::SendThread( LPVOID lpParam )
{
	CMemoryShare* pThis = (CMemoryShare*)lpParam;
	
	stSendItem si;

	CLocker* pLocker;
	while( 1 )
	{
		pLocker = new CLocker( pThis->m_cs );
		if( pThis->m_sendList.size() > 0 )
		{
			si = pThis->m_sendList.front();
			pThis->m_sendList.pop_front();	
			delete pLocker;
		}
		else
		{
			delete pLocker;
			Sleep( 100 );
			continue;
		}


		WaitForSingleObject( pThis->GetLocalEventHandle(), INFINITE );

		pThis->Write( si.pBuf, si.nSize );
		CMemPool::GetInstance()->Free( (MemPoolEntry)si.pBuf );
		
		ResetEvent( pThis->GetLocalEventHandle() );

		pThis->NotiyRemote();

		
	}
}

void CMemoryShare::Write( char* lpBuffer, int nSize )
{
	if( GetLastErrorCode() != MEMORY_SHARE_ERROR_NONE )
		return ;

	if( nSize > m_nBufferSize - 2 )
		nSize = m_nBufferSize - 2;

	unsigned short wSize = (unsigned short)nSize;
	memcpy( GetBuffer(), &wSize, 2 );
	memcpy( GetBuffer() + 2, lpBuffer, nSize );
}

HANDLE CMemoryShare::GetLocalEventHandle()
{
	return m_hLocalHandle;
}