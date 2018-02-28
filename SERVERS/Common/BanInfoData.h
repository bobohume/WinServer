#ifndef _BANINFODATA_H
#define _BANINFODATA_H
#pragma once

#include <vector>
#include <hash_map>
#if !( defined( WORLDSERVER ) | defined( CHATSERVER ) )
#include "platform\types.h"
#else
#include "base/types.h"
#endif

//配合 USES_CONVERSION 使用
//改變Code Page 為Gb2312
#define UseGB2312 _acp = 936

void A2W_s(const char* pInchar,std::wstring& pStr);
void W2A_s(const wchar_t* pInchar,std::string& pStr);

class CBanInfoRepository
{
public:
	typedef	std::vector<std::wstring>					 VEC_STR;
	typedef std::vector<std::wstring>::const_iterator    VEC_STR_CONITER;

	const VEC_STR& getBaneName()const;
	const VEC_STR& getBaneWord()const;
	static CBanInfoRepository& Instance();
	void read();
	void clear();
private:
	void _readBanName();
	void _readBanWord();
	VEC_STR m_vecBanName;
	VEC_STR m_vecBanWord;
};

class Predicate
	:public std::binary_function<std::wstring, std::wstring, bool>
{
public:
	bool operator()(const std::wstring& left, const std::wstring& right) const
	{
		return (left.size() > right.size());
	}
};

#define g_BanInfoRepository CBanInfoRepository::Instance()

#endif