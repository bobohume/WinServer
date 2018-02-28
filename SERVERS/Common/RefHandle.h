#ifndef REF_HANDLE_H
#define REF_HANDLE_H

#include <winsock2.h>
#include <Windows.h>

#include "CppTypeInfo.h"

template< typename _Ty >
class RefObject : public CTI::ClassType< RefObject<_Ty> >
{
public:
	RefObject() {mRefCount = 0;}
	virtual ~RefObject() {}

	void addRef()
	{
#ifdef _WIN64
		InterlockedIncrement64( &mRefCount );
#else
		InterlockedIncrement( &mRefCount );
#endif
	}

	void releaseRef()
	{
#ifdef _WIN64
		InterlockedDecrement64( &mRefCount );
#else
		InterlockedDecrement( &mRefCount );
#endif

		if( mRefCount == 0 )
		{
			_Ty* obj = dynamic_cast<_Ty*>( this );
			MEMPOOL->FreeObj( obj );
		}
	}
	
private:
#ifdef _WIN64
	volatile LONGLONG mRefCount;
#else
	volatile LONG mRefCount;
#endif
};

template< typename _Ty >
class RefHandle
{
public:
	RefHandle()
	{
		pData = NULL;
	}

	RefHandle( _Ty* data )
	{
		_Ty* oldData;
		do 
		{
			oldData = (_Ty*)pData;
#ifdef _WIN64
		} while (::InterlockedCompareExchange64( (volatile LONGLONG* )&pData, (LONGLONG)data, (LONGLONG)oldData )!= (LONGLONG)oldData);
#else
		} while (::InterlockedCompareExchange( (volatile LONG* )&pData, (LONG)data, (LONG)oldData )!= (LONG)oldData);
#endif
		
		if( pData )
			((_Ty*)pData)->addRef();
	}

	RefHandle( const _Ty* data )
	{
		_Ty* oldData;
		do 
		{
			oldData = (_Ty*)pData;
#ifdef _WIN64
		} while (::InterlockedCompareExchange64( (volatile LONGLONG* )&pData, (LONGLONG)data, (LONGLONG)oldData )!= (LONGLONG)oldData);
#else
		} while (::InterlockedCompareExchange( (volatile LONG* )&pData, (LONG)data, (LONG)oldData )!= (LONG)oldData);
#endif

		if( pData )
			((_Ty*)pData)->addRef();
	}

	RefHandle( const RefHandle<_Ty>& other )
	{
		//pData = other.pData;
		_Ty* oldData;
		do 
		{
			oldData = (_Ty*)pData;
#ifdef _WIN64
		} while (::InterlockedCompareExchange64( (volatile LONGLONG* )&pData, (LONGLONG)other.pData, (LONGLONG)oldData )!= (LONGLONG)oldData);
#else
		} while (::InterlockedCompareExchange( (volatile LONG* )&pData, (LONG)other.pData, (LONG)oldData )!= (LONG)oldData);
#endif
		if( pData )
			((_Ty*)pData)->addRef();
	}

	~RefHandle()
	{
#if !defined(_UNITTEST)&&!defined(_NTJ_UNITTEST)

		if( pData )
			((_Ty*)pData)->releaseRef();
#endif
	}

	_Ty* operator ->()
	{
		assert( pData != NULL );
		return (_Ty*)pData;
	}

	//operator _Ty* ()
	//{
	//	assert( pData != NULL );
	//	return pData;
	//}

	_Ty* getData()
	{
		return (_Ty*)pData;
	}

	operator bool ()
	{
		return pData != NULL;
	}

	bool operator == ( const _Ty* data )
	{
		return pData == (LONG)data;
	}

	RefHandle<_Ty>& operator = ( const _Ty* data )
	{
		if( pData )
			((_Ty*)pData)->releaseRef();

		//pData = (_Ty*)data;
		_Ty* oldData;
		do 
		{
			oldData = (_Ty*)pData;
#ifdef _WIN64
		} while (::InterlockedCompareExchange64( (volatile LONGLONG* )&pData, (LONGLONG)data, (LONGLONG)oldData )!= (LONGLONG)oldData);
#else
		} while (::InterlockedCompareExchange( (volatile LONG* )&pData, (LONG)data, (LONG)oldData )!= (LONG)oldData);
#endif
		if( pData )
			((_Ty*)pData)->addRef();
		return *this;
	}

	RefHandle<_Ty>& operator = ( const RefHandle<_Ty>& other )
	{
		if( pData )
			((_Ty*)pData)->releaseRef();

		//pData = other.pData;
		_Ty* oldData;
		do 
		{
			oldData = (_Ty*)pData;
#ifdef _WIN64
		} while (::InterlockedCompareExchange64( (volatile LONGLONG* )&pData, (LONGLONG)other.pData, (LONGLONG)oldData )!= (LONGLONG)oldData);
#else
		} while (::InterlockedCompareExchange( (volatile LONG* )&pData, (LONG)other.pData, (LONG)oldData )!= (LONG)oldData);
#endif
		if( pData )
			((_Ty*)pData)->addRef();
		return *this;
	}

	bool isNull()
	{
		return pData == NULL;
	}

private:
	volatile LONG pData;
};

#endif