#include "CardData.h"
#include "CommLib/CommLib.h"
#include "Common/OrgBase.h"

CCardDataRes *g_CardDataMgr = new CCardDataRes();

CCardDataRes::CCardDataRes()
{

}
CCardDataRes::~CCardDataRes()
{
	clear();
}

bool CCardDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Minister.dat"))
	{
		JUST_ASSERT("read Minister.data error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		U32 nSkillSeries = 0;
		U16 nSkillLevel  = 0;
		CCardData* pdata = new CCardData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read carddata ID error");
		pdata->m_CardID = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read carddata name error");
		pdata->m_sName = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read carddata des error");
		pdata->m_sDes = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read carddata color error");
		pdata->m_Color = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read carddata type error");
		pdata->m_Icon = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read carddata InitAgi error");
		pdata->m_InitAgi = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read carddata InitInt error");
		pdata->m_InitInt = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read carddata InitStR error");
		pdata->m_InitStr = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read carddata agigrow error");
		pdata->m_InitAgiGrow = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read carddata intgrow error");
		pdata->m_InitIntGrow = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read carddata strgrow error");
		pdata->m_InitStrGrow = LineData.m_U16;

		for (int i = 0; i < MAX_INIT_SKILL; ++i)
		{
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read carddata skillseries error");
			nSkillSeries = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U16, "read carddata skilllevel error");
			nSkillLevel = LineData.m_U16;
			pdata->m_SkillID[i] = MACRO_SKILL_ID(nSkillSeries, nSkillLevel);
		}

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read carddata hiretype error");
		pdata->m_HireType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read carddata hireval error");
		pdata->m_HireVal = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read carddata cardpos error");
		pdata->m_Pos = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read carddata applypos error");
		pdata->m_ApplyPos = LineData.m_U8;
	
		addData(pdata->m_CardID, pdata);
	}
	return true;
}

//------------------------------------------------------------
//					卡牌技能升级data
//------------------------------------------------------------
CExpDataRes* g_ExpDataMgr = new CExpDataRes();
CExpDataRes::CExpDataRes()
{

}
CExpDataRes::~CExpDataRes()
{
	clear();
}

bool CExpDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Exp.dat"))
	{
		JUST_ASSERT("read Exp.data error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		U32 nColor = 0;
		U32 nLevel = 0;
		CExpData* pdata = new CExpData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read expdata color ID error");
		nColor = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read expdata level error");
		nLevel = LineData.m_U16;

		pdata->m_Id = MACRO_EXP_ID(nColor, nLevel);

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read expdata costmoney error");
		pdata->m_CostMoney = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read expdata totalmoney error");
		pdata->m_TotalMoney = LineData.m_S32;

		addData(pdata->m_Id, pdata);
	}
	return true;
}


//------------------------------------------------------------
//					奏章data
//------------------------------------------------------------
CApplyDataRes* g_ApplyDataMgr = new CApplyDataRes();
CApplyDataRes::CApplyDataRes()
{

}
CApplyDataRes::~CApplyDataRes()
{
}

bool CApplyDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Apply.dat"))
	{
		JUST_ASSERT("read Apply.data error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CApplyData* pdata = new CApplyData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read applydata ID error");
		pdata->m_Id = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read applydata 奏章描述 error");
		pdata->m_Des = LineData.m_string;

		for (auto i = 0; i < 2; ++i)
		{
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U8, "read applydata 执行效果 error");
			pdata->m_Type[i] = LineData.m_U8;
		}

		addData(pdata->m_Id, pdata);
	}
	return true;
}


//------------------------------------------------------------
//					等级上限data
//------------------------------------------------------------
CLvLimitDataRes* g_LvLimitDataMgr = new CLvLimitDataRes();
CLvLimitDataRes::CLvLimitDataRes()
{

}
CLvLimitDataRes::~CLvLimitDataRes()
{
}

bool CLvLimitDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Peerage.dat"))
	{
		JUST_ASSERT("read Peerage.data error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CLvLimitData* pdata = new CLvLimitData();
		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Peerage ID error");
		pdata->m_Id = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read Peerage Name error");
		pdata->m_Des = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Peerage Limit error");
		pdata->m_LimitLv = LineData.m_U16;

		for (auto i = 0; i < 3; ++i)
		{
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read Peerage Item error");
			pdata->m_ItemId[i] = LineData.m_U32;
		}

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Peerage award error");
		pdata->m_AddV = LineData.m_U8;

		addData(pdata->m_Id, pdata);
	}
	return true;
}

//------------------------------------------------------------
//					大臣缘分data
//------------------------------------------------------------
CFateDataRes* g_FateDataMgr = new CFateDataRes();
CFateDataRes::CFateDataRes()
{

}
CFateDataRes::~CFateDataRes()
{
	m_DataVec.clear();
}

bool CFateDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Fate.dat"))
	{
		JUST_ASSERT("read Fate.data error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CFateData* pdata = new CFateData();
		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Fate ID error");
		pdata->m_Id = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read Fate Name error");
		pdata->m_Name = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Fate Type error");
		pdata->m_Type = LineData.m_U8;

		{
			std::string str;
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_string, "read Fate Name error");
			str = LineData.m_string;
			char* tokenPtr = strtok((char*)str.c_str(), ";");
			while(tokenPtr != NULL)
			{
				pdata->m_CardVec.push_back(atoi(tokenPtr));
				tokenPtr = strtok(NULL, ";");
			}
		}

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Fate addtype error");
		pdata->m_AddType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Fate addval error");
		pdata->m_AddVal= LineData.m_U8;

		addData(pdata->m_Id, pdata);
		m_DataVec.push_back(pdata);
	}
	return true;
}

std::vector<CFateData*>& CFateDataRes::getFateVec()
{
	return m_DataVec;
}