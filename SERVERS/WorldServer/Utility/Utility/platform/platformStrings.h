//-----------------------------------------------------------------------------
// PowerEngine 
// Copyright (C) FairRain.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORM_STRINGS_H_
#define _PLATFORM_STRINGS_H_

#include "base/types.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mbstring.h>
#include <string>

// These standard functions are not defined on Win32 and other Microsoft platforms...
#define strcasecmp   _stricmp
#define strncasecmp  _strnicmp
#define strtof       (double)strtod

//------------------------------------------------------------------------------
// standard string functions [defined in platformString.cpp]

inline char *dStrcat(char *dst, size_t dstSize, const char *src)
{
   char *ret = NULL;

   if (strcat_s(dst, dstSize, src) == 0)
   {
		ret = dst;
   }

   return ret;
}   

inline char *dStrncat(char *dst, size_t dstSize, const char *src, size_t len)
{
   char *ret = NULL;

   if (strncat_s(dst, dstSize, src, len) == 0)
   {
	   ret = dst;
   }

   return ret;
}

inline int  dStrcmp(const char *str1, const char *str2)
{
   return strcmp(str1, str2);   
}

inline int  dStrncmp(const char *str1, const char *str2, size_t len)
{
   return strncmp(str1, str2, len);   
}  

inline int  dStricmp(const char *str1, const char *str2)
{
   return strcasecmp( str1, str2 );
}

inline int  dStrnicmp(const char *str1, const char *str2, size_t len)
{
   return strncasecmp( str1, str2, len );
}

inline char *dStrchr(char *str, int c)
{
   //return strchr(str,c);
	return (char*)_mbschr((unsigned char*)str,c);
}   

inline const char *dStrchr(const char *str, int c)
{
   //return strchr(str,c);
	return (const char*)_mbschr((const unsigned char*)str,c);
}   

inline const char *dStrrchr(const char *str, int c)
{
   //return strrchr(str,c);
	return (const char*)_mbsrchr((const unsigned char*)str,c);
}   

inline char *dStrrchr(char *str, int c)
{
   //return strrchr(str,c);
	return (char*)_mbsrchr((const unsigned char*)str,c);
}

inline size_t dStrspn(const char *str, const char *set)
{
   //return (size_t)strspn(str, set);
	return (size_t)_mbsspn((const unsigned char*)str, (const unsigned char*)set);
}

inline size_t dStrcspn(const char *str, const char *set)
{
   //return (size_t)strcspn(str, set);
	return (size_t)_mbscspn((const unsigned char*)str, (const unsigned char*)set);
}   

inline char *dStrtok(char *str, const char *sep, char **context)
{
	//return strtok_s(str, sep, context);
	return (char *)_mbstok_s((unsigned char*)str, (const unsigned char *)sep, (unsigned char**)context);
}

inline S32 dAtoi(const char *str)
{
   return strtol(str, NULL, 10);
}

inline U32 dAtol(const char *str)
{
	return strtoul(str, NULL, 10);
}

inline S64 dAtoi64(const char *str)
{
	return _strtoi64(str, NULL, 10);
}

inline U64 dAtol64(const char *str)
{
	return _strtoui64(str, NULL, 10);
}

inline F64 dAtof(const char *str)
{
   return strtod(str, NULL);
}

inline char dToupper(const char c)
{
   return toupper( (int)c );
}

inline char dTolower(const char c)
{
   return tolower( (int)c );
}

inline bool dIsalnum(const char c)
{
   //return isalnum((int)c);
	return c > 0 ? dIsalnum((int)c) : false;
}

inline bool dIsalpha(const char c)
{
   //return isalpha((int)c);
	return c > 0 ? dIsalpha((int)c) : false;
}

inline bool dIsspace(const char c)
{
   //return isspace((int)c);
	return c > 0 ? (0 != isspace((int)c)) : false;
}

inline bool dIsdigit(const char c)
{
   //return isdigit((int)c);
	return c > 0 ? (0 != isdigit((int)c)) : 0;
}


//------------------------------------------------------------------------------
// non-standard string functions [defined in platformString.cpp]

#define dStrdup(x) dStrdup_r(x, __FILE__, __LINE__)
extern char* dStrdup_r(const char *src, const char*, size_t);

extern char*       dStrcpyl(char *dst, size_t dstSize, ...);
extern char*       dStrcatl(char *dst, size_t dstSize, ...);

extern int         dStrcmp(const UTF16 *str1, const UTF16 *str2);

extern char*       dStrupr(char *str);
extern char*       dStrlwr(char *str);
extern UTF16*      dStrupr(UTF16 *str);
extern UTF16*      dStrlwr(UTF16 *str);

inline bool dAtob(const char *str)
{
   return !dStricmp(str, "true") || dAtof(str);
}

//------------------------------------------------------------------------------
// standard I/O functions [defined in platformString.cpp]

extern void   dPrintf(const char *format, ...);
extern int    dVprintf(const char *format, void *arglist);
extern int    dSprintf(char *buffer, U32 bufferSize, const char *format, ...);
extern int    dVsprintf(char *buffer, U32 bufferSize, const char *format, void *arglist);
extern int    dSscanf(const char *buffer, const char *format, ...);
extern std::string   dTrim(std::string str);
#endif
