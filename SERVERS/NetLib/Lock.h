#ifndef LOCK_H
#define LOCK_H

class NETLIB_CLASS CLockerEx;

class NETLIB_CLASS CLockable
{
public:
	CLockable();
	virtual ~CLockable();

	void lock();
	void unlock();

private:
	CRITICAL_SECTION mCS;
};

class NETLIB_CLASS CLockerEx
{
public:
	CLockerEx( CLockable& cs );
	virtual ~CLockerEx();

private:
	CLockable& mLockable;
};


#endif