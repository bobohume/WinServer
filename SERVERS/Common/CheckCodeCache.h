#ifndef CHECKCODECACHE_H
#define CHECKCODECACHE_H

#define CHECKCODE_COUNT		1000

struct CacheItem 
{
	char str[8];
	char *image;
	CacheItem()	{ 
		str[0]=str[1]=str[2]=str[3]=str[4]=str[5]=str[6]=str[7]=0;
		image=0;
	}
};

class CheckCodeCache
{
public:
	static CheckCodeCache * mInstancePtr;
	static const int mImageWidth;
	static const int mImageHeight;

    CheckCodeCache(void);

	static CheckCodeCache * GetInstancePtr()
	{
		if ( !mInstancePtr )
		{
			mInstancePtr = new CheckCodeCache;
		}

		return mInstancePtr;
	}

	static void FreeInstance()
	{
		if ( mInstancePtr )
		{
			delete mInstancePtr;
			mInstancePtr = 0;
		}
	}

public:
	void Init();
	void Dispose();

	CacheItem* GetRandomCheckCode();
	
protected:
	CacheItem *mCacheData;
};

#endif // CHECKCODECACHE_H