#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#include "Base/Types.h"
#include "TCMalloc/TCMallocInclude.h"
#include "MemManager.h"

#define _1M		1048576
#define _1K		1024
#define _1B		1


#ifdef _DEBUG
#define MEMPOOL_ALLOC(p)  malloc(p)  //CMemManager::getInstance()->_alloc(p, __FUNCTION__, __LINE__)
#else
#define MEMPOOL_ALLOC(p) malloc(p)  //CMemManager::getInstance()->_alloc(p)
#endif

#define MEMPOOL_FREE free//CMemManager::getInstance()->_free

//#define MEMPOOL_ALLOC malloc
//#define MEMPOOL_FREE free

typedef U8* MemPoolEntry;

class CMemPool
{
	CMemPool();

	static CMemPool *m_pInstance;

public:
	~CMemPool();

	static CMemPool *GetInstance()
	{
		if(!m_pInstance)
			m_pInstance = new CMemPool;
			
		return m_pInstance;
	}
	static void shutdown()
	{
		if(m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}

	MemPoolEntry Alloc(U32 Size);

	void Free(MemPoolEntry);

	template <class T>
	__inline void AllocObj(T *&pObj)
	{
		pObj = (T *)Alloc(sizeof(T));
		//CMemManager::getInstance()->alloc( pObj );
		constructInPlace<T>(pObj);
	}

	template <class T>
	__inline void FreeObj(T *&pObj)
	{
		destructInPlace<T>(pObj);
		//CMemManager::getInstance()->free( pObj );
		Free((MemPoolEntry)pObj);
	}
};

__inline MemPoolEntry CMemPool::Alloc(U32 Size)
{
	void* p = MEMPOOL_ALLOC(Size);
	return (MemPoolEntry)p;
}

__inline void CMemPool::Free(MemPoolEntry p)
{
	MEMPOOL_FREE(p);
	p = NULL;
}

#ifdef TCP_SERVER
//extern void* ::operator new( size_t size );
//extern void ::operator delete( void* ptr );
//
//extern void* ::operator new[]( size_t size );
//extern void ::operator delete[]( void* ptr );
#endif

#endif