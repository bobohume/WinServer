#ifndef MEMORY_SHARE_H
#define MEMORY_SHARE_H

#include <WinSock2.h>
#include <windows.h>
#include "base/Locker.h"
#include <deque>

enum eMemoryShareError
{								 
	MEMORY_SHARE_ERROR_NONE = 0,		// ³É¹¦

	MEMORY_SHARE_ERROR_INVALID_EVENT,
	MEMORY_SHARE_ERROR_INVALID_HANDLE,
	MEMORY_SHARE_ERROR_INVALID_REMOTE,
	MEMORY_SHARE_ERROR_FILE,
	MEMORY_SHARE_ERROR_MEMORY,

	MEMORY_SHARE_ERROR_UNKNOWN,
};

#define MEMORY_SHARE_MAX_DATABUFFER_SIZE 1024

#define CheckError(p,err) {if( p == NULL ) {SetErrorCode(err);Cleanup();return;}}

class CMemoryShare
{
public:
	CMemoryShare( LPCTSTR lpLocalName, LPCTSTR lpRemoteName, bool bIsServer = FALSE, LPCTSTR lpDataName = TEXT("ShareMemory.dat"), int nDataBufferSize = MEMORY_SHARE_MAX_DATABUFFER_SIZE );
	virtual ~CMemoryShare();

	int GetLastErrorCode();

public:
	char*	GetBuffer();
	void	SetErrorCode( int nErrorCode );
	void	Send( char* lpBuffer, int nSize );
	void	Read( char* lpBuffer, int* pSize = NULL );
	void	Write( char* lpBuffer, int nSize );
	void	NotiyRemote();
	HANDLE	GetLocalEventHandle();
private:
	static void WaitRemoteThread( LPVOID lpParam );
	static void SendThread( LPVOID lpParam );
	void Cleanup();

	HANDLE	m_hRemoteHandle;
	HANDLE	m_hLocalHandle;
	HANDLE	m_hThread;
	HANDLE	m_hSendThread;
	HANDLE	m_hMemoryFile;
	int		m_nLastError;
	void*	m_lpBuffer;
	TCHAR	lpRemoteName[64];
	bool	m_bIsServer;
	CMyCriticalSection m_cs;

	int		m_nBufferSize;

	struct stSendItem
	{
		char* pBuf;
		int nSize;
	};

	std::deque<stSendItem> m_sendList;
};

#endif

