#ifndef _SAFE_STRING_
#define _SAFE_STRING_

#include <tchar.h>

__inline bool sTestBuffer(const char *pSource,int iSSize,const char *pDest,int iDSize)
{
	bool ok = true;

	if(!pSource || !pDest)
		ok = false;
	else
	{
		int size = (int)strlen(pSource);
		if(size >= iSSize)
			ok = false;
		else if(size >= iDSize)
			ok = false;
	}
	return ok;
}

__inline int sStrlen(const char *pSource,int iSSize)
{
	int size = (int)strlen(pSource);
	if(size>iSSize-1)
		size = iSSize-1;

	return size;
}

__inline bool sStrcpy(char *pDest,int iDSize,const char *pSource,int iSSize)
{
	if(!pDest)
		return false;
	else
		pDest[0] = 0;

	if(!sTestBuffer(pSource,iSSize,pDest,iDSize))
		return false;

	strcpy_s(pDest, iDSize, pSource);
	return true;
}

__inline int sStrcmp(const char *pSource,int iSSize,const char *pDest,int iDSize)
{
	if(!sTestBuffer(pSource,iSSize,pDest,iDSize))
		return false;

	return strcmp(pSource,pDest);
}

__inline int sStricmp(const char *pSource,int iSSize,const char *pDest,int iDSize)
{
	if(!sTestBuffer(pSource,iSSize,pDest,iDSize))
		return false;

	return _stricmp(pSource,pDest);
}

#endif



