#ifndef __SKILL_DATA__
#define __SKILL_DATA__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>

#ifndef MACRO_SKILL_ID
#define  MACRO_SKILL_ID(series,level)  (U32)((U32)series *1000 +(U32)level)
#endif

#define	 MACRO_SKILL_SERIES(skillid)   (U32)((U32)skillid / 1000)
#define	 MACRO_SKILL_LEVEL(skillid)    (U32)((U32)skillid % 1000)

const U32 MAX_SKILL_EFFECT = 3;

struct CSkillData;
struct Stats;

class CSkillDataRes : public CBaseDataRes<CSkillData>
{
public:
	CSkillDataRes();
	~CSkillDataRes();

	bool read();
};

struct CSkillData
{
	CSkillData():m_SkillId(0),m_sName(""), m_ResID(0), m_SkillType(0), m_BattleVal(0),\
		m_SucessVal(0)
	{
		memset(m_LevelUpItem, 0, sizeof(m_LevelUpItem));
		memset(m_CostItemNum, 0, sizeof(m_CostItemNum));
		memset(m_BuffId, 0, sizeof(m_BuffId));
	}

	~CSkillData(){};
	
	void getStats(Stats& stats);

	inline bool CanLearn() { return m_SkillType == 1; }

	U32 m_SkillId;
	std::string	m_sName;
	U16	m_ResID;
	U8  m_SkillType;
	U32	m_BuffId[MAX_SKILL_EFFECT];
	U16 m_BattleVal;
	U32 m_LevelUpItem[2];
	S32	m_CostItemNum[2];
	U8	m_SucessVal;
};

struct CTechnologyData
{
	CTechnologyData() : m_ID(0), m_sName(""), m_ResID(0), m_NextID(0), \
		m_Type(0), m_Buff(0), m_Int(0), m_Ore(0), m_Time(0)
	{
	}

	~CTechnologyData() {};

	U32 m_ID;
	std::string m_sName;
	U32 m_ResID;
	U32 m_NextID;
	U32 m_Type;
	U32 m_Buff;
	S32 m_Int;
	S32 m_Ore;
	S32 m_Time;

	void getStats(Stats& stats);
};

class CTechnologyDataRes : public CBaseDataRes<CTechnologyData>
{
public:
	CTechnologyDataRes();
	~CTechnologyDataRes();

	bool read();
};

extern CSkillDataRes* g_SkillDataMgr;
extern CTechnologyDataRes* g_TechnologyDataMgr;
#endif