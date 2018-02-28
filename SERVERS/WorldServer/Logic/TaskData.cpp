#include "TaskData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"
#include "BuffData.h"

CTaskDataRes* g_TaskDataMgr =new CTaskDataRes();

CTaskDataRes::CTaskDataRes()
{

}
CTaskDataRes::~CTaskDataRes()
{
	clear();
}

bool CTaskDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/Task.dat"))
	{
		JUST_ASSERT("read TaskDat error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		CTaskData* pdata = new CTaskData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Task taskid error");
		pdata->m_TaskId = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read Task name error");
		pdata->m_TaskName = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read Task des error");
		pdata->m_TaskDes = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Task type error");
		pdata->m_Type = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Task m_NextTaskId error");
		pdata->m_NextTaskId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Task finish error");
		pdata->m_Finish = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Task finishval error");
		pdata->m_FinishVal = LineData.m_U32;

		for (auto i = 0; i < 3; ++i) {
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read Task awardtype error");
			pdata->m_AwardType[i] = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read Task awardval error");
			pdata->m_AwardVal[i] = LineData.m_U32;
		}

		m_TypeTaskMap.insert(TASK_TYPE_MAP::value_type(pdata->m_Finish, pdata->m_TaskId));
		addData(pdata->m_TaskId, pdata);
	}
	return true;
}

std::set<S32> CTaskDataRes::GetTaskIdByType(S32 nType) {
	std::set<S32> tempSet;
	for (auto itr = g_TaskDataMgr->m_TypeTaskMap.lower_bound(nType); itr != g_TaskDataMgr->m_TypeTaskMap.upper_bound(nType); ++itr)
	{
		tempSet.insert(itr->second);
	}
	return tempSet;
}