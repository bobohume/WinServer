#include "UtilString.h"
#include <Windows.h>
#include <assert.h>
#include <string>

wchar_t* Util::Utf8ToUnicode(const char* pUtf8)
{
	assert(pUtf8);
	int nChar = MultiByteToWideChar(CP_UTF8, 0, pUtf8, -1, NULL, 0);

	wchar_t* pwchBuffer = new wchar_t[nChar+1];

	nChar = MultiByteToWideChar(CP_UTF8, 0, pUtf8, -1, pwchBuffer, nChar+1);
	if (nChar == 0)
	{
		delete[] pwchBuffer;
		return NULL;
	}

	pwchBuffer[nChar] = 0;
	return pwchBuffer;
}


char* Util::UnicodeToUtf8(const wchar_t* pUnicode)
{
	assert(pUnicode);
	int nChar = WideCharToMultiByte(CP_UTF8, 0, pUnicode, -1, 0, 0, 0, 0);

	char* pchBuffer = new char[nChar+1];

	nChar = WideCharToMultiByte(CP_UTF8, 0, pUnicode, -1, pchBuffer, nChar+1, 0, 0);
	if (nChar == 0)
	{
		delete[] pchBuffer;
		return NULL;
	}

	pchBuffer[nChar] = 0;
	return pchBuffer;
}

wchar_t* Util::MbcsToUnicode(const char* pMbcs)
{
	int codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
	int nChar = MultiByteToWideChar(codepage, 0, pMbcs, -1, NULL,0);

	wchar_t* pwchBuffer = new wchar_t[nChar+1];

	nChar = MultiByteToWideChar(codepage, 0, pMbcs, -1, pwchBuffer, nChar+1);
	if (nChar == 0)
	{
		delete[] pwchBuffer;
		return NULL;
	}

	pwchBuffer[nChar] = 0;
	return pwchBuffer;
}

char* Util::UnicodeToMbcs(const wchar_t* pUnicode)
{
	int codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
	int nChar = WideCharToMultiByte(codepage, 0, pUnicode, -1, 0, 0, 0, 0);

	char* pchBuffer = new char[nChar+1];

	nChar = WideCharToMultiByte(codepage, 0, pUnicode, -1, pchBuffer, nChar+1, 0, 0);
	if (nChar == 0)
	{
		delete[] pchBuffer;
		return NULL;
	}

	pchBuffer[nChar] = 0;
	return pchBuffer;
}

char* Util::MbcsToUtf8(const char* pMbcs)
{
	assert(pMbcs);
	wchar_t* pUnicode = MbcsToUnicode(pMbcs);
	char* pRet = UnicodeToUtf8(pUnicode);
	delete[] pUnicode;
	return pRet;
}

std::string Util::MbcsToUtf8(std::string strMbcs)
{
	std::string str;
	char* temp = MbcsToUtf8(strMbcs.c_str());
	str = temp;
	delete[] temp;
	return str;
}

std::string Util::Utf8ToMbcs(std::string strUtf8)
{
	std::string str;
	char* pRet = Utf8ToMbcs(strUtf8.c_str());
	str = pRet;
	delete[] pRet;
	return str;
}

char* Util::Utf8ToMbcs(const char* pUtf8)
{
	assert(pUtf8);
	wchar_t* pUnicode = Utf8ToUnicode(pUtf8);
	char* pRet = UnicodeToMbcs(pUnicode);
	delete[] pUnicode;
	return pRet;
}

bool Util::StringToInt(const char* strValue, int& iValue)
{
	iValue = 0;
	bool br = true;
	if (!strValue || !*strValue)
	{
		return false;
	}
	while (isspace((int)*strValue))
	{
		strValue ++;
	}
	if (*strValue == '+' || *strValue == '-')
	{
		br = (*strValue++ == '+');
	}
	bool bOK = true;
	if (strValue[0] == '0' && (strValue[1]|0x20)=='x')
	{
		strValue += 2;
		for (;;)
		{
			TCHAR ch = *strValue;
			int iValue2 = 0;
			if (ch >= '0' && ch <= '9')	iValue2 = iValue*16 + ch -'0';
			else if (ch>='a' && ch<='f') iValue2 = iValue*16 + ch -'a'+10;
			else if (ch>='A' && ch<='F') iValue2 = iValue*16 + ch -'A'+10;
			else break;
			if (iValue2 < 0 || iValue >= 134217728)	bOK = false;
			iValue = iValue2;
			++strValue;
		}
	}
	else
	{
		while (*strValue >= '0' && *strValue <= '9')
		{
			int iValue2 = iValue * 10 + *strValue++ -'0';
			if (iValue2 < 0 || iValue > 214748364) bOK = false;
			iValue = iValue2;
		}
	}
	if (!br) iValue = -iValue;
	while (*strValue && isspace((BYTE)*strValue)) ++strValue;
	return bOK && strValue[0] == 0;
}

bool Util::StringToDWord(const char* strValue, unsigned int& dwValue)
{
	dwValue = 0;
	if (!strValue || !*strValue)
	{
		return false;
	}
	while (isspace((int)*strValue))
	{
		strValue ++;
	}
	if (*strValue == '+')
	{
		strValue ++;
	}
	bool bOK = true;
	if (strValue[0] == '0' && (strValue[1]|0x20)=='x')
	{
		strValue += 2;
		for (;;)
		{
			TCHAR ch = *strValue;
			unsigned int dwValue2 = 0;
			if (ch >= '0' && ch <= '9')	dwValue2 = dwValue*16 + ch -'0';
			else if (ch>='a' && ch<='f') dwValue2 = dwValue*16 + ch -'a'+10;
			else if (ch>='A' && ch<='F') dwValue2 = dwValue*16 + ch -'A'+10;
			else break;
			if (dwValue2 < dwValue || dwValue >= 268435456) bOK = false;
			dwValue = dwValue2;
			++strValue;
		}
	}
	else
	{
		while (*strValue >= '0' && *strValue <= '9')
		{
			unsigned int dwValue2 = dwValue * 10 + *strValue++ -'0';
			if (dwValue2 < dwValue || dwValue > 429496729) bOK = false;
			dwValue = dwValue2;
		}
	}
	while (*strValue && isspace((BYTE)*strValue)) ++strValue;
	return bOK && strValue[0] == 0;
}

bool Util::StringToInt64(const char* strValue, long long& i64Value)
{
	i64Value = 0;
	bool br = true;
	if (!strValue || !*strValue)
	{
		return false;
	}
	while (isspace((int)*strValue))
	{
		strValue ++;
	}
	if (*strValue == '+' || *strValue == '-')
	{
		br = (*strValue++ == '+');
	}
	bool bOK = true;
	if (strValue[0] == '0' && (strValue[1]|0x20)=='x')
	{
		strValue += 2;
		for (;;)
		{
			TCHAR ch = *strValue;
			long long i64Value2 = 0;
			if (ch >= '0' && ch <= '9')	i64Value2 = i64Value*16 + ch -'0';
			else if (ch>='a' && ch<='f') i64Value2 = i64Value*16 + ch -'a'+10;
			else if (ch>='A' && ch<='F') i64Value2 = i64Value*16 + ch -'A'+10;
			else break;
			if (i64Value2 < 0 || i64Value >= 576460752303423488ULL) bOK = false;
			i64Value = i64Value2;
			++strValue;
		}
	}
	else
	{
		while (*strValue >= '0' && *strValue <= '9')
		{
			long long i64Value2 = i64Value * 10 + *strValue++ -'0';
			if (i64Value2 < 0 || i64Value > 922337203685477580ULL) bOK = false;

			i64Value = i64Value2;
		}
	}
	if (!br) i64Value = -i64Value;
	while (*strValue && isspace((BYTE)*strValue)) ++strValue;
	return bOK && strValue[0] == 0;
}

bool Util::StringToUnsignedInt64(const char* strValue, unsigned long long& ui64Value)
{
	ui64Value = 0;
	if (!strValue || !*strValue)
	{
		return false;
	}
	while (isspace((int)*strValue))
	{
		strValue ++;
	}
	if (*strValue == '+')
	{
		strValue ++;
	}
	bool bOK = true;
	if (strValue[0] == '0' && (strValue[1]|0x20)=='x')
	{
		strValue += 2;
		for (;;)
		{
			TCHAR ch = *strValue;
			unsigned long long ui64Value2 = 0;
			if (ch >= '0' && ch <= '9')	ui64Value2 = ui64Value*16 + ch -'0';
			else if (ch>='a' && ch<='f') ui64Value2 = ui64Value*16 + ch -'a'+10;
			else if (ch>='A' && ch<='F') ui64Value2 = ui64Value*16 + ch -'A'+10;
			else break;
			if (ui64Value2 < ui64Value || ui64Value >= 1152921504606846976ULL) bOK = false;
			ui64Value = ui64Value2;
			++strValue;
		}
	}
	else
	{
		while (*strValue >= '0' && *strValue <= '9')
		{
			unsigned long long ui64Value2 = ui64Value * 10 + *strValue++ -'0';
			if (ui64Value2 < ui64Value || ui64Value > 1844674407370955161ULL) bOK = false;
			ui64Value = ui64Value2;
		}
	}
	while (*strValue && isspace((BYTE)*strValue)) ++strValue;
	return bOK && strValue[0] == 0;
}

char* Util::FormUrlEncode(const char* pEncode)
{	
	std::string result;
	for (int i=0; pEncode[i]; i++)
	{
		if (isalnum((BYTE)pEncode[i]))
		{
			result += pEncode[i];
		}
		else if (isspace((BYTE)pEncode[i]))
		{
			result += '+';
		}
		else
		{
			char tempbuff[4];
			sprintf_s(tempbuff, sizeof(tempbuff), "%%%X%X", ((BYTE)pEncode[i])>>4, ((BYTE)pEncode[i])%16);
			result += tempbuff;
		}
	}
	char * szBuffer = new char[result.length() + 1];
	strncpy_s(szBuffer, result.length()+1, result.c_str(), result.length());
	return szBuffer;
}

char CharToInt(char ch)
{
	//做为解Url使用
	if(ch>='0' && ch<='9') return (char)(ch-'0');
	if(ch>='a' && ch<='f') return (char)(ch-'a'+10);
	if(ch>='A' && ch<='F') return (char)(ch-'A'+10);
	return -1;
}

char StrToBin(char* str)
{
	char tempWord[2];
	char chn;
	tempWord[0] = CharToInt(str[0]);			//make the B to 11 -- 00001011
	tempWord[1] = CharToInt(str[1]);			//make the 0 to 0 -- 00000000
	chn = (tempWord[0] << 4) | tempWord[1];		//to change the BO to 10110000
	return chn;
}

char* Util::FormUrlDecode(const char* pDecode)
{
	std::string result;
	int i = 0;
	while (pDecode[i])
	{
		if (pDecode[i] == '%'){
			char tmp[2];
			tmp[0] = pDecode[i+1];
			tmp[1] = pDecode[i+2];
			result += StrToBin(tmp);
			i = i+3;
		}
		else if (pDecode[i] == '+'){
			result += ' ';
			i++;
		}
		else{
			result += pDecode[i];
			i++;
		}
	}
	char * szBuffer = new char[result.length() + 1];
	strncpy_s(szBuffer, result.length()+1, result.c_str(), result.length());
	return szBuffer;
}

char* Util::MbcsToUrlEncodeUtf8(const char* pMbcs)
{
	assert(pMbcs);
	char* pUtf8 = MbcsToUtf8(pMbcs);
	char* pRet = FormUrlEncode(pUtf8);
	delete[] pUtf8;
	return pRet;
}

char* Util::Utf8ToUrlDecodeMbcs(const char* pUtf8)
{
	assert(pUtf8);
	char* pDecode = FormUrlDecode(pUtf8);
	char* pRet = Utf8ToMbcs(pDecode);
	delete[] pDecode;
	return pRet;
}