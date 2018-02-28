#include "BossData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"
#include "BuffData.h"

CBossDataRes* g_BossDataMgr = new CBossDataRes();

CBossDataRes::CBossDataRes()
{

}

CBossDataRes::~CBossDataRes()
{
	clear();
}

bool CBossDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/Boss.dat"))
	{
		JUST_ASSERT("read LeagueExp error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		CBossData* pdata = new CBossData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Boss id error");
		pdata->m_Id = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read Boss name error");
		pdata->m_Name = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Boss level error");
		pdata->m_Level = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Boss cost error");
		pdata->m_Cost = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Boss hp error");
		pdata->m_Hp = S64(LineData.m_U16) * 100*1000*1000;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Boss award error");
		pdata->m_Award = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Boss money error");
		pdata->m_Money = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Boss ore error");
		pdata->m_Ore = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Boss troops error");
		pdata->m_Troops = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Boss award error");
		pdata->m_Award1 = LineData.m_U32;

		for (auto i = 0; i < 4; ++i) {
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read Boss atkaward error");
			pdata->m_AtkItem[i] = LineData.m_U32;
		}

		addData(pdata->m_Id, pdata);
	}
	return true;
}