//
//  NpcData.h
//  HelloLua
//
//  Created by BaoGuo on 14-1-25.
//
//

#ifndef HelloLua_NpcData_h
#define HelloLua_NpcData_h
#include "Common/DataFile.h"
#include <string.h>
#include <hash_map>
#include "Common/PlayerStruct.h"

enum enMasterGroup
{
	GROUP_SHENG     = BIT(0),//Family_Sheng
	GROUP_FO        = BIT(1),//Family_Fo
	GROUP_XIAN      = BIT(2),//Family_Xian
	GROUP_JING      = BIT(3),
	GROUP_GUI       = BIT(4),
	GROUP_GUAI      = BIT(5),
	GROUP_YAO       = BIT(6),
	GROUP_MO        = BIT(7),
	GROUP_SHOU      = BIT(8),
	GROUP_LONG      = BIT(9),
};

enum enForce
{
	Force_None        = 0,
	Force_Peace,
	Force_1,
	Force_2,
	Force_3,
	Force_4,
	Force_5,
	Force_6,
	Force_7,
	Force_8,
	Force_9,
	Force_Max,
};

enum enMonsterDiff
{
	Monster_None = 0,
	Monster_Normal1,
	Monster_Normal2,
	Monster_Elite,
	Monster_Boss,
};

enum enLimitType
{
	LIMIT_INTERACTIVE       = 0,
	LIMIT_COMMBAT           = 1,
};


const static char * s_ForceName[Force_Max] =
{
	"无",
	"中立",
	"势力1",
	"势力2",
	"势力3",
	"势力4",
	"势力5",
	"势力6",
	"势力7",
	"势力8",
	"势力9",
};

class CNpcData;
class CNpcDataRes
{
public:
	enum{
		NPC_BEGIN_ID = 2000000,
		NPC_END_ID   = 2999999,
	};

	enum enEffectTarget{
		EffectSkill_Self     = BIT(0),
		EffectSkill_Monster  = BIT(1),
		EffectSkill_Player   = BIT(2)
	};

	typedef stdext::hash_map<S32, CNpcData*> NPCDATAMAP;
	typedef NPCDATAMAP::iterator NPCDATAITR;
	typedef std::vector<S32> NPCVECT;
	typedef NPCVECT::iterator NPCVECTITR;
	CNpcDataRes();
	~CNpcDataRes();
	bool read();
	void close();
	void clear();
	CNpcData* getNpcData(S32 NpcId);
	NPCVECT m_NpcVec[Family_Max];

private:
	NPCDATAMAP m_DataMap;
};

class CNpcData
{
public:
	enum
	{
		MAX_NPC_BUFFSIZE = 4,
	};
	CNpcData():m_NpcId(0), m_sName(""), m_sIcon(""), m_Family(0), m_Force(0), m_Difficulty(0),
		m_Limit(0), m_BaseAtk_Pc(0), m_BaseDef_Pc(0), m_BaseHp_Pc(0), m_BaseDoe_Pc(0), m_BaseCri_Pc(0),
		m_BaseSpe_Pc(0), m_See(0), m_Drop(0), m_DropVal(0), m_Active(0), m_Charater(0), m_Level(1),
		m_EffectTarget(0), m_SkillId(0), m_SkillCd(0), m_SkillRange(0)
	{
		memset(m_BaseBuff, 0, sizeof(S32) * MAX_NPC_BUFFSIZE);
	}
	~CNpcData() {};
	S32 m_NpcId;//npcid
	std::string m_sName;
	std::string m_sIcon;
	U8          m_Family;//种族
	U8          m_Force;//势力
	U8          m_Difficulty;//难度
	U8          m_Limit;//基本标记
	F32         m_BaseAtk_Pc;//攻击
	F32         m_BaseDef_Pc;//防御
	F32         m_BaseHp_Pc;//
	F32         m_BaseDoe_Pc;//闪避
	F32         m_BaseCri_Pc;//暴击
	F32         m_BaseSpe_Pc;//速度
	S32         m_BaseBuff[MAX_NPC_BUFFSIZE];
	U32         m_EffectTarget;//技能目标对象
	U32         m_SkillId;//技能id
	U32         m_SkillCd;//技能cd
	S32         m_SkillRange;//技能释法距离
	S32         m_See;//视野
	S32         m_Drop;//掉落事件
	S32         m_DropVal;//掉落几率
	S32         m_Active;//交互编号
	S32         m_Charater;//性格
	S32         m_Level;
};

extern CNpcDataRes* g_NpcDataMgr;
#endif
