#ifndef _CHARSET_H_
#define _CHARSET_H_

#include "CommLib.h"

extern UTF16* UTF8toUTF16(const UTF8 *unistring);
extern UTF8*  UTF16toUTF8(const UTF16* unistring);
extern U32    UTF8toUTF16(const UTF8 *unistring,UTF16 *outbuffer, U32 len);
extern U32    UTF16toUTF8(const UTF16 *unistring,UTF8  *outbuffer, U32 len);

#endif /*_CHARSET_H_*/