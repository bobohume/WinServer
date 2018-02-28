#ifndef _UTIL_STRING_H_
#define _UTIL_STRING_H_

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

namespace Util
{
	//不同格式字符串之间的转换函数
	//返回值用完后需调用delete[]释放
	char*	    MbcsToUtf8(const char* pMbcs);
	char*	    Utf8ToMbcs(const char* pUtf8);
	wchar_t*	Utf8ToUnicode(const char* pUtf8);
	char*		UnicodeToUtf8(const wchar_t* pUnicode);
	wchar_t*	MbcsToUnicode(const char* pMbcs);
	char*		UnicodeToMbcs(const wchar_t* pUnicode);

	//字符串到数字的转换函数
	bool		StringToInt(const char* pValue, int & nValue);
	bool		StringToDWord(const char* pValue, unsigned int & dwValue);
	bool		StringToInt64(const char* pValue, long long & i64Value);
	bool		StringToUnsignedInt64(const char* pValue, unsigned long long & ui64Value);


	///和aouth相关的URL编码解码函数
	//返回值用完后需调用delete[]释放
	char*		FormUrlEncode(const char* pEncode);
	char*		FormUrlDecode(const char* pDecode);

	char*		MbcsToUrlEncodeUtf8(const char* pMbcs);
	char*		Utf8ToUrlDecodeMbcs(const char* pUtf8);
}

#endif