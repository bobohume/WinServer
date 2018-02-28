#ifndef LOCK_REF_H
#define LOCK_REF_H

#include "base/Locker.h"

struct CLockRefBase
{
	CMyCriticalSection m_cs;
};

template< typename _Ty >
struct CLockRef
{
	CLockRef( _Ty* ref )
	{
		mRef = ref;
		mRef->m_cs.Lock();
	}

	~CLockRef()
	{
		mRef->m_cs.Unlock();
	}

	_Ty* operator -> ()
	{
		return mRef;
	}

private:
	_Ty* mRef;
};


#endif