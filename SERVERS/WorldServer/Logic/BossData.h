#ifndef __BOSS_DATA__
#define __BOSS_DATA__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>

struct CBossData;
class CBossDataRes : public CBaseDataRes<CBossData>
{
	typedef std::unordered_multimap<S32, S32> TASK_TYPE_MAP;
public:
	CBossDataRes();
	~CBossDataRes();

	bool read();
};

struct CBossData
{
	CBossData() :m_Id(0), m_Name(""), m_Level(0), m_Cost(0), m_Hp(0),
		m_Award(0), m_Money(0), m_Ore(0), m_Troops(0), m_Award1(0)
	{
		memset(m_AtkItem, 0, sizeof(m_AtkItem));
	}

	~CBossData() {};

	S32 m_Id;
	std::string m_Name;
	S32 m_Level;
	S32 m_Cost;
	S64 m_Hp;
	S32 m_Award;
	S32 m_Money;
	S32 m_Ore;
	S32 m_Troops;
	S32 m_Award1;
	S32 m_AtkItem[4];
};

extern CBossDataRes* g_BossDataMgr;
#endif//__BOSS_DATA__