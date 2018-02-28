#include "OrgData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"
#include "BuffData.h"

COrgExpDataRes* g_OrgExpDataMgr = new COrgExpDataRes();

COrgExpDataRes::COrgExpDataRes()
{

}

COrgExpDataRes::~COrgExpDataRes()
{
	clear();
}

bool COrgExpDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/LeagueExp.dat"))
	{
		JUST_ASSERT("read LeagueExp error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		COrgExpData* pdata = new COrgExpData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read LeagueExp level error");
		pdata->m_Level = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read LeagueExp exp error");
		pdata->m_Exp = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read LeagueExp toalexp error");
		pdata->m_TotalExp = LineData.m_S32;

		addData(pdata->m_Level, pdata);
	}
	return true;
}