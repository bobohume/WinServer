#include "stdafx.h"
#include "CharSet.h"

#include <windows.h>
#include <memory>

static U32 __dStrlen(const UTF16 *unistring)
{
   if(!unistring)
      return 0;

   U32 i = 0;

   while(unistring[i] != '\0')
      i++;
      
   return i;
}

//-----------------------------------------------------------------------------
/// replacement character. Standard correct value is 0xFFFD.
#define kReplacementChar 0xFFFD

/// Look up table. Shift a byte >> 1, then look up how many bytes to expect after it.
/// Contains -1's for illegal values.
static U8 firstByteLUT[128] = 
{
   1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 0x0F // single byte ascii
   1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 0x1F // single byte ascii
   1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 0x2F // single byte ascii
   1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 0x3F // single byte ascii

   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x4F // trailing utf8
   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x5F // trailing utf8
   2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2, // 0x6F // first of 2
   3, 3, 3, 3,  3, 3, 3, 3,  4, 4, 4, 4,  5, 5, 6, 0, // 0x7F // first of 3,4,5,illegal in utf-8
};

/// Look up table. Shift a 16-bit word >> 10, then look up whether it is a surrogate,
///  and which part. 0 means non-surrogate, 1 means 1st in pair, 2 means 2nd in pair.
static U8 surrogateLUT[64] = 
{
   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x0F 
   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x1F 
   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x2F 
   0, 0, 0, 0,  0, 0, 1, 2,  0, 0, 0, 0,  0, 0, 0, 0, // 0x3F 
};

/// Look up table. Feed value from firstByteLUT in, gives you
/// the mask for the data bits of that UTF-8 code unit.
static U8  byteMask8LUT[]  = { 0x3f, 0x7f, 0x1f, 0x0f, 0x07, 0x03, 0x01 }; // last 0=6, 1=7, 2=5, 4, 3, 2, 1 bits

/// Mask for the data bits of a UTF-16 surrogate.
static U16 byteMaskLow10 = 0x03ff;

//-----------------------------------------------------------------------------
static  bool isSurrogateRange(U32 codepoint)
{
   return ( 0xd800 < codepoint && codepoint < 0xdfff );
}

static  bool isAboveBMP(U32 codepoint)
{
   return ( codepoint > 0xFFFF );
}

static UTF32 _oneUTF8toUTF32( const UTF8* codepoint, U32 *unitsWalked)
{
   // codepoints 6 codeunits long are read, but do not convert correctly,
   // and are filtered out anyway.
   
   // early out for ascii
   if(!(*codepoint & 0x0080))
   {
      *unitsWalked = 1;
      return (UTF32)*codepoint;
   }
   
   U32 expectedByteCount;
   UTF32  ret = 0;
   U8 codeunit;
   
   // check the first byte ( a.k.a. codeunit ) .
   unsigned char c = codepoint[0];
   c = c >> 1;
   expectedByteCount = firstByteLUT[c];
   if(expectedByteCount > 0) // 0 or negative is illegal to start with
   {
      // process 1st codeunit
      ret |= byteMask8LUT[expectedByteCount] & codepoint[0]; // bug?
      
      // process trailing codeunits
      for(U32 i=1;i<expectedByteCount; i++)
      {
         codeunit = codepoint[i];
         if( firstByteLUT[codeunit>>1] == 0 )
         {
            ret <<= 6;                 // shift up 6
            ret |= (codeunit & 0x3f);  // mask in the low 6 bits of this codeunit byte.
         }
         else
         {
            // found a bad codepoint - did not get a medial where we wanted one.
            // Dump the replacement, and claim to have parsed only 1 char,
            // so that we'll dump a slew of replacements, instead of eating the next char.            
            ret = kReplacementChar;
            expectedByteCount = 1;
            break;
         }
      }
   }
   else 
   {
      // found a bad codepoint - got a medial or an illegal codeunit. 
      // Dump the replacement, and claim to have parsed only 1 char,
      // so that we'll dump a slew of replacements, instead of eating the next char.
      ret = kReplacementChar;
      expectedByteCount = 1;
   }
   
   if(unitsWalked != NULL)
      *unitsWalked = expectedByteCount;
   
   // codepoints in the surrogate range are illegal, and should be replaced.
   if(isSurrogateRange(ret))
      ret = kReplacementChar;
   
   // codepoints outside the Basic Multilingual Plane add complexity to our UTF16 string classes,
   // we've read them correctly so they won't foul the byte stream,
   // but we kill them here to make sure they wont foul anything else
   if(isAboveBMP(ret))
      ret = kReplacementChar;

   return ret;
}

static UTF16 _oneUTF8toUTF32(const UTF32 codepoint)
{
   // found a codepoint outside the encodable UTF-16 range!
   // or, found an illegal codepoint!
   if(codepoint >= 0x10FFFF || isSurrogateRange(codepoint))
      return kReplacementChar;
   
   // these are legal, we just don't want to deal with them.
   if(isAboveBMP(codepoint))
      return kReplacementChar;

   return (UTF16)codepoint;
}

static UTF32  _oneUTF16toUTF32(const UTF16* codepoint, U32 *unitsWalked)
{
   U8    expectedType;
   U32   unitCount;
   UTF32 ret = 0;
   UTF16 codeunit1,codeunit2;
   
   codeunit1 = codepoint[0];
   expectedType = surrogateLUT[codeunit1 >> 10];
   switch(expectedType)
   {
      case 0: // simple
         ret = codeunit1;
         unitCount = 1;
         break;
      case 1: // 2 surrogates
         codeunit2 = codepoint[1];
         if( surrogateLUT[codeunit2 >> 10] == 2)
         {
            ret = ((codeunit1 & byteMaskLow10 ) << 10) | (codeunit2 & byteMaskLow10);
            unitCount = 2;
            break;
         }
         // else, did not find a trailing surrogate where we expected one,
         // so fall through to the error
      case 2: // error
         // found a trailing surrogate where we expected a codepoint or leading surrogate.
         // Dump the replacement.
         ret = kReplacementChar;
         unitCount = 1;
         break;
      default:
         // unexpected return
         SERVER_ASSERT(false, "oneUTF16toUTF323: unexpected type");
         ret = kReplacementChar;
         unitCount = 1;
         break;
   }

   if(unitsWalked != NULL)
      *unitsWalked = unitCount;

   // codepoints in the surrogate range are illegal, and should be replaced.
   if(isSurrogateRange(ret))
      ret = kReplacementChar;

   // codepoints outside the Basic Multilingual Plane add complexity to our UTF16 string classes,
   // we've read them correctly so they wont foul the byte stream,
   // but we kill them here to make sure they wont foul anything else
   // NOTE: these are perfectly legal codepoints, we just dont want to deal with them.
   if(isAboveBMP(ret))
      ret = kReplacementChar;

   return ret;
}

static U32 _oneUTF32toUTF8(const UTF32 codepoint, UTF8 *threeByteCodeunitBuf)
{
   U32 bytecount = 0;
   UTF8 *buf;
   U32 working = codepoint;
   buf = threeByteCodeunitBuf;

   //-----------------
   if(isSurrogateRange(working))  // found an illegal codepoint!
      working = kReplacementChar;
   
   if(isAboveBMP(working))        // these are legal, we just dont want to deal with them.
      working = kReplacementChar;

   //-----------------
   if( working < (1 << 7))        // codeable in 7 bits
      bytecount = 1;
   else if( working < (1 << 11))  // codeable in 11 bits
      bytecount = 2;
   else if( working < (1 << 16))  // codeable in 16 bits
      bytecount = 3;

   assert( bytecount > 0 && "Error converting to UTF-8 in _oneUTF32toUTF8(). isAboveBMP() should have caught this!");

   //-----------------
   U8  mask = byteMask8LUT[0];            // 0011 1111
   U8  marker = ( ~mask << 1);            // 1000 0000
   
   // Process the low order bytes, shifting the codepoint down 6 each pass.
   for( int i = bytecount-1; i > 0; i--)
   {
      threeByteCodeunitBuf[i] = marker | (working & mask); 
      working >>= 6;
   }

   // Process the 1st byte. filter based on the # of expected bytes.
   mask = byteMask8LUT[bytecount];
   marker = ( ~mask << 1 );
   threeByteCodeunitBuf[0] = marker | working & mask;
   
   return bytecount;
}

U32 UTF8toUTF16(const UTF8 *unistring, UTF16 *outbuffer, U32 len)
{
   // GB2312 -> unicode(UTF16) [9/8/2008 joy]
   U32 BuffLen = MultiByteToWideChar(936, MB_PRECOMPOSED, unistring, -1, (LPWSTR)outbuffer, len);

   if(BuffLen == 0)
   {
      UTF8* instr = new UTF8[len * sizeof(UTF8)];
      dStrncpy(instr, len * sizeof(UTF8), unistring, len-1);
	  instr[len] = 0;
      BuffLen = MultiByteToWideChar(936, MB_PRECOMPOSED, instr, -1, (LPWSTR)outbuffer, len);
      delete []instr;
   }

   if(BuffLen > 0)
   {
      BuffLen = getMin(BuffLen,len - 1);
      outbuffer[BuffLen] = '\0';
   }
   else
   {
      U32 walked, nCodepoints;
      UTF32 middleman;
   
      nCodepoints=0;

      while(*unistring != '\0' && nCodepoints < len)
      {
         walked = 1;
         middleman = _oneUTF8toUTF32(unistring,&walked);
         outbuffer[nCodepoints] = _oneUTF8toUTF32(middleman);
         unistring+=walked;
         nCodepoints++;
      }
      BuffLen = getMin(nCodepoints,len - 1);
      outbuffer[BuffLen] = '\0';
   }
   
   return BuffLen; 
}

U32 UTF16toUTF8( const UTF16 *unistring, UTF8  *outbuffer, U32 len)
{
   // unicode(UTF16) -> GB2312 [9/8/2008 joy]
   U32 BuffLen = WideCharToMultiByte( 936, WC_NO_BEST_FIT_CHARS,(LPCWSTR)unistring, __dStrlen(unistring), outbuffer, len, NULL, false);

   if(BuffLen > 0)
   {
      BuffLen = getMin(BuffLen,len - 1);
      outbuffer[BuffLen] = '\0';
   }
   else
   {
      U32 walked, nCodeunits, codeunitLen;
      UTF32 middleman;

      nCodeunits=0;
      while( *unistring != '\0' && nCodeunits + 3 < len )
      {
         walked = 1;
         middleman  = _oneUTF16toUTF32(unistring,&walked);
         codeunitLen = _oneUTF32toUTF8(middleman, &outbuffer[nCodeunits]);
         unistring += walked;
         nCodeunits += codeunitLen;
      }

      BuffLen = getMin(nCodeunits,len - 1);
      outbuffer[BuffLen] = '\0';
   }
   return BuffLen;
}

UTF16* UTF8toUTF16( const UTF8* unistring)
{
   // allocate plenty of memory.
   U32 nCodepoints, len = dStrlen(unistring) + 1;
   UTF16* buf = new UTF16[len];
   
   // perform conversion
   nCodepoints = UTF8toUTF16( unistring, buf, len);
   
   // add 1 for the NULL terminator the converter promises it included.
   nCodepoints++;
   
   // allocate the return buffer, copy over, and return it.
   UTF16 *ret = new UTF16[nCodepoints];
   dMemcpy(ret, buf, nCodepoints * sizeof(UTF16));
   delete []buf;
   return ret;
}

UTF8* convertUTF16toUTF8( const UTF16* unistring)
{
   // allocate plenty of memory.
   U32 nCodeunits, len = __dStrlen(unistring) * 3 + 1;
   UTF8* buf = new UTF8[len];
   
   // perform conversion
   nCodeunits = UTF16toUTF8( unistring, buf, len);
   
   // add 1 for the NULL terminator the converter promises it included.
   nCodeunits++;
   
   // allocate the return buffer, copy over, and return it.
   UTF8 *ret = new UTF8[nCodeunits];
   dMemcpy(ret, buf, nCodeunits * sizeof(UTF8));
   delete []buf;
   return ret;
}