#ifndef __LOCK_H__
#define __LOCK_H__

#ifndef __DEFINE_H__
#include "Define.h"
#endif

#ifndef __LOCKER_H__
#include "base/Locker.h"
#endif

class MyLock
{
	CRITICAL_SECTION m_Lock ;
public :
	MyLock() { InitializeCriticalSection(&m_Lock); } ;
	~MyLock() { DeleteCriticalSection(&m_Lock); } ;
	void Lock() { EnterCriticalSection(&m_Lock); } ;
	void Unlock() { LeaveCriticalSection(&m_Lock); } ;
};

#endif//__LOCK_H__