#include "OfficerData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"

COfficerDataRes* g_OfficerDataMgr = new COfficerDataRes();

COfficerDataRes::COfficerDataRes()
{
}

COfficerDataRes::~COfficerDataRes()
{
	clear();
}

bool COfficerDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/Position.dat"))
	{
		JUST_ASSERT("read positiondat error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		COfficerData* pdata = new COfficerData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read positiondat id error");
		pdata->m_OfficerId = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read positiondat name error");
		pdata->m_sDes = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read positiondat type error");
		pdata->m_OfficerType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read positiondat val error");
		pdata->m_OfficerVal = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read positiondat skill error");
		pdata->m_SkillID = LineData.m_U32;
		pdata->m_SkillID = pdata->m_SkillID * 1000 + 1;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read positiondat  countryflag error");
		pdata->m_CountryFlag = LineData.m_U8;

		addData(pdata->m_OfficerId, pdata);
	}
	return true;
}