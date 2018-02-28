#include "GdiUtil.h"
#define _USE_MATH_DEFINES

#include <cmath>

GdiUtil::Helper::Helper()
{
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

GdiUtil::Helper::~Helper()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

#ifndef NTJ_ROBOT
GdiUtil::Helper GdiUtil::sHelper;
#endif

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

char* GdiUtil::genImage( char* buf, int width, int height )
{
	if( !buf )
		return NULL;

	const int fontSize = 12;
	const int fontPadding = 2;
	Gdiplus::Color noiseColor( 0xFF878787 );
	Gdiplus::Color backgroundColor( 0xFF2c2c2c );
	Gdiplus::Color fontColor( 255, rand()%255, rand()%255, rand()%255 );//0xFF878787

	Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap( width, height, PixelFormat24bppRGB);
	Gdiplus::Graphics* pPaint = Gdiplus::Graphics::FromImage( bmp );
	Gdiplus::Font* pFont = new Gdiplus::Font( L"zill spills", fontSize, Gdiplus::FontStyleBold );

	if( !bmp || !pPaint || !pFont )
		return NULL;

	Gdiplus::StringFormat sf;
	sf.SetLineAlignment( Gdiplus::StringAlignmentCenter );
	sf.SetAlignment( Gdiplus::StringAlignmentCenter );

	Gdiplus::SolidBrush* brush = new Gdiplus::SolidBrush( fontColor );

	pPaint->Clear( backgroundColor );

#if 1
	//增加噪点
	//Gdiplus::Pen *pPen = new Gdiplus::Pen( Gdiplus::Color( 0xFF676767 ), 0 );
	Gdiplus::Bitmap singlePixel( 1, 1, PixelFormat24bppRGB );
	singlePixel.SetPixel( 0, 0, noiseColor );
	int numNoise = width * height * 0.1;				// 控制噪点数量
	for (int i=0; i<numNoise; i++)
	{
		int x = rand() % (width - 20) + 10;
		int y = rand() % height;
		//pPaint->DrawImage( &singlePixel, x, y );
		bmp->SetPixel( x, y, noiseColor );
	}
#endif

	//画干扰线
	Gdiplus::Pen pen( noiseColor, 1 );
	int x1,y1,x2,y2;
	x1 = rand() % 20;
	x2 = rand() % 10 + width - 20;
	y1 = rand() % height;
	y2 = rand() % height;
	while ( abs( y1 - y2 ) < 5 )					// 防止横线出现
	{
		y2 = rand() % height;
	}
	pPaint->DrawLine( &pen, x1, y1, x2, y2 );

	wchar_t wbuf[128] = { 0, };

	_bstr_t bStr;
	bStr = buf;

	wcscpy_s( wbuf, _countof( wbuf ), bStr );

	//Gdiplus::RectF r( 0.0f, 0.0f, width*1.0f, (height - 3)*1.0f );
	//pPaint->DrawString( wbuf, -1, pFont, r,  &sf, brush );

	//修改排版
	int lineSpace = height - fontSize - 2*fontPadding;
	int len = strlen( buf );
	int left = ( width - (fontSize + fontPadding) * len )/2, 
		top = 0;
	for (int i=0; i<len; i++)
	{
		top = rand() % lineSpace;
		Gdiplus::SolidBrush brush( Gdiplus::Color( 255, rand()%255, rand()%255, rand()%255 ) );
		pPaint->DrawString( wbuf+i, 1, pFont, Gdiplus::PointF(left,top), &brush );
		left += ( fontSize + fontPadding );
	}

	//添加扭曲
	Gdiplus::Bitmap twistedBmp( width, height, PixelFormat24bppRGB);
	Gdiplus::Graphics* pNewPaint = Gdiplus::Graphics::FromImage( &twistedBmp );
	pNewPaint->Clear( backgroundColor );
	delete pNewPaint;

	const bool axisDir = true;						//轴向
	const double multValue = 2.5;					//倍乘
	const double phase = rand() % 4;				//相位
	double dBaseAxisLen = axisDir ? (double)height : (double)width;
	for (int i=0; i<width; i++)
	{
		for (int j=0; j<height; j++)
		{
			double dx = 0;
			dx = axisDir ? (2*M_PI * j)/dBaseAxisLen : (2*M_PI * i)/dBaseAxisLen;
			dx += phase;
			double dy = sin( dx );

			int nx = 0, ny = 0;
			nx = axisDir ? (i + dy * multValue) : i;
			ny = axisDir ? j : (j + dy *multValue);
			Gdiplus::Color color;
			bmp->GetPixel( i,j, &color);
			twistedBmp.SetPixel( nx, ny, color);
		}
	}

	Gdiplus::Rect rect(0, 0, width, height);
	char *bufImg = new char[width * height * 3];

	Gdiplus::BitmapData data;
	twistedBmp.LockBits( &rect, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &data );

	char* pSrc = (char*)data.Scan0;
	char* pDest = bufImg;

	int length = width * 3;

	for( int i = 0; i < height; i++ )
	{
		memcpy( pDest,  pSrc, length);
		pSrc += data.Stride;
		pDest += length;
	}

	twistedBmp.UnlockBits( &data );

	//delete pPen;
	delete brush;
	delete pFont;
	delete pPaint;
	delete bmp;

	return bufImg;
}

void GdiUtil::loadImage( IDirectDrawSurface7* pSurface, char* buf, int length, int width, int height )
{
	Gdiplus::Bitmap* pBmp = Gdiplus::Bitmap::FromDirectDrawSurface7( pSurface );
	Gdiplus::Graphics* pPaint = Gdiplus::Graphics::FromImage( pBmp );

	if( !pBmp || !pPaint )
		return ;

	// 生成一个默认的灰度调色板
	int size = pBmp->GetPaletteSize();
	Gdiplus::ColorPalette* pPal = (Gdiplus::ColorPalette*)malloc( size );
	if( !pPal )
		return ;

#pragma region 设置调色板内容

	for( UINT i = 0; i < pPal->Count; i++ )
	{
		pPal->Entries[i] = 0xff000000 | i << 24 | i << 16 | i;
	}

	pBmp->SetPalette( pPal );

#pragma endregion 设置调色板内容

	Gdiplus::Rect rect(0, 0, width, height);
	char *bufImg = new char[width * height * 3];

	Gdiplus::BitmapData data;
	pBmp->LockBits( &rect, Gdiplus::ImageLockModeRead, PixelFormat8bppIndexed, &data );

	char* pSrc = (char*)data.Scan0;
	char* pDest = bufImg;

	for( int i = 0; i < height; i++ )
	{
		memcpy( pDest,  pSrc, length);
		pSrc += data.Stride;
		pDest += length;
	}

	pBmp->UnlockBits( &data );


	free(pPal);
	delete pPaint;
	delete pBmp;
}
