#include "FilterOpt.h"
#include "Common/BanInfoData.h"
#include "AtlConv.h"

#if !( defined( WORLDSERVER ) | defined( CHATSERVER ) )
#include "platform/types.h"
#include "platform/platformAssert.h"
#include "console/console.h"
#else
#include "base/types.h"
#define AssertError(a,b,c) { if(!a) c; }
#endif



bool FilterOpt::isBanName(const char* pName)
{
	AssertError(pName != NULL,"Invalid Par In FilterOpt::isBanName",return true);

	std::wstring wName;
	A2W_s(pName, wName);

#if !defined( CHATSERVER )
	U32 uLen = wcslen(wName.c_str());
	for (U32 i=0; i < uLen; ++i)
	{
		if (wName[i] <= 255 && !iswdigit(wName[i]) && !iswalpha(wName[i]) && wName[i] != L'_')
		{
			return true;
		}
// 		if ((wName[i] >=0 && wName[i] < 32) || wName[i] == L'\'' || wName[i] == L',' || wName[i] == L'|' || wName[i] == L'\\' || wName[i] == L'/')
//			return true;
	}
#endif

	const CBanInfoRepository::VEC_STR& vecBanName = g_BanInfoRepository.getBaneName();
	CBanInfoRepository::VEC_STR_CONITER iterBegin = vecBanName.begin();
	CBanInfoRepository::VEC_STR_CONITER iterEnd = vecBanName.end();

	for(;iterBegin != iterEnd; ++iterBegin)
	{
		if (iterBegin->size() > 0 && wcsstr(wName.c_str(), iterBegin->c_str()) != NULL)
			return true;
	}

#if ( defined(WORLDSERVER) | defined(CHATSERVER) )
	return false;
#endif

	return isBanWord(pName);
}

bool FilterOpt::isBanWord(const char* pWord)
{
	AssertError(pWord != NULL,"Invalid Par In FilterOpt::isBanWord",return true);

	std::wstring wWord;
	A2W_s(pWord, wWord);

	const CBanInfoRepository::VEC_STR& vecBanWord = g_BanInfoRepository.getBaneWord();
	CBanInfoRepository::VEC_STR_CONITER iterBegin = vecBanWord.begin();
	CBanInfoRepository::VEC_STR_CONITER iterEnd = vecBanWord.end();

	for(;iterBegin != iterEnd; ++iterBegin)
	{
		if (iterBegin->size() > 0 && wcsstr(wWord.c_str(), iterBegin->c_str()) != NULL)
			return true;
	}
	return false;
}

bool FilterOpt::replaceBanWord(char* pNewWord,U32 uSize,const char* pOldWord,const char cReplace /* = '*'*/ )
{
	AssertError(pNewWord != NULL && pOldWord != NULL,"Invalid Par In FilterOpt::replaceBanWord",return true);

	const CBanInfoRepository::VEC_STR& vecBanWord = g_BanInfoRepository.getBaneWord();
	CBanInfoRepository::VEC_STR_CONITER iterBegin = vecBanWord.begin();
	CBanInfoRepository::VEC_STR_CONITER iterEnd = vecBanWord.end();

	std::wstring str;
	A2W_s(pOldWord, str);
	bool bBanWord = false;
	for(;iterBegin != iterEnd; ++iterBegin)
	{
		size_t sIndex = str.find(iterBegin->c_str());

		while (sIndex != std::wstring::npos)
		{
			str.replace(sIndex,iterBegin->size(),iterBegin->size(),cReplace);
			sIndex = str.find(iterBegin->c_str(),sIndex+1);
			bBanWord = true;
		}
	}

	if(bBanWord)
	{
		std::string newWord;
		W2A_s(str.c_str(), newWord);
		dStrcpy(pNewWord,uSize,newWord.c_str());
	}
	else
		dStrcpy(pNewWord,uSize,pOldWord);

	return bBanWord;
}

bool FilterOpt::replaceSqlWord(char* pNewWord, U32 uNewSize, const char* pOldWord, U32 uOldSize, const char cReplace)
{
	bool bSqlWord = false;
	U32 i = 0;
	for (; i<uNewSize && i<uOldSize; ++i)
	{
		switch (pOldWord[i])
		{
		case '\'':
		case '\"':
		case ')':
		case '(':
		case '=':
		case '%':
			bSqlWord = true;
			pNewWord[i] = cReplace;
			break;
		default:
			pNewWord[i] = pOldWord[i];
			break;
		}
		if (pOldWord[i] == '\0')
			return bSqlWord;
	}
	if (i < uNewSize)
		pNewWord[i] = '\0';
	else
		pNewWord[uNewSize-1] = '\0';
	return bSqlWord;
}

#if !( defined( WORLDSERVER ) | defined( CHATSERVER ) )
ConsoleFunction(isBanName,bool,2,2,"isBanWord(%name)")
{
	return FilterOpt::isBanName(argv[1]);
}
ConsoleFunction(isBanWord,bool,2,2,"isBanWord(%Word)")
{
	return FilterOpt::isBanWord(argv[1]);
}
ConsoleFunction(replaceBanWord,const char*,2,2,"replaceBanWord(%Word)")
{
	static char s_Buf[4096] = "";
	FilterOpt::replaceBanWord(s_Buf,sizeof(s_Buf),argv[1]);
	return s_Buf;
}

#endif