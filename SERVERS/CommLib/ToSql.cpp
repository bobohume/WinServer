#include "ToSql.h"

ISqlUnit::ISqlUnit()
{
}

ISqlUnit::~ISqlUnit()
{
}


CSqlUnitMgr::CSqlUnitMgr()
{
}

CSqlUnitMgr::~CSqlUnitMgr()
{
	m_SqlUnitMap.clear();
}

CSqlUnitMgr* CSqlUnitMgr::Instance()
{
	static CSqlUnitMgr s_LocalMgr;
	return &s_LocalMgr;
}

void CSqlUnitMgr::RegisterUnit(std::string sName, ISqlUnit* pRedisUnit)
{
	m_SqlUnitMap.insert(SQLUNIT_MAP::value_type(sName, pRedisUnit));
}

ISqlUnit* CSqlUnitMgr::GetUnit(std::string sName)
{
	auto itr = m_SqlUnitMap.find(sName);
	if (itr != m_SqlUnitMap.end())
	{
		return itr->second;
	}

	return NULL;
}