#include "stdafx.h"
#include "lock.h"

CLockerEx::CLockerEx( CLockable& cs )
	: mLockable( cs )
{
	mLockable.lock();
}

CLockerEx::~CLockerEx()
{
	mLockable.unlock();
}

CLockable::CLockable()
{
	::InitializeCriticalSectionAndSpinCount( &mCS, 4000 );
}

CLockable::~CLockable()
{
	::DeleteCriticalSection( &mCS );
}

void CLockable::lock()
{
	::EnterCriticalSection( &mCS );
}

void CLockable::unlock()
{
	::LeaveCriticalSection( &mCS );
}