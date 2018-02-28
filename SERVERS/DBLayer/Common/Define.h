#ifndef __DEFINE_H__
#define __DEFINE_H__

/*#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif*/

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>

#ifndef DEBUG
#define __ENTER_FUNCTION 
#define __LEAVE_FUNCTION
#else
#define __ENTER_FUNCTION {try{
#define __LEAVE_FUNCTION }catch(...){AssertSpecial(FALSE,__FUNCTION__);}}
#endif

#undef  SAFE_DELETE
#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;

#undef  SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(a) if( (a) != NULL ) delete [] (a); (a) = NULL;

#undef  SAFE_FREE
#define SAFE_FREE(a)   if( (a) != NULL ) dFree (a); (a) = NULL;

template< typename T >
struct TempAlloc
{
	T* ptr;
	unsigned long size;

	TempAlloc( unsigned long size )
		: size( size )
	{
		ptr = ( T* ) malloc( size * sizeof( T ) );
		memset(ptr, 0, size * sizeof(T));
	}
	~TempAlloc()
	{
		if( ptr )
			free( ptr );
	}
	operator T*()
	{
		return ptr;
	}
};

#define __SP__ "\t"

#define DBECATCH() 	catch(ExceptionResult &e) \
{\
	g_Log.WriteWarn("<<WARN>>code=%d, msg=%s, func='%s'", e.code, e.str(), __FUNCTION__);\
	err = e.code;\
}\
	catch (CDBException &e)\
{\
	g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);\
	err = DBERR_DB_INTERNALERR;\
}\
	catch(...)\
{\
	g_Log.WriteFocus("<<ERROR>>未知的错误, func='%s'", __FUNCTION__);\
	err = DBERR_UNKNOWERR;\
}


#define DBCATCH(key) 	catch(ExceptionResult &e) \
{\
	g_Log.WriteWarn("<<WARN>>code=%d, msg=%s, key=%d, func='%s'", e.code, e.str(), key, __FUNCTION__);\
	err = e.code;\
}\
catch (CDBException &e)\
{\
	g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, key=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, key, __FUNCTION__);\
	err = DBERR_DB_INTERNALERR;\
}\
catch(...)\
{\
	g_Log.WriteFocus("<<ERROR>>未知的错误, key=%d, func='%s'", key, __FUNCTION__);\
	err = DBERR_UNKNOWERR;\
}

#define DBSCATCH(szKey) 	catch(ExceptionResult &e) \
{\
	g_Log.WriteWarn("<<WARN>>code=%d, msg=%s, key=%s, func='%s'", e.code, e.str(), szKey, __FUNCTION__);\
	err = e.code;\
}\
	catch (CDBException &e)\
{\
	g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, key=%s, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, szKey, __FUNCTION__);\
	err = DBERR_DB_INTERNALERR;\
}\
	catch(...)\
{\
	g_Log.WriteFocus("<<ERROR>>未知的错误, key=%s, func='%s'", szKey, __FUNCTION__);\
	err = DBERR_UNKNOWERR;\
}

#endif//__DEFINE_H__