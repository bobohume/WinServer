#ifndef SAFE_HANDLE_H
#define SAFE_HANDLE_H

#include <windows.h>

template< typename _Ty >
class SafeHandle
{
public:
	SafeHandle( _Ty* objPtr )
	{
		mObjPtr = objPtr;
	}

	~SafeHandle()
	{
		if( mRefCount == 0 )
			delete mObjPtr;
	}

	SafeHandle<_Ty>& operator = ( SafeHandle<_Ty>& other )
	{
		mObjPtr = other.mObjPtr;

		return *this;
	}

	SafeHandle<_Ty>& operator = ( _Ty* objPtr )
	{
		mObjPtr = objPtr;
	}

	operator _Ty* ()
	{

	}

	void addRef()
	{
		InterlockedIncrement( &mRefCount );
	}

	void releaseRef()
	{
		InterlockedDecrement( &mRefCount );
	}
private:

	volatile LONG mRefCount;
	_Ty* mObjPtr;
};

template< typename _Ty >
class SafeRef
{
public:
	SafeRef( const SafeHandle<_Ty>& objHandle )
		: mHandle( objHandle )
	{
		mHandle.addRef();
	}

	~SafeRef()
	{
		mHandle.releaseRef();
	}

private:
	SafeHandle<_Ty>& mHandle;
};

#endif