#ifndef __VISIT_DATA__
#define __VISIT_DATA__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>

struct CVisitData;
class CVisitDataRes : public CBaseDataResEx<CVisitData>
{
public:
	CVisitDataRes();
	~CVisitDataRes();

	bool read();

	CVisitData* getData(S32 nCityID, S32 nLevel);
};

struct CVisitData
{
	CVisitData() :m_CityId(0), m_CityLv(0), m_Money(0), m_Ore(0.0f), m_MakeType(0), \
		m_MakeVal(0), m_PrincessId(0), m_RandVal(0), m_DropId(0)
	{
	}

	~CVisitData() {};

	S32 m_CityId;
	S8	m_CityLv;
	S32	m_Money;
	F32 m_Ore;
	S8	m_MakeType;
	S32 m_MakeVal;
	S32 m_PrincessId;
	S8	m_RandVal;
	S32	m_DropId;

};

extern CVisitDataRes* g_VisitDataMgr;
#endif