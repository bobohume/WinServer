#ifndef GDI_UTIL_H
#define GDI_UTIL_H

#include <WinSock2.h>
#include <Windows.h>
#include <comdef.h>
#include <GdiPlus.h>

#pragma comment( lib, "GdiPlus.lib" )

#ifdef NTJ_CLIENT

#endif

struct IDirectDrawSurface7;

//---------------------------------------------------------------
// GDI ∏®÷˙¿‡
//---------------------------------------------------------------
class GdiUtil
{
public:
	struct Helper
	{
		Helper();
		virtual ~Helper();

		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;

	};

	enum
	{
		PixelSize = 1
	};

	static Helper sHelper;

	static char* genImage( char* buf, int width, int height );
	static void loadImage( IDirectDrawSurface7* pSurface, char* buf, int length, int width, int height );
};

#endif