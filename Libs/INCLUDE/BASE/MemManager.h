#ifndef MEM_MANAGER_H
#define MEM_MANAGER_H

#pragma once

#include <hash_map>
#include <exception>
#include <stack>
#include <hash_set>

#include "locker.h"

class CMemManager
{
	//class IMemPool
	//{
	//public:
	//	virtual void* alloc() = 0;
	//	virtual void free( void* ptr ) = 0;
	//	virtual ~IMemPool(){};
	//};

	class MemPoolHelper
	{
	public:
		struct _block
		{
		public:
			_block( size_t blockSize, size_t blockCount );

			virtual ~_block();

			char* alloc();

			void free( void* ptr );

			typedef std::stack< char* > FreeList;
			FreeList mFreeList;

			typedef stdext::hash_set< char* > UsedSet;
			UsedSet mUsedSet;

			size_t mFreeCount;
			size_t mBlockSize;
			size_t mBlockCount;
			char* mMemBlock;
		};

		struct _block_header
		{
			_block* pBlock;
			char cppFile[100];
			int lineId;
		};
	public:
		MemPoolHelper( size_t blockSize, size_t blockCount );

		virtual ~MemPoolHelper();

		_block* getNewBlock();

#ifdef _DEBUG
		void* alloc(const char* cppFile, int lineId);
#else
		void* alloc();
#endif

		void free( void* ptr );

	private:
		typedef std::list< _block* > BlockList;
		BlockList mBlockList;
		int mSizeCount;

		size_t mBlockCount;
		size_t mBlockSize;

		_block* mCurBlock;
	};

	class CMemPool : /*public IMemPool, */private MemPoolHelper
	{
	public:
		CMemPool(size_t blockSize) : MemPoolHelper( blockSize, BlockCount ) {}

#ifdef _DEBUG
		void* alloc( const char* cppFile, int lineId );
#else
		void* alloc();
#endif

		void free( void* ptr );
	};

	CMemPool* getMemPool( size_t size );

public:
	size_t getSize( size_t size );

	//template< typename _Ty >
	//void alloc( _Ty*& objPtr )
	//{
	//	CLocker lock( m_cs );

	//	objPtr = static_cast< _Ty*>( getMemPool( getSize(sizeof( _Ty )) )->alloc() );
	//}

#ifdef _DEBUG
	void* _alloc( size_t size, const char* cppFile, int lineId );
#else
	void* _alloc( size_t size );
#endif

	//template< typename _Ty >
	//void free( _Ty*& objPtr )
	//{
	//	CLocker lock( m_cs );

	//	getMemPool( getSize(sizeof( _Ty )))->free( objPtr );
	//	objPtr = NULL;
	//}

	void _free( void* ptr );

	~CMemManager();

	static CMemManager* getInstance();
	
	static CMemManager instance;

private:
	typedef stdext::hash_map< size_t, CMemPool* > MemPoolMap;
	MemPoolMap mMemPoolMap;

	CMyCriticalSection m_cs;
	const static int BlockCount = 256;
};

#endif /*MEM_MANAGER_H*/
