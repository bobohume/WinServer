#include "PvpData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"
#include "Common/mRandom.h"

CPvpDataRes* g_PvpDataMgr = new CPvpDataRes();
CPvpBuffDataRes* g_PvpBuffDataMgr = new CPvpBuffDataRes();
CTopAwardDataRes* g_TopAwardDataMgr = new CTopAwardDataRes();

CPvpDataRes::CPvpDataRes()
{
}

CPvpDataRes::~CPvpDataRes()
{
	clear();
}

bool CPvpDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/Robot.dat"))
	{
		JUST_ASSERT("read Robotdata error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		CPvpData* pdata = new CPvpData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read pvpdata id error");
		pdata->m_ID = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read pvpdata name error");
		pdata->m_sName = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read pvpdata score error");
		pdata->m_Score = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read pvpdata cardid error");
		pdata->m_CardID = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read pvpdata agi error");
		pdata->m_Agi = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read pvpdata int error");
		pdata->m_Int = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read pvpdata str error");
		pdata->m_Str = LineData.m_U32;

		addData(pdata->m_ID, pdata);
		m_PvpMap.insert(PVPMAP::value_type(pdata->m_ID, pdata));
	}
	return true;
}

std::vector<CPvpData*> CPvpDataRes::GetPvpData(S32 Id)
{
	std::vector<CPvpData*> Vec;
	auto LowItr = m_DataMap.lower_bound(Id);
	auto UpItr = m_DataMap.upper_bound(Id);
	for (auto itr = LowItr; itr != UpItr; ++itr)
	{
		Vec.push_back(itr->second);
	}
	return Vec;
}

//-----------------------------------------------------------------------------
//								Õ½¶·buff
//-----------------------------------------------------------------------------
CPvpBuffDataRes::CPvpBuffDataRes()
{
}

CPvpBuffDataRes::~CPvpBuffDataRes()
{
	clear();
}

bool CPvpBuffDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/BattleBuff.dat"))
	{
		JUST_ASSERT("read BattleBuff error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CPvpBuffData* pdata = new CPvpBuffData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read BattleBuff id error");
		pdata->m_ID = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read BattleBuff name error");
		pdata->m_sName = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read BattleBuff bufflv error");
		pdata->m_BuffLv = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read BattleBuff bufftype error");
		pdata->m_BuffType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read BattleBuff buff error");
		pdata->m_BuffID = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read BattleBuff type error");
		pdata->m_CostType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read BattleBuff num error");
		pdata->m_CostNum = LineData.m_U16;

		addData(pdata->m_ID, pdata);
		m_BuffTypeVec[pdata->m_BuffType].push_back(pdata);
	}
	return true;
}

S8	CPvpBuffDataRes::randomBuff(S32 nType)
{
	nType = mClamp(nType, 1, 4);
	S32 nRandVal = gRandGen.randI(0, m_BuffTypeVec[nType].size() - 1);
	CPvpBuffData* pData = m_BuffTypeVec[nType][nRandVal];
	if (pData)
	{
		return pData->m_ID;
	}

	return -1;
}

//----------------------------------------------------------------------
//							ÅÅÐÐ°ñ½±Àø
//
//----------------------------------------------------------------------
CTopAwardDataRes::CTopAwardDataRes()
{
}

CTopAwardDataRes::~CTopAwardDataRes()
{
	clear();
}

bool CTopAwardDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/RankingReward.dat"))
	{
		JUST_ASSERT("read RankingReward error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CTopAwardData* pdata = new CTopAwardData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read RankingReward id error");
		pdata->m_ID = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read RankingReward rankbegin error");
		pdata->m_Begin = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read RankingReward rankend error");
		pdata->m_End = LineData.m_U32;

		for (auto i = 0; i < 4; ++i)
		{
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read RankingReward awarditem error");
			pdata->m_Award[i] = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U16, "read RankingReward awardnum error");
			pdata->m_AwardNum[i] = LineData.m_U16;
		}
	
		addData(pdata->m_ID, pdata);
	}
	return true;
}

CTopAwardData* CTopAwardDataRes::getTopRand(S32 nRank)
{
	for (auto itr = m_DataMap.begin(); itr != m_DataMap.end(); ++itr) 
	{
		if (itr->second) 
		{
			if (nRank >= itr->second->m_Begin && nRank <= itr->second->m_End) 
			{
				return itr->second;
			}
		}
	}

	return NULL;
}

