//
//  NpcData.cpp
//  HelloLua
//
//  Created by BaoGuo on 14-1-25.
//
//

#include "NpcData.h"
#include <sstream>
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"

CNpcDataRes *g_NpcDataMgr = new CNpcDataRes();

CNpcDataRes::CNpcDataRes()
{

}

CNpcDataRes::~CNpcDataRes()
{
	clear();
}

bool CNpcDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("Data_2_Monster.dat"))
	{
		JUST_ASSERT("read NpcData error");
		return  false;
	}


	for(int i = 0; i < file.RecordNum; ++i)
	{
		CNpcData* pdata = new CNpcData();
		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read npcdata ID error");
		pdata->m_NpcId = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_string, "read npcdata Name error");
		pdata->m_sName = LineData.m_string;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_string, "read npcdata Icon error");
		pdata->m_sIcon = LineData.m_string;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_enum16, "read npcdata Family error");
		pdata->m_Family = LineData.m_Enum16;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_enum16, "read npcdata Type error");
		pdata->m_Force = LineData.m_Enum16;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_enum16, "read npcdata Str error");
		pdata->m_Difficulty = LineData.m_Enum16;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_enum16, "read npcdata Limit error");
		pdata->m_Limit = LineData.m_Enum16;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_F32, "read npcdata Atk error");
		pdata->m_BaseAtk_Pc = LineData.m_F32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_F32, "read npcdata Def error");
		pdata->m_BaseDef_Pc = LineData.m_F32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_F32, "read npcdata HP error");
		pdata->m_BaseHp_Pc = LineData.m_F32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_F32, "read npcdata Doe error");
		pdata->m_BaseDoe_Pc = LineData.m_F32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_F32, "read npcdata Cri error");
		pdata->m_BaseCri_Pc = LineData.m_F32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_F32, "read npcdata Spe error");
		pdata->m_BaseSpe_Pc = LineData.m_F32;

		for(int i = 0; i < CNpcData::MAX_NPC_BUFFSIZE; ++i)
		{
			std::ostringstream ostr;
			ostr<<"read npcdata buff"
				<<i
				<<" error";

			file.GetData(LineData);
			ASSERT(LineData.m_Type == DType_S32, ostr.str().c_str());
			pdata->m_BaseBuff[i] = LineData.m_S32;
		}

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read npcdata 技能目标 error");
		pdata->m_EffectTarget = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read npcdata 技能ID error");
		pdata->m_SkillId = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read npcdata 技能CD error");
		pdata->m_SkillCd = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read npcdata 技能释放距离 error");
		pdata->m_SkillRange = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read npcdata see error");
		pdata->m_See = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read npcdata drop error");
		pdata->m_Drop = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read npcdata pro error");
		pdata->m_DropVal = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read npcdata active error");
		pdata->m_Active = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_enum16, "read npcdata char error");
		pdata->m_Charater = LineData.m_Enum16;

		m_DataMap.insert(NPCDATAMAP::value_type(pdata->m_NpcId, pdata));
		//只记录等阶1的怪物
		if((pdata->m_NpcId % 10 == 1) && (pdata->m_Limit == LIMIT_COMMBAT))
		{
			m_NpcVec[pdata->m_Family].push_back(pdata->m_NpcId);
		}
	}

	return true;
}


CNpcData* CNpcDataRes::getNpcData(S32 NpcId)
{
	if(NpcId < NPC_BEGIN_ID || NpcId > NPC_END_ID)
		return NULL;

	NPCDATAITR itr = m_DataMap.find(NpcId);
	if(itr != m_DataMap.end())
	{
		if(itr->second)
			return itr->second;
		else
			return NULL;
	}

	return NULL;
}


void CNpcDataRes::clear()
{
	for(NPCDATAITR itr = m_DataMap.begin(); itr != m_DataMap.end(); ++itr)
	{
		SAFE_DELETE(itr->second);
	}
	m_DataMap.clear();

	for(int i = 0; i < Family_Max; ++i)
	{
		m_NpcVec[i].clear();
	}
}

void CNpcDataRes::close()
{
	clear();
}