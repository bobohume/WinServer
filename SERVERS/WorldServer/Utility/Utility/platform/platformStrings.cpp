
#include <stdarg.h>
#include <stdio.h>
#include "platformStrings.h"
#include "stringTable.h"

// This standard function is not defined when compiling with VC7...
#define vsnprintf	_vsnprintf_s

//------------------------------------------------------------------------------
// non-standard string functions

char *dStrdup_r(const char *src, const char *fileName, size_t lineNumber)
{
   S32 length = dStrlen(src) + 1;
   char *buffer = (char *) dMalloc(length);
   dStrcpy(buffer, length, src);
   return buffer;
}


// concatenates a list of src's onto the end of dst
// the list of src's MUST be terminated by a NULL parameter
// dStrcatl(dst, sizeof(dst), src1, src2, NULL);
char* dStrcatl(char *dst, size_t dstSize, ...)
{
   const char* src = NULL;
   char *p = dst;

   LIB_ASSERT(dstSize > 0, "dStrcatl: destination size is set zero");
   dstSize--;  // leave room for string termination

   // find end of dst
   while (dstSize && *p++)
      dstSize--;

   va_list args;
   va_start(args, dstSize);

   // concatenate each src to end of dst
   while ( (src = va_arg(args, const char*)) != NULL )
   {
      while( dstSize && *src )
      {
         *p++ = *src++;
         dstSize--;
      }
   }

   va_end(args);

   // make sure the string is terminated
   *p = 0;

   return dst;
}


// copy a list of src's into dst
// the list of src's MUST be terminated by a NULL parameter
// dStrccpyl(dst, sizeof(dst), src1, src2, NULL);
char* dStrcpyl(char *dst, size_t dstSize, ...)
{
   const char* src = NULL;
   char *p = dst;

   LIB_ASSERT(dstSize > 0, "dStrcpyl: destination size is set zero");
   dstSize--;  // leave room for string termination

   va_list args;
   va_start(args, dstSize);

   // concatenate each src to end of dst
   while ( (src = va_arg(args, const char*)) != NULL )
   {
      while( dstSize && *src )
      {
         *p++ = *src++;
         dstSize--;
      }
   }

   va_end(args);

   // make sure the string is terminated
   *p = 0;

   return dst;
}
 

int dStrcmp( const UTF16 *str1, const UTF16 *str2)
{
#if defined(POWER_OS_WIN32) || defined(POWER_OS_XBOX) || defined(POWER_OS_XENON)
   return wcscmp( reinterpret_cast<const wchar_t *>( str1 ), reinterpret_cast<const wchar_t *>( str2 ) );
#else
   int ret;
   const UTF16 *a, *b;
   a = str1;
   b = str2;

   while( *a && *b && (ret = *a - *b) == 0)
      a++, b++;

   return ret;
#endif
}  

char* dStrupr(char *str)
{
   if (str == NULL)
      return(NULL);

   char* saveStr = str;

   while (*str)
   {
      *str = toupper(*str);
      str++;
   }

   return saveStr;
}

char* dStrlwr(char *str)
{
   if (str == NULL)
      return(NULL);

   char* saveStr = str;
   while (*str)
   {
      *str = tolower(*str);
      str++;
   }

   return saveStr;
}

UTF16* dStrupr(UTF16 *str)
{
    if (str == NULL)
      return(NULL);

   UTF16* saveStr = str;
   while (*str)
   {
      if (iswlower(*str))
      {
        *str = toupper(*str);
      }

      str++;
   }
   return saveStr;
}

UTF16* dStrlwr(UTF16 *str)
{
    if (str == NULL)
        return(NULL);

    UTF16* saveStr = str;
    while (*str)
    {
        if (iswupper(*str))
        {
            *str = tolower(*str);
        }

        str++;
    }
    return saveStr;
}

//------------------------------------------------------------------------------
// standard I/O functions

void dPrintf(const char *format, ...)
{
   va_list args;
   va_start(args, format);
   vprintf(format, args);
}

S32 dVprintf(const char *format, void *arglist)
{
   return vprintf(format, (char*)arglist);
}

S32 dSprintf(char *buffer, U32 bufferSize, const char *format, ...)
{
   va_list args;
   va_start(args, format);

   S32 len = vsnprintf(buffer, bufferSize, bufferSize, format, args);

   LIB_ASSERT( len < (int)bufferSize, "dSprintf wrote to more memory than the specified buffer size" );

   return (len);
}


S32 dVsprintf(char *buffer, U32 bufferSize, const char *format, void *arglist)
{
   S32 len = vsnprintf(buffer, bufferSize, bufferSize, format, (char*)arglist);

   LIB_ASSERT( len < (int)bufferSize, "dVsprintf wrote to more memory than the specified buffer size" );

   return (len);
}


S32 dSscanf(const char *buffer, const char *format, ...)
{
   va_list args;
   va_start(args, format);

   // Boy is this lame.  We have to scan through the format string, and find out how many
   //  arguments there are.  We'll store them off as void*, and pass them to the sscanf
   //  function through specialized calls.  We're going to have to put a cap on the number of args that
   //  can be passed, 8 for the moment.  Sigh.
   static void* sVarArgs[20];
   U32 numArgs = 0;

   for (const char* search = format; *search != '\0'; search++) {
      if (search[0] == '%' && search[1] != '%')
         numArgs++;
   }
   LIB_ASSERT(numArgs <= 20, "Error, too many arguments to lame implementation of dSscanf.  Fix implmentation");

   // Ok, we have the number of arguments...
   for (U32 i = 0; i < numArgs; i++)
      sVarArgs[i] = va_arg(args, void*);
   va_end(args);

   switch (numArgs) {
     case 0: return 0;
     case 1:  return sscanf_s(buffer, format, sVarArgs[0]);
     case 2:  return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1]);
     case 3:  return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2]);
     case 4:  return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3]);
     case 5:  return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4]);
     case 6:  return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5]);
     case 7:  return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6]);
     case 8:  return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7]);
     case 9:  return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8]);
     case 10: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9]);
     case 11: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10]);
     case 12: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11]);
     case 13: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12]);
     case 14: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13]);
     case 15: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14]);
     case 16: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15]);
     case 17: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15], sVarArgs[16]);
     case 18: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15], sVarArgs[16], sVarArgs[17]);
     case 19: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15], sVarArgs[16], sVarArgs[17], sVarArgs[18]);
     case 20: return sscanf_s(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15], sVarArgs[16], sVarArgs[17], sVarArgs[18], sVarArgs[19]);
   }
   return 0;
}

std::string dTrim(std::string str)
{
	if(str.empty())
		return str;

	str.erase(0,str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ")+1);

	/*std::string::iterator it;
	for(it=str.begin(); it != str.end() && iswspace(*it++););
	str.erase(str.begin(), --it);

	for(it=str.end(); it != str.begin() && iswspace(*--it););
	str.erase(++it, str.end());*/
	return str;
}