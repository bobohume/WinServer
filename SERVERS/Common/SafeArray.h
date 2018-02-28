#ifndef SAFE_ARRAY_H
#define SAFE_ARRAY_H

#include <winsock2.h>
#include <windows.h>

#include <assert.h>

template< _Ty >
class SafeArray
{
public:
	SafeArray( int size ) :
	  mSize( size ), mPageMemory( NULL ), mArrayPointer( NULL )
	{
		
	}

	~SafeArray()
	{
		
	}

	_Ty& operator [] ( int index )
	{
		SERVER_ASSERT( mArrayPointer != NULL, "SafeArray : used before allocation" );

		SERVER_ASSERT( index >= 0 && index < mSize, "SafeArray : out of range !" );

		return ((_Ty*)mArrayPointer)[index];
	}

	operator _Ty* ()
	{
		SERVER_ASSERT( mArrayPointer != NULL, "SafeArray : used before allocation" );

		return mArrayPointer;
	}

private:
	// depends on the specific os
	static const int PageSize = 4096;

	void* mPageMemory;
	void* mArrayPointer;

	int mSize;
};

#endif