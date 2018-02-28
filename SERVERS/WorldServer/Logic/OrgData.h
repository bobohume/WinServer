#ifndef __ORG_DATA__
#define __ORG_DATA__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>

struct COrgExpData;
class COrgExpDataRes : public CBaseDataRes<COrgExpData>
{
	typedef std::unordered_multimap<S32, S32> TASK_TYPE_MAP;
public:
	COrgExpDataRes();
	~COrgExpDataRes();

	bool read();
};

struct COrgExpData
{
	COrgExpData() :m_Level(0), m_Exp(0), m_TotalExp(0)
	{
	}

	~COrgExpData() {};

	S32 m_Level;
	S32 m_Exp;
	S32 m_TotalExp;
};

extern COrgExpDataRes* g_OrgExpDataMgr;
#endif//__ORG_DATA__