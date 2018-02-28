#include "PrincessData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"
#include "Common/mRandom.h"

CPrincessDataRes* g_PrincessDataMgr = new CPrincessDataRes();

CPrincessDataRes::CPrincessDataRes()
{
}

CPrincessDataRes::~CPrincessDataRes()
{
	clear();
}

bool CPrincessDataRes::GetUIDsByHireVal(U32 HireVal, std::vector<U32>& UIDs)
{
	if (HireMap.find(HireVal) != HireMap.end()) {
		UIDs = HireMap[HireVal];
		return true;
	}
	return false;
}

bool CPrincessDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/Princess.dat"))
	{
		JUST_ASSERT("read princessdata error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		CPrincessData* pdata = new CPrincessData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read princessdata id error");
		pdata->m_ID = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read princessdata name error");
		pdata->m_sName = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read princessdata des error");
		pdata->m_sDes = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read princessdata resid error");
		pdata->m_ResID = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read princessdata charm error");
		pdata->m_InitCharm = LineData.m_U16;

		for (auto i = 0; i < 3; ++i)
		{
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read princessdata skill error");
			S32 iSkillID = LineData.m_U32;
			if (iSkillID)
				pdata->m_SkillID[i] = iSkillID * 1000 + 0;
		}

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read princessdata hiretype error");
		pdata->m_HireType = LineData.m_U8;
	
		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read princessdata hrieval error");
		pdata->m_HireVal = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read princessdata position error");
		pdata->m_Pos1 = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read princessdata position error");
		pdata->m_Pos = LineData.m_string;

		addData(pdata->m_ID, pdata);

		if (3 == pdata->m_HireType) {
			if (HireMap.find(pdata->m_HireVal) != HireMap.end()) {
				HireMap[pdata->m_HireVal].push_back(pdata->m_ID);
			}
			else {
				std::vector<U32> uids = { (U32)pdata->m_ID };
				HireMap.insert(std::make_pair(pdata->m_HireVal, uids));
			}
		}
	}
	return true;
}

CFlowerDataRes* g_FlowerDataMgr = new CFlowerDataRes();

CFlowerDataRes::CFlowerDataRes()
{
}

CFlowerDataRes::~CFlowerDataRes()
{
	clear();
}

bool CFlowerDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Flower.dat"))
	{
		JUST_ASSERT("read Flower error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CFlowerData* pdata = new CFlowerData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Flower id error");
		pdata->m_ItemID = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Flower charm error");
		pdata->m_Charm = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Flower time des error");
		pdata->m_FlowerTime = LineData.m_U16;

		addData(pdata->m_ItemID, pdata);
	}
	return true;
}

CPrincessRobotDataRes* g_PrincessRobotDataMgr = new CPrincessRobotDataRes();

CPrincessRobotDataRes::CPrincessRobotDataRes()
{
}

CPrincessRobotDataRes::~CPrincessRobotDataRes()
{
	clear();
}

bool CPrincessRobotDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/FeastRobot.dat"))
	{
		JUST_ASSERT("read FeastRobot error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CPrincessRobotData* pdata = new CPrincessRobotData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read FeastRobot id error");
		pdata->m_Id = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read FeastRobot PrincessId error");
		pdata->m_PrincessId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read FeastRobot charm error");
		pdata->Charm = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read FeastRobot rite error");
		pdata->Rite = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read FeastRobot Culture error");
		pdata->Culture = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read FeastRobot Battle error");
		pdata->Battle = LineData.m_U32;

		addData(pdata->m_Id, pdata);
		m_RobotVec.push_back(pdata);
	}
	return true;
}

CPrincessRobotData* CPrincessRobotDataRes::RandRobot() 
{
	S32 nRandVal = gRandGen.randI(0, m_RobotVec.size() - 1);
	nRandVal = mClamp(nRandVal, 0, m_RobotVec.size() - 1);
	return m_RobotVec[nRandVal];
}