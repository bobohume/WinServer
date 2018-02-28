#ifndef BACK_WORKER_H
#define BACK_WORKER_H

#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <stdarg.h>
#include <process.h>

class CBackWorker
{
public:
	typedef void* WorkFunction;

	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType;     // must be 0x1000
		LPCSTR szName;    // pointer to name (in user address space)
		DWORD dwThreadID; // thread ID (-1 = caller thread)
		DWORD dwFlags;    // reserved for future use, must be zero
	} THREADNAME_INFO;


	CBackWorker( int nWorkThreadCount = 5, const char* name = "BackWorker" );
	virtual ~CBackWorker();

	void __cdecl postWork( WorkFunction funct, void* pThis, int argc = 0, ... );
	void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName );

private:
	struct stWorkItem
	{
		WorkFunction		mFunction;
		void*				mThis;
		int					mCount;
		int*				mParamList;
	};

	static const int MAX_WORK_THREAD_COUNT = 16;

private:

	HANDLE mWorkHandle;
	HANDLE mWorkThreadHandles[MAX_WORK_THREAD_COUNT];
	int mThreadCount;


private:
	static void WorkThread( void* pThis );

	void _callMethod( stWorkItem* pWork );
};

template<class To,class From>
union u
{
	From f;
	To   t;
	u():t(To()){}
};

template <class To,class From>
To union_cast(const From x)
{
	u<To,From> d;
	d.f=x;
	return d.t;
}

#define WorkMethod(c) (union_cast<CBackWorker::WorkFunction>( &c ))

#endif