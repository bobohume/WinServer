#include "CopyData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"

CCopyData::~CCopyData()
{
	for (GATE_ITR itr = m_GateMap.begin(); itr != m_GateMap.end(); ++itr)
	{
		SAFE_DELETE(itr->second);
	}
	m_GateMap.clear();
}

CCopyDataRes *g_CopyDataMgr =new CCopyDataRes();

CCopyDataRes::CCopyDataRes()
{

}
CCopyDataRes::~CCopyDataRes()
{
	clear();
}

bool CCopyDataRes::read()
{
	{
		CDataFile file;
		RData LineData;

		if (!file.readDataFile("data/Map.dat"))
		{
			JUST_ASSERT("read MapData error");
			return  false;
		}

		for (int i = 0; i < file.RecordNum; ++i)
		{
			CCopyData* pdata = new CCopyData();

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read mapdata ID error");
			pdata->m_CopyID = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_string, "read mapdata name error");
			pdata->m_sName = LineData.m_string;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read mapdata land error");
			pdata->m_Land = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_S32, "read mapdata troops error");
			pdata->m_Troops = LineData.m_S32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_S16, "read mapdata hp error");
			pdata->m_SoldierHp = LineData.m_S16;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_S16, "read mapdata atk error");
			pdata->m_SoldierAtk = LineData.m_S16;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_S8, "read mapdata gatenum error");
			pdata->m_GateNum = LineData.m_S8;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_S32, "read mapdata npcicon error");
			pdata->m_NpcIcon = LineData.m_S32;

			for(int i = 0; i < 4; ++i)
			{
				file.GetData(LineData);
				DOASSERT(LineData.m_Type == DType_S32, "read mapdata awardid error");
				pdata->m_AwardItem[i] = LineData.m_S32;

				file.GetData(LineData);
				DOASSERT(LineData.m_Type == DType_S16, "read mapdata awardnum error");
				pdata->m_AwardNum[i] = LineData.m_S16;
			}

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read mapdata rebellion error");
			pdata->m_Rebellion = LineData.m_U32;

			for (int i = 0; i < 4; ++i)
			{
				file.GetData(LineData);
				DOASSERT(LineData.m_Type == DType_U32, "read mapdata rebellionItem error");
				pdata->m_RebellionItem[i] = LineData.m_U32;
			}
	
			addData(pdata->m_CopyID, pdata);
		}
	}

	{
		CDataFile file;
		RData LineData;

		if (!file.readDataFile("data/Stronghold.dat"))
		{
			JUST_ASSERT("read MapData error");
			return  false;
		}

		for (int i = 0; i < file.RecordNum; ++i)
		{
			CGateData* pdata = new CGateData();

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read Strongholddata gateid error");
			pdata->m_GateID = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U16, "read Strongholddata copyid error");
			pdata->m_CopyID = LineData.m_U16;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read Strongholddata troop error");
			pdata->m_Troops = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U16, "read Strongholddata hp error");
			pdata->m_SoldierHp = LineData.m_U16;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U16, "read Strongholddata atk error");
			pdata->m_SoldierAtk = LineData.m_U16;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read Strongholddata money error");
			pdata->m_AddMoney = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read Strongholddata achievement error");
			pdata->m_AddAchievement = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U16, "read Strongholddata land error");
			pdata->m_AddLand = LineData.m_U16;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U8, "read Strongholddata back error");
			pdata->m_BattleBack = LineData.m_U8;

			m_DataMap[pdata->m_CopyID]->m_GateMap.insert(CCopyData::GATE_MAP::value_type(pdata->m_GateID, pdata));
		}
	}

	return true;
}

CGateData* CCopyDataRes::getGateData(S32 GateId)
{
	U32 CopyId = GateId / 10000;
	CCopyData* pData = getData(CopyId);
	if (pData)
	{
		CCopyData::GATE_MAP& GateMap = pData->m_GateMap;
		for (CCopyData::GATE_ITR itr = GateMap.begin(); itr != GateMap.end(); ++itr)
		{
			if (itr->first == GateId)
				return itr->second;
		}
	}
	return NULL;
}

U32 CCopyDataRes::getNextGateData(S32 GateId)
{
	U32 CopyId = GateId / 10000;
	GateId++;
	if (getGateData(GateId) == NULL)
	{
		GateId = (++CopyId) * 10000 + 1;
		if (getGateData(GateId) != NULL)
		{
			return GateId;
		}
	}
	else
	{
		return GateId;
	}

	return U32_MAX;
}