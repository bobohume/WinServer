#include "CheckCodeCache.h"
#include "GdiUtil.h"
#include "base/log.h"

CheckCodeCache *CheckCodeCache::mInstancePtr = NULL;
const int CheckCodeCache::mImageWidth = 90;
const int CheckCodeCache::mImageHeight = 20;

CheckCodeCache::CheckCodeCache(void)
{
    mCacheData = 0;
}

void CheckCodeCache::Init()
{
#ifdef NTJ_ROBOT
    return;
#endif

    g_Log.WriteLog("正在初始化校验码...");

	mCacheData = new CacheItem[CHECKCODE_COUNT];
	memset( mCacheData, 0, sizeof(CacheItem)*CHECKCODE_COUNT );

	static char CharSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz";
	static int CharSetCount = 62;

	int length = mImageWidth * mImageHeight;

	srand( ::GetTickCount() );

	for (int i=0; i<CHECKCODE_COUNT; i++)
	{
		for( int j = 0; j < 4; j++ )
		{
			mCacheData[i].str[j] = CharSet[int( ( float( rand() ) / RAND_MAX ) * ( CharSetCount - 1 ) )];
		}

		mCacheData[i].image = GdiUtil::genImage( mCacheData[i].str, mImageWidth, mImageHeight );
	}
}

void CheckCodeCache::Dispose()
{
	if ( mCacheData )
	{
		for (int i=0; i<CHECKCODE_COUNT; i++)
		{
			char *data = (mCacheData+i)->image;
			if ( data )
			{
				delete [] data;
			}
		}

		delete [] mCacheData;
		mCacheData = NULL;
	}
}	

CacheItem* CheckCodeCache::GetRandomCheckCode()
{
	int i = rand() % CHECKCODE_COUNT;
	return mCacheData + i;
}