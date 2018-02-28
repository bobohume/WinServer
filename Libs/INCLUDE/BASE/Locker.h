#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <crtdbg.h>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>

class CLockableObject
{
public:
	CLockableObject(void) {}
	virtual ~CLockableObject(void) {}
	virtual void Lock() = 0;
	virtual void Unlock() = 0;
};

#define LOCK_SPIN_COUNT 4000

class CMyCriticalSection : public CLockableObject
{
public:
	CMyCriticalSection();

	virtual ~CMyCriticalSection();

	virtual void Lock();

	virtual void Unlock();

	bool IsLocked();

	//__inline virtual bool TryLock()		{ return TryEnterCriticalSection(&m_CritSect) != 0; };
private:
	CRITICAL_SECTION m_CritSect;
	bool mIsLocked;
};

class CLocker
{
public:
	CLocker( void* key, bool bInitialLock = true);

	CLocker(CLockableObject* pLockable, bool bInitialLock = true);

	CLocker(CLockableObject& Lockable, bool bInitialLock = true);

	~CLocker()							{ if(m_bIsLocked) { m_pLockable->Unlock(); m_bIsLocked = false;}; };
	__inline void Lock()				{ if(!m_bIsLocked) { m_pLockable->Lock(); m_bIsLocked = true;}; };
	__inline void Unlock()				{ if(m_bIsLocked) { m_pLockable->Unlock(); m_bIsLocked = false;}; };

private:
	CLockableObject* m_pLockable;
	bool	m_bIsLocked;
};

extern const CLockableObject* GetGlobabLocker(void * key);
#define OLD_DO_LOCK(p) CLocker _locke(p);
//
//#ifdef NTJ_SERVER
//#undef OLD_DO_LOCK
//#define OLD_DO_LOCK(p) __noop(p);
//#endif
//
//#ifdef NTJ_CLIENT
//#undef OLD_DO_LOCK
//#define OLD_DO_LOCK(p) __noop(p);
//#endif
//
//#ifdef NTJ_EDITOR
//#undef OLD_DO_LOCK
//#define OLD_DO_LOCK(p) __noop(p);
//#endif

#endif
