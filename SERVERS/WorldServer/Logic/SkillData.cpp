#include "SkillData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"
#include "BuffData.h"

 CSkillDataRes* g_SkillDataMgr =new CSkillDataRes();

 CSkillDataRes::CSkillDataRes()
{

}
 CSkillDataRes::~CSkillDataRes()
{
	clear();
}

bool CSkillDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/Skill.dat"))
	{
		JUST_ASSERT("read itemData error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		U32 nSkillSeries = 0;
		U32 nSkillLevel = 0;
		CSkillData* pdata = new CSkillData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read skilldata series error");
		nSkillSeries = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read skilldata level error");
		nSkillLevel = LineData.m_U8;
		pdata->m_SkillId = MACRO_SKILL_ID(nSkillSeries, nSkillLevel);

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read skilldata name error");
		pdata->m_sName= LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read skilldata res error");
		pdata->m_ResID = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read skilldata type error");
		pdata->m_SkillType = LineData.m_U8;

		for (int i = 0; i < MAX_SKILL_EFFECT; ++i)
		{
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read itemdata buff error");
			pdata->m_BuffId[i] = LineData.m_U32;
		}

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read skilldata battle error");
		pdata->m_BattleVal = LineData.m_U16;

		for (int i = 0; i < 2; ++i)
		{
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read skilldata itemid error");
			pdata->m_LevelUpItem[i] = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read skilldata costnum error");
			pdata->m_CostItemNum[i] = LineData.m_U32;
		}

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read skilldata scuess error");
		pdata->m_SucessVal = LineData.m_U8;

		addData(pdata->m_SkillId, pdata);
	}
	return true;
}

void CSkillData::getStats(Stats& stats)
{
	for (int i = 0; i < MAX_SKILL_EFFECT; ++i)
	{
		if (m_BuffId[i])
		{
			CBuffData* pBuffData = g_BuffDataMgr->getData(m_BuffId[i]);
			if (pBuffData)
			{
				pBuffData->getStats(stats);
			}
		}
	}
}

CTechnologyDataRes* g_TechnologyDataMgr = new CTechnologyDataRes();
CTechnologyDataRes::CTechnologyDataRes()
{
}
CTechnologyDataRes::~CTechnologyDataRes()
{
	clear();
}

bool CTechnologyDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Tech.dat"))
	{
		JUST_ASSERT("read techData error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		U32 nSeries = 0;
		U32 nLevel = 0;
		CTechnologyData* pdata = new CTechnologyData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read techdat series error");
		nSeries = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read techdat level error");
		nLevel = LineData.m_U8;
		pdata->m_ID = MACRO_SKILL_ID(nSeries, nLevel);

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read techdat name error");
		pdata->m_sName = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read techdat res error");
		pdata->m_ResID = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read techdat preid error");
		pdata->m_NextID = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read techdat type error");
		pdata->m_Type = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read techdat buff error");
		pdata->m_Buff = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read techdat int error");
		pdata->m_Int = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read techdat ore error");
		pdata->m_Ore = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read techdat time error");
		pdata->m_Time = LineData.m_U16;

		addData(pdata->m_ID, pdata);
	}
	return true;
}

void CTechnologyData::getStats(Stats& stats)
{
	CBuffData* pBuffData = g_BuffDataMgr->getData(m_Buff);
	if (pBuffData)
	{
		pBuffData->getStats(stats);
	}
}
