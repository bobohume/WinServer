#include "VisitData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"
#include "BuffData.h"

CVisitDataRes* g_VisitDataMgr =new CVisitDataRes();

CVisitDataRes::CVisitDataRes()
{

}
CVisitDataRes::~CVisitDataRes()
{
	clear();
}

bool CVisitDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/Visit.dat"))
	{
		JUST_ASSERT("read VisitDat error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		U32 nSkillSeries = 0;
		U32 nSkillLevel = 0;
		CVisitData* pdata = new CVisitData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Visit cityid error");
		pdata->m_CityId = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Visit citylv error");
		pdata->m_CityLv = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Visit money error");
		pdata->m_Money = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_F32, "read Visit ore error");
		pdata->m_Ore = LineData.m_F32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Visit makeType error");
		pdata->m_MakeType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Visit makeVal error");
		pdata->m_MakeVal = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Visit princessid error");
		pdata->m_PrincessId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Visit randval error");
		pdata->m_RandVal = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Visit dropid error");
		pdata->m_DropId = LineData.m_U32;

		addData(pdata->m_CityId, pdata);
	}
	return true;
}

CVisitData* CVisitDataRes::getData(S32 nCityID, S32 nLevel)
{
	for (auto itr = m_DataMap.lower_bound(nCityID); itr != m_DataMap.upper_bound(nCityID); ++itr)
	{
		if (itr->second && itr->second->m_CityLv == nLevel)
			return itr->second;
	}

	return NULL;
}