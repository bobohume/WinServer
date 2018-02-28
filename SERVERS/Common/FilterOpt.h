#ifndef _FILTEROPT_H
#define _FILTEROPT_H
#pragma  once

#if !( defined( WORLDSERVER ) | defined( CHATSERVER ) )
#include "platform/types.h"
#else
#include "base/types.h"
#endif

struct FilterOpt
{
	static bool isBanName(const char* pName);
	static bool isBanWord(const char* pWord);
	//返回值表示是否banword
	static bool replaceBanWord(char* pNewWord,U32 uSize,const char* pOldWord,const char cReplace = '*');
	static bool replaceSqlWord(char* pNewWord, U32 uNewSize, const char* pOldWord, U32 uOldSize, const char cReplace='*');
};
#endif