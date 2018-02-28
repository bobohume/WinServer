#include "BackWorker.h"

CBackWorker::CBackWorker( int nWorkThreadCount /*= 5 */, const char* name )
{
	if( nWorkThreadCount > MAX_WORK_THREAD_COUNT )
		nWorkThreadCount = MAX_WORK_THREAD_COUNT;

	mWorkHandle = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, NULL, nWorkThreadCount );

	mThreadCount = nWorkThreadCount;

	for( int i = 0; i < nWorkThreadCount; i++ )
	{
		mWorkThreadHandles[i] = (HANDLE)_beginthread( WorkThread, 0, this ); 
		//SetThreadName( GetThreadId( mWorkThreadHandles[i] ), name );
	}
}

void CBackWorker::SetThreadName( DWORD dwThreadID, LPCSTR szThreadName )
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException( 0x406D1388, 0,
			sizeof(info) / sizeof(DWORD),
			(DWORD*)&info );
	}
	__except( EXCEPTION_CONTINUE_EXECUTION ) {
	}
}

void CBackWorker::WorkThread( void* pThis )
{
	CBackWorker* pthis = ( CBackWorker* )pThis;
	stWorkItem* pWork;
	LPOVERLAPPED* pData = NULL;
	DWORD dwLength;

	while( 1 )
	{
		::GetQueuedCompletionStatus( pthis->mWorkHandle, &dwLength, ( PULONG_PTR )&pWork, (LPOVERLAPPED*)&pData, -1 );
		if( pWork->mFunction == NULL )
		{
			delete pWork;

			break;
		}
		pthis->_callMethod(pWork);
		delete[] pWork->mParamList;
		delete pWork;
	}
}

// now, we can use arguments as many as possible
// and we do not need the target method must be signed by _cdecl
void CBackWorker::_callMethod( stWorkItem* pWork )
{
	// for a better performance
	void *funct = pWork->mFunction;
	void *pthis = pWork->mThis;
	int nCount = pWork->mCount * 4;
	void *pESP;

	__asm
	{
		mov eax, nCount;
		sub esp, eax;
		mov pESP, esp;
	}

	if( pWork->mCount > 0)
		memcpy( pESP, pWork->mParamList, pWork->mCount * 4 );

	__asm
	{
		mov ecx, pthis;   // for [thiscall]
		mov eax, funct;
		call eax;
	} 	
}

void __cdecl CBackWorker::postWork( WorkFunction funct, void* pThis, int argc, ... )
{
	stWorkItem* pWork = new stWorkItem();

	pWork->mFunction = funct;
	pWork->mThis = pThis;
	pWork->mCount = argc;


	// for a batter performance 
	if( argc > 0 )
	{	
		pWork->mParamList = new int[argc];
		int* pSrc = (int*)&argc;
		pSrc ++;
		memcpy( pWork->mParamList, pSrc, argc * 4);
	}

	::PostQueuedCompletionStatus( mWorkHandle, 0, (ULONG_PTR)pWork, (LPOVERLAPPED)pWork );
}

CBackWorker::~CBackWorker()
{
	for( int i = 0; i < mThreadCount; i++ )
	{
		stWorkItem* pWork = new stWorkItem();

		pWork->mFunction = NULL;

		::PostQueuedCompletionStatus( mWorkHandle, 0, (ULONG_PTR)pWork, (LPOVERLAPPED)pWork );
	}

	WaitForMultipleObjects( mThreadCount, mWorkThreadHandles, TRUE, -1 );
}