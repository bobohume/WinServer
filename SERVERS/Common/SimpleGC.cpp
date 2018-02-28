#include "SimpleGC.h"
#include <windows.h>

CSimpleGC*			CSimpleGC::m_pInstance	= NULL;
CSimpleGC::FnFree	GlobalFree				= NULL;


CSimpleGC::Ptr::Ptr()
{
	SetPtr( NULL );
}

CSimpleGC::Ptr::Ptr( Ptr& data )
{
	SetPtr( data.m_ptr );
}

void CSimpleGC::Ptr::Release()
{
	if( m_ptr )
	{
		free( m_ptr );
		CSimpleGC::GetInstance()->RemoveMemory( m_ptr );
		m_ptr = NULL;
	}
}

CSimpleGC::Ptr::~Ptr()
{
	CSimpleGC::GetInstance()->RemoveData( this );
}

void CSimpleGC::Ptr::operator = ( CSimpleGC::Ptr& data )
{
	this->m_ptr = data.m_ptr;
}

void CSimpleGC::Ptr::SetPtr( void* pMemory )
{
	this->m_ptr = pMemory;

	CSimpleGC::GetInstance()->AddData( this );
}


void CSimpleGC::AddData( Ptr* pData )
{
	CLocker locker(m_cs);

	m_DataSet.insert( pData );
	m_pMemorySet->insert( pData->m_ptr );
}

void CSimpleGC::RemoveData( Ptr* pData )
{
	CLocker locker(m_cs);

	stdext::hash_set<Ptr*>::iterator it;

	it = m_DataSet.find( pData );
	if( it != m_DataSet.end() )
		m_DataSet.erase( it );
}

void CSimpleGC::RemoveMemory( void* pMemory )
{
	CLocker locker(m_cs);

	stdext::hash_set<void*>::iterator it;

	it = m_pMemorySet->find( pMemory );
	if( it != m_pMemorySet->end() )
	{
		free( *it );
		m_pMemorySet->erase( it );
	}
}

void CSimpleGC::AddObject( Object* pObject )
{
	CLocker locker(m_cs);

	m_ObjectSet.insert( pObject );
}

// void CSimpleGC::RemoveObject( Object* pObject )
// {
// 	CLocker locker(m_cs);
// 
// 	stdext::hash_set<Object*>::iterator it;
// 
// 	it = m_ObjectSet.find( pObject );
// 	if( it != m_ObjectSet.end() )
// 		m_ObjectSet.erase( it );
// 
// }


// 垃圾回收过程（标记清理法）
int CSimpleGC::GC()
{
	CLocker locker(m_cs);

	stdext::hash_set<void*>::iterator vit;
	stdext::hash_set<void*>::iterator vtit;
	stdext::hash_set<Ptr*>::iterator dit;
	stdext::hash_set<Object*>::iterator oit;
	stdext::hash_map<void*,char> temp_map;
	stdext::hash_set<void*>* temp_set = new stdext::hash_set<void*>();

	// 清空标记
	for( vit = m_pMemorySet->begin(); vit != m_pMemorySet->end(); vit++ )
	{
		temp_map[*vit] = NULL;
	}

	// 遍历内存并产生标记
	for( dit = m_DataSet.begin(); dit != m_DataSet.end(); dit++ )
	{

		vit = m_pMemorySet->find( (*dit)->m_ptr );
		if( vit != m_pMemorySet->end() )
			temp_map[*vit] = 1;
	}

	int nCount = 0;

	// 将未标记的内存清除
	for( vit = m_pMemorySet->begin(); vit != m_pMemorySet->end(); vit++ )
	{
		if( temp_map[*vit] == NULL)
		{
			if( *vit != NULL )
			{
				oit = m_ObjectSet.find( (Object*)*vit );
				if( oit != m_ObjectSet.end() )
				{
					(*oit)->~Object();
					m_ObjectSet.erase( oit );
				}

				nCount++;		
				free( *vit );
				const_cast<void *>(*vit) = NULL;
			}
		}
		else
		{
			temp_set->insert( *vit );
		}
	}

	if( nCount > 0 )
	{
		std::stringstream str;
		str<<"系统回收"<<nCount<<"个垃圾对象"<<std::endl;
		OutputDebugStringA( str.str().c_str() ); 
	}
	return nCount;
}

// 获取CSimpleGC实例
// 这里使用Singleton模式，保证全局唯一
CSimpleGC* CSimpleGC::GetInstance()
{
	static CSimpleGC instance;

	if( !m_pInstance )
		m_pInstance = &instance;

	return m_pInstance;
}

CSimpleGC::~CSimpleGC()
{
	GC();

	delete m_pMemorySet;
}

CSimpleGC::CSimpleGC()
{
	if( free == NULL )
	{
		free = ::free;
		OutputDebugString( TEXT("SimpleGC警告: 没有发现内存释放器，使用默认\n") );
	}

	m_pMemorySet = new stdext::hash_set<void*>();
}