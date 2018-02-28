#pragma once

#include <list>
//#include <afxmt.h>
#include <winuser.h>

#define WM_DELEGATE	WM_USER + 110

struct DelegateArgs
{
	void* Sender;
	void* Param;
};

struct _Delegate
{
	template< typename _Handler, typename _Method >
	static void invoke( _Handler handler, _Method method, DelegateArgs* pArgs )
	{
		static void* pMethod;
		memcpy( &pMethod, &method, sizeof( _Method ) );
		SendMessage( handler->GetSafeHwnd(), WM_DELEGATE, (WPARAM)pMethod, (LPARAM)pArgs );
	}
};

template< typename _Ty >
class CDelegateHandler
{
public:
	typedef void (_Ty::*Type)( DelegateArgs* pArgs );

	void OnInvoke( void *pThis, WPARAM pMethod, LPARAM pArgs )
	{
		static Type method;
		memcpy( &method, &pMethod, sizeof( Type ) );

#ifndef _WIN64	
		__asm
		{
			mov eax, pArgs;
			push eax;
			mov ecx, pThis;
			mov eax, pMethod;
			call eax;
		}
#else
		( ( dynamic_cast< _Ty* >( this ) )->*method )( (DelegateArgs*)pArgs );
#endif

		//(((_Ty*)pThis)->*method)( (DelegateArgs*)pArgs );	
	}
};




