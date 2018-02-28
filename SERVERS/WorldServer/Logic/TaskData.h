#ifndef __TASK_DATA__
#define __TASK_DATA__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>
#include <set>

struct CTaskData;
class CTaskDataRes : public CBaseDataRes<CTaskData>
{
	typedef std::unordered_multimap<S32, S32> TASK_TYPE_MAP;
public:
	CTaskDataRes();
	~CTaskDataRes();

	bool read();
	std::set<S32> GetTaskIdByType(S32 nType);
	TASK_TYPE_MAP m_TypeTaskMap;
};

struct CTaskData
{
	CTaskData() :m_TaskId(0), m_TaskName(""), m_TaskDes(""), m_Type(0), m_NextTaskId(0), \
		m_Finish(0), m_FinishVal(0)
	{
		memset(m_AwardType, 0, sizeof(m_AwardType));
		memset(m_AwardVal, 0, sizeof(m_AwardVal));
	}

	~CTaskData() {};

	S32 m_TaskId;
	std::string m_TaskName;
	std::string m_TaskDes;
	S8	m_Type;
	S32	m_NextTaskId;
	S8  m_Finish;
	S32	m_FinishVal;
	S32 m_AwardType[3];
	S32 m_AwardVal[3];
};

extern CTaskDataRes* g_TaskDataMgr;
#endif