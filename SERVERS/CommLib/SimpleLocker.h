#ifndef _SIMPLELOCKER_H_
#define _SIMPLELOCKER_H_

#include <winsock2.h>

//¶àÏß³ÌËø
class SimpleLock
{
public :
	inline SimpleLock(void)    { InitializeCriticalSection(&m_Lock); } ;
	inline ~SimpleLock(void)   { DeleteCriticalSection(&m_Lock); } ;

	inline void	Lock()         { EnterCriticalSection(&m_Lock); } ;
	inline void	Unlock()       { LeaveCriticalSection(&m_Lock); } ;
protected:
    CRITICAL_SECTION m_Lock;
};

class AutoLock
{
public:
    inline AutoLock(SimpleLock& rLock): m_lock(rLock)
	{
		m_lock.Lock();
	}

	inline ~AutoLock(void)
	{
		m_lock.Unlock();
	}

protected:
	AutoLock(void);
	SimpleLock& m_lock;
};

#endif /*_SIMPLELOCKER_H_*/