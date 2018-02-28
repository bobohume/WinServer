#include <algorithm>
#include "BanInfoData.h"
#include "AtlConv.h"

#if !( defined( WORLDSERVER ) | defined( CHATSERVER ) )
#include "Gameplay/Data/readDataFile.h"
#else
#include "DataFile.h"
#define AssertFatal(x,y)
#endif


void CBanInfoRepository::read()
{
    m_vecBanName.clear();
    m_vecBanWord.clear();

	_readBanName();
	_readBanWord();
}

void CBanInfoRepository::clear()
{
	//do nothing
}

void CBanInfoRepository::_readBanName()
{
	CDataFile op;
	RData tempdata;
	op.ReadDataInit();
	char filename[1024] = "";
	sprintf_s( filename,1024, "data/Shield.dat" );

	if(!op.readDataFile(filename))
	{
		op.ReadDataClose();
		SERVER_ASSERT(false,"Cannt read file : Shield.dat");
		return;
	}

	for(int i = 0; i < op.RecordNum; ++i)
	{
		for(int h = 0; h < op.ColumNum; ++h)
		{
			op.GetData(tempdata);
			SERVER_ASSERT(tempdata.m_Type == DType_string,"错误的类型在 Shield.dat");

			std::wstring _str;
			A2W_s(tempdata.m_string,_str);
			m_vecBanName.push_back(_str);
		}
	}

#if !defined( CHATSERVER )
	std::wstring _str;
	A2W_s("\"",_str);
	m_vecBanName.push_back(_str);
	A2W_s(" ",_str);
	m_vecBanName.push_back(_str);
	A2W_s("　",_str);
	m_vecBanName.push_back(_str);
#endif

	op.ReadDataClose();
}

void CBanInfoRepository::_readBanWord()
{
	CDataFile op;
	RData tempdata;
	op.ReadDataInit();
	char filename[1024] = "";
	sprintf_s( filename,1024, "data/Shield.dat" );

	if(!op.readDataFile(filename))
	{
		op.ReadDataClose();
		SERVER_ASSERT(false,"Cannt read file : Shield..dat");
		return;
	}


	for(int i = 0; i < op.RecordNum; ++i)
	{
		for(int h = 0; h < op.ColumNum; ++h)
		{
			op.GetData(tempdata);
			SERVER_ASSERT(tempdata.m_Type == DType_string,"错误的类型在Shield.dat");

			std::wstring _str;
			A2W_s(tempdata.m_string,_str);
			m_vecBanWord.push_back(_str);
		}
	}

	//pa长到短，解决当短字符串被替换后，长字符串无法被替换的问题
	std::stable_sort(m_vecBanWord.begin(), m_vecBanWord.end(), Predicate());


	op.ReadDataClose();
}

CBanInfoRepository& CBanInfoRepository::Instance()
{
	static CBanInfoRepository s_Intance;
	return s_Intance;
}

const CBanInfoRepository::VEC_STR& CBanInfoRepository::getBaneName()const
{
	return m_vecBanName;
}
const CBanInfoRepository::VEC_STR& CBanInfoRepository::getBaneWord()const
{
	return m_vecBanWord;
}

void A2W_s(const char* pInchar,std::wstring& pStr)
{
	USES_CONVERSION;
	UseGB2312;
	pStr = A2W(pInchar);

}

void W2A_s(const wchar_t* pInchar,std::string& pStr)
{
	USES_CONVERSION;
	UseGB2312;
	pStr = W2A(pInchar);
}

