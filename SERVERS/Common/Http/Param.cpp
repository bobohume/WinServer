#include "Param.h"
#include <assert.h>
#include "Common/UtilString.h"
#include "CommLib/CommLib.h"

CParam::CParam()
{
}

CParam::~CParam()
{
}

void CParam::AddParam(const char* pKey, const char* pValue)
{
	if (pKey==NULL || pValue==NULL)
	{
		assert(0);
		return;
    }
	
	char* pUtf8	= Util::MbcsToUtf8(pValue);	
	m_mapParam.insert(std::make_pair(pKey, pUtf8));
	SAFE_DELETE_ARRAY(pUtf8);
}

char* CParam::GetParam(const char* pKey)
{
	if (!pKey)
	{
		assert(0);
	    return NULL;
	}

	MapKey2Value::iterator it = m_mapParam.find(pKey);
	if (it == m_mapParam.end())
		return NULL;

	char* pRetBuffer = new char[it->second.length()+1];
	strncpy_s(pRetBuffer, it->second.length()+1, it->second.c_str(), it->second.length());
	return pRetBuffer;
}

void CParam::AddPicNameParam(const char* pKey, const char* pValue)
{
	if (!pKey || !pValue)
	{
		assert(0);
	    return;
	}

	m_mapPicParam[pKey] = pValue;
}

char* CParam::GetPicNameParam(const char* pKey)
{
	if (!pKey)
	{
		assert(0);
	    return NULL;
	}

	MapKey2Value::iterator it = m_mapPicParam.find(pKey);
	if (it == m_mapPicParam.end())
		return NULL;

	char* pRetBuffer = new char[it->second.length()+1];
	strncpy_s(pRetBuffer, it->second.length()+1, it->second.c_str(), it->second.length());
	return pRetBuffer;
}

char* CParam::GetUrlParamString()
{
	std::string strResult;
	for (MapKey2Value::iterator it=m_mapParam.begin(); it!=m_mapParam.end(); ++it)
	{
		if (it != m_mapParam.begin())
			strResult += "&";

		strResult += it->first;
		strResult += "=";
		strResult += it->second;
	}

	char* pRetBuffer = new char[strResult.length()+1];
	strncpy_s(pRetBuffer, strResult.length()+1, strResult.c_str(), strResult.length());
	return pRetBuffer;
}

void CParam::Clear()
{
	m_mapParam.clear();
	m_mapPicParam.clear();
}

CParam::MapKey2Value& CParam::GetParamMap()
{
	return m_mapParam;
}

CParam::MapKey2Value& CParam::GetPicParamMap()
{
	return m_mapPicParam;
}