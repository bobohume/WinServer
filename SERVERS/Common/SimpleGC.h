#ifndef SIMPLE_GC_H
#define SIMPLE_GC_H

#include "Base/Locker.h"

#include <hash_set>
#include <hash_map>
#include <sstream>

#define SET_MANAGED_VALUE_TYPE(p) \
	Ptr( p pMemory ) { SetPtr((void*)pMemory); } \
	inline void operator = ( p pMemory ) { SetPtr((void*)pMemory);} \
	inline operator p () { return (p)m_ptr; }

class CSimpleGC
{
public:
	typedef void (*FnFree)( void* pMemory );

	class Ptr
	{
	public:
		friend class CSimpleGC;
		friend class ObjPtr;

		Ptr();
		Ptr( Ptr& data );

		~Ptr();

		void operator = ( Ptr& data );

		SET_MANAGED_VALUE_TYPE( char* );
		SET_MANAGED_VALUE_TYPE( void* );
		SET_MANAGED_VALUE_TYPE( const char* );

		inline Ptr( int& value ) { m_ptr = NULL; }
		inline void operator = ( int value ) { SetPtr( NULL ); }

		virtual void Release(); // 释放Ptr所指向的内存

	private:

		void* operator new( size_t s ) {return malloc(s);}
		void SetPtr( void* pMemory );
		void* m_ptr;
	} ;

	class Object
	{
	public:
		virtual ~Object() {;}
	};

	// 使用ObjPtr的对象必须从CSimple::Object对象继承
	template <class T = Object>
	class ObjPtr : public Ptr
	{
	public:
		inline ObjPtr() { SetPtr(NULL);}
		inline ObjPtr( T* pObj ) { SetPtr( pObj ); CSimpleGC::GetInstance()->AddObject( pObj ); }

		inline operator T* () { return (T*)m_ptr; }
		inline operator T& () { return *((T*)m_ptr); }

		inline void operator = ( T* pObj ) { SetPtr( (void*)pObj ); CSimpleGC::GetInstance()->AddObject( pObj ); }

		~ObjPtr() { ; }

		void Release()
		{
			((T*)this)->~T();
			Ptr::Release();			
		}
	} ;

	template < class T >
	class GCObject : public Object
	{

	};

	static FnFree free;
	static CSimpleGC* GetInstance();

	int GC();
private:
	CSimpleGC();
	~CSimpleGC();

private:
	stdext::hash_set<Ptr*> m_DataSet;
	stdext::hash_set<void*>* m_pMemorySet;
	stdext::hash_set<Object*> m_ObjectSet;

	static CSimpleGC* m_pInstance;


	CMyCriticalSection m_cs;

	void AddData( Ptr* pData );
	void RemoveData( Ptr* pData );
	void RemoveMemory( void* pMemory );
	void AddObject( Object* pObject );
};

typedef CSimpleGC::Ptr		gc_ptr;
typedef CSimpleGC::Ptr		GCPtr;

#define gc_objptr	CSimpleGC::ObjPtr

#define GlobalGC	CSimpleGC::GetInstance()->GC
#define GlobalFree	CSimpleGC::free

#endif



