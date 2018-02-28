#ifndef __BASE_DATA__
#define __BASE_DATA__
#include "Common/DataFile.h"
#include "CommLib/CommLib.h"
#include <string.h>
#include <unordered_map>

class IBaseDataRes
{
public:
	IBaseDataRes() : m_FileName("")
	{
	}

	~IBaseDataRes() {}

	virtual bool read() = 0;
	virtual bool reLoad() = 0;
private:
	std::string m_FileName;
};

template<class T>
class CBaseDataRes
{
protected:
	typedef std::unordered_map<S32, T*>					DATA_MAP;
	typedef typename DATA_MAP::iterator					DATA_ITR;
public:
	CBaseDataRes() {};

	virtual ~CBaseDataRes()
	{
		clear();
	}

	void close()
	{
		clear();
	}

	void clear()
	{
		for (DATA_ITR itr = m_DataMap.begin(); itr != m_DataMap.end(); ++itr) {
			SAFE_DELETE(itr->second);
		}
		m_DataMap.clear();
	}
	
	void addData(S32 id, T* pData)
	{
		m_DataMap.insert(DATA_MAP::value_type(id, pData));
	}

	T* getData(S32 Id)
	{
		DATA_ITR itr = m_DataMap.find(Id);
		if (itr != m_DataMap.end()) {
			if (itr->second)
				return itr->second;
			else
				return NULL;

		}
		return NULL;
	}

protected:
	DATA_MAP m_DataMap;
};


template<class T>
class CBaseDataResEx
{
protected:
	typedef std::unordered_multimap<S32, T*>			DATA_MAP;
	typedef typename DATA_MAP::iterator					DATA_ITR;
public:
	CBaseDataResEx() {};

	virtual ~CBaseDataResEx()
	{
		clear();
	}

	void close()
	{
		clear();
	}

	void clear()
	{
		for (DATA_ITR itr = m_DataMap.begin(); itr != m_DataMap.end(); ++itr) {
			SAFE_DELETE(itr->second);
		}
		m_DataMap.clear();
	}

	void addData(S32 id, T* pData)
	{
		m_DataMap.insert(DATA_MAP::value_type(id, pData));
	}

	T* getData(S32 Id)
	{
		DATA_ITR itr = m_DataMap.find(Id);
		if (itr != m_DataMap.end()) {
			if (itr->second)
				return itr->second;
			else
				return NULL;

		}
		return NULL;
	}

protected:
	DATA_MAP m_DataMap;
};
#endif//__BASE_DATA__