//
//  NpcData.cpp
//  HelloLua
//
//  Created by BaoGuo on 14-1-25.
//
//
#include "MissionData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"
#include <sstream>

CMissionDataRes *g_MissionDataMgr = new CMissionDataRes();

CMissionDataRes::CMissionDataRes()
{

}

CMissionDataRes::~CMissionDataRes()
{
	clear();
}

bool CMissionDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/Data_7_Mission.dat"))
	{
		assert("read NpcData error");
		return  false;
	}


	for(int i = 0; i < file.RecordNum; ++i)
	{
		CMissionData* pdata = new CMissionData();
		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata ID error");
		pdata->m_MissionId = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_enum16, "read missiondata category error");
		pdata->m_Category = LineData.m_Enum16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_enum16, "read missiondata subcategory error");
		pdata->m_SubCategory = LineData.m_Enum16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata preMissionId error");
		pdata->m_PreMissionId = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read missiondata name error");
		pdata->m_MissionName = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata receiveLv error");
		pdata->m_RecLevel = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata submitLv error");
		pdata->m_SubLevel = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata weekday error");
		pdata->m_WeekDay = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read missiondata begintime error");
		pdata->m_BeginTime = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata lastTime error");
		pdata->m_LastTime = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata revNpcId error");
		pdata->m_RecNpcId = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata subNpcId error");
		pdata->m_SubNpcId = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_enum16, "read missiondata type error");
		pdata->m_MissionType = LineData.m_Enum16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata target error");
		pdata->m_TargetId = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata neednum error");
		pdata->m_NeedNum = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata exp error");
		pdata->m_AwardExp = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata money error");
		pdata->m_AwardMoney = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata gold error");
		pdata->m_AwardGold = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata points error");
		pdata->m_Points = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata item error");
		pdata->m_AwardItemId = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_S32, "read missiondata points error");
		pdata->m_AwardScore = LineData.m_S32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_enum16, "read missiondata interval error");
		pdata->m_Interval = LineData.m_Enum16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read missiondata desc error");
		pdata->m_MissionDesc = LineData.m_string;

		m_MissionDataMap.insert(MISSIONDATAMAP::value_type(pdata->m_MissionId, pdata));
		//主线任务有前置任务
		if((pdata->m_Category == CMissionData::MISSION_CATEGORY_PLOT) && (pdata->m_PreMissionId > 0))
		{
			m_MissisonIdMap.insert(MISSIONIDMAP::value_type(pdata->m_PreMissionId, pdata->m_MissionId));
		}
	}
	return true;
}


CMissionData* CMissionDataRes::getMissionData(S32 iMissionId)
{
	if(iMissionId < MISSION_BEGIN_ID || iMissionId > MISSION_END_ID)
		return NULL;

	MISSIONDATAITR itr = m_MissionDataMap.find(iMissionId);
	if(itr != m_MissionDataMap.end())
	{
		if(itr->second)
			return itr->second;
		else
			return NULL;
	}

	return NULL;
}

CMissionData* CMissionDataRes::getMissionDataEx(S32 iMissionId)
{
	if(!g_MissionDataMgr)
		return NULL;

	return g_MissionDataMgr->getMissionData(iMissionId);
}

bool CMissionDataRes::getNextMissionId(S32 iPreMissionId, std::set<S32>& MissionSet)
{
	MISSIONIDITR lowItr = m_MissisonIdMap.lower_bound(iPreMissionId);
	MISSIONIDITR upItr	= m_MissisonIdMap.upper_bound(iPreMissionId);

	for(MISSIONIDITR itr = lowItr; itr != upItr; ++itr)
	{
		MissionSet.insert(itr->second);
	}

	return true;
}

void CMissionDataRes::clear()
{
	for(MISSIONDATAITR itr = m_MissionDataMap.begin(); itr != m_MissionDataMap.end(); ++itr)
	{
		SAFE_DELETE(itr->second);
	}
	m_MissionDataMap.clear();
	m_MissisonIdMap.clear();
}

void CMissionDataRes::close()
{
	clear();
}

void CMissionData::printAllEmlem()
{
	printf("----------CMissionData---------------\n");
	printf("m_MissionId : %d, m_Category : %d, m_SubCategory : %d, m_PreMissionId : %d, m_MissionName : %s\n", m_MissionId, m_Category, m_SubCategory, m_PreMissionId, m_MissionName.c_str());
	printf("m_RecLevel : %d,  m_SubLevel : %d, m_WeekDay : %d,     m_BeginTime : %s,    m_LastTime : %d\n", m_RecLevel, m_SubLevel, m_WeekDay, m_BeginTime.c_str(), m_LastTime);
	printf("m_RecNpcId : %d,  m_SubNpcId : %d, m_MissionType : %d, m_TargetId  : %d,    m_NeedNum : %d\n", m_RecNpcId, m_SubNpcId, m_MissionType, m_TargetId, m_NeedNum);
	printf("m_AwardExp : %d,  m_AwardMoney : %d, m_AwardGold : %d, m_AwardItemId  : %d, m_Interval : %d\n", m_AwardExp, m_AwardMoney, m_AwardGold, m_AwardItemId, m_Interval);
	printf("m_Points:%d  ,m_AwardScore:%d",m_Points,m_AwardScore);
	printf("----------CMissionData---------------\n");
}
