#ifndef HelloLua_BuffData_h
#define HelloLua_BuffData_h
#include "Common/DataFile.h"
#include <string.h>
#include <hash_map>
#include "../Utility/GameObject/Stats.h"

class CNpcData;
//特效1
enum enEffect
{
	Effect_None=0,
	Effect_Attack,
	Effect_MaxAttack,
	Effect_Defend,
	Effect_MaxDefend,
	Effect_HP,
	Effect_MaxHP,
	Effect_ResumeHP,
	Effect_Power,
	Effect_MaxPower,
	Effect_ConsumePower,
	Effect_FireHurt,
	Effect_IceHurt,
	Effect_EleHurt,
	Effect_SubFireHurt,
	Effect_SubIceHurt,
	Effect_SubEleHurt,
	Effect_ComAttHideOdds,
	Effect_FarAttHideOdds,
	Effect_HurtFix,
	Effect_FarAttFix,
	Effect_FarHurtFix,
	Effect_Speed,
	Effect_ExpGet,
	Effect_SkillGet,
	Effect_ViewArea,
	Effect_WeaponAttResume,
	Effect_HurtResume,
	Effect_HurtRaceAdd,
	Effect_HurtRaceSub,
	Effect_AttBeatBack,
	Effect_AddImprint,
	Effect_GoodsThrowDis,
	Effect_BagCap,
	Effect_EatFoodFix,
	Effect_MonsterAdd,
	Effect_MagicOdds,
	Effect_Max,
};

const static char* s_EffectName[Effect_Max]=
{
	"无",
	"攻击",
	"最大攻击",
	"防御",
	"最大防御",
	"HP",
	"最大HP",
	"持续回复HP",
	"体力",
	"最大体力",
	"行动消耗体力",
	"火伤害",
	"冰伤害",
	"电伤害",
	"减免火伤害",
	"减免冰伤害",
	"减免电伤害",
	"普通攻击闪避率",
	"远程武器闪避率",
	"受到伤害修正",
	"远程武器攻击力修正",
	"受到远程武器伤害修正",
	"速度",
	"经验获取",
	"熟练度获取",
	"视野范围",
	"每次武器攻击回复HP",
	"每次造成伤害回复HP",
	"对种族伤害增加%",
	"受到种族伤害减少%",
	"受到普通攻击，反击伤害",
	"增加印记",
	"物品投掷距离",
	"背包容量",
	"吃食物时，恢复体力量修正",
	"召唤的怪物属性层次提升数",
	"特技触发概率"
};

const static U64 Effect1_PhyDefMax      = BIT64(0);//无敌
const static U64 Effect1_ImmunitPhy     = BIT64(1);//免伤害
const static U64 Effect1_ImmunitSpl     = BIT64(2);//免魔
const static U64 Effect1_Floating       = BIT64(3);//浮空
const static U64 Effect1_TrueSight      = BIT64(4);//真视
const static U64 Effect1_Apperceive     = BIT64(5);//感知
const static U64 Effect1_CrossAny       = BIT64(6);//魂体
const static U64 Effect1_NotDeath       = BIT64(7);//不死
const static U64 Effect1_CantAffortAtk  = BIT64(8);//二命
const static U64 Effect1_Deadly         = BIT64(9);//致死
//BIT64(10) NO USE
const static U64 Effect1_BeStealed      = BIT64(11);//被盗
const static U64 Effect1_Dreaming       = BIT64(12);//幻觉
const static U64 Effect1_Forgeting      = BIT64(13);//遗忘
const static U64 Effect1_Poisoning      = BIT64(14);//中毒
const static U64 Effect1_WetBody        = BIT64(15);//湿身
const static U64 Effect1_FireBody       = BIT64(16);//焚身
const static U64 Effect1_FeelThunder    = BIT64(17);//感电
const static U64 Effect1_Blind          = BIT64(18);//盲目
const static U64 Effect1_Invisibility   = BIT64(19);//隐形
const static U64 Effect1_BeatBack       = BIT64(20);//击退
const static U64 Effect1_Stun           = BIT64(21);//昏迷
const static U64 Effect1_Space          = BIT64(22);//空间
const static U64 Effect1_KnockDown      = BIT64(23);//击倒
const static U64 Effect1_Slumber        = BIT64(24);//睡眠
const static U64 Effect1_Fear           = BIT64(25);//恐惧
const static U64 Effect1_RunAway        = BIT64(26);//逃跑
const static U64 Effect1_Confusion      = BIT64(27);//混乱
const static U64 Effect1_Sneer          = BIT64(28);//嘲讽
const static U64 Effect1_NoFeet         = BIT64(29);//禁足
const static U64 Effect1_NoHand         = BIT64(30);//禁手
const static U64 Effect1_NoEat          = BIT64(31);//禁口
const static U64 Effect1_NoSeeing       = BIT64(32);//禁视
const static U64 Effect1_NoEquip        = BIT64(33);//缴械
const static U64 Effect1_Deformation    = BIT64(34);//变形
const static U64 Effect1_Silence        = BIT64(35);//沉默
const static U64 Effect1_Blurred        = BIT64(36);//迷糊


const static U64 Effect2_BreakTrap      = BIT64(0);//破陷
const static U64 Effect2_CutTree        = BIT64(1);//砍树
const static U64 Effect2_CutGrassland   = BIT64(2);//割草
const static U64 Effect2_BreakWall      = BIT64(3);//毁墙
const static U64 Effect2_Greedy         = BIT64(4);//吸金
const static U64 Effect2_CutMeat        = BIT64(5);//割肉
const static U64 Effect2_Curse          = BIT64(6);//诅咒
const static U64 Effect2_Corrosion      = BIT64(7);//腐蚀
const static U64 Effect2_BreakEquip     = BIT64(8);//损坏
const static U64 Effect2_Camouflage     = BIT64(9);//伪装
const static U64 Effect2_Marked         = BIT64(10);//标记
const static U64 Effect2_Bless          = BIT64(11);//祝福
const static U64 Effect2_DesAtk         = BIT64(12);//魔罗
const static U64 Effect2_NoStealEquip   = BIT64(13);//龙珠
const static U64 Effect2_NoStealFood    = BIT64(14);//魔珠
const static U64 Effect2_NoStealGold    = BIT64(15);//金珠
const static U64 Effect2_NoBeatBackFly  = BIT64(16);//八荒
const static U64 Effect2_NoConfusion    = BIT64(17);//免混
const static U64 Effect2_NoWeak         = BIT64(18);//免虚
const static U64 Effect2_NoIgnoreDef    = BIT64(19);//免破
const static U64 Effect2_NoSlumber      = BIT64(20);//免睡
const static U64 Effect2_NoFear         = BIT64(21);//免恐
const static U64 Effect2_NoStun         = BIT64(22);//免昏
const static U64 Effect2_NoKnockDown    = BIT64(23);//免倒
const static U64 Effect2_NoDeformation  = BIT64(24);//免变
const static U64 Effect2_Puncture       = BIT64(25);//刺击
const static U64 Effect2_AtkRow         = BIT64(26);//弧斩
const static U64 Effect2_AtkRound       = BIT64(27);//横扫
const static U64 Effect2_CrossByHp      = BIT64(28);//穿越
const static U64 Effect2_Summon         = BIT64(29);//召唤
const static U64 Effect2_NoDropEquip    = BIT64(30);//缠绕
const static U64 Effect2_NoBreakEquip   = BIT64(31);//防碎
const static U64 Effect2_AutoIdentify   = BIT64(32);//鉴定
const static U64 Effect2_SeeAnything    = BIT64(33);//现形
const static U64 Effect2_ShopVip        = BIT64(34);//高贵
const static U64 Effect2_BeAtked        = BIT64(35);//受伤
const static U64 Effect2_KillEnemy      = BIT64(36);//杀敌
const static U64 Effect2_DoubleRound    = BIT64(37);//加速
const static U64 Effect2_StopRound      = BIT64(38);//减速

class CBuffData;

class CBuffDataRes {

public:
	enum {
		BUFF_BEGIN_ID   = 4000001,
		BUFF_END_ID     = 4999999,
	};
	typedef stdext::hash_map<S32,CBuffData*> BUFFDATAMAP;
	typedef BUFFDATAMAP::iterator BUFFDATA_ITR;
	CBuffDataRes();
	~CBuffDataRes();


	bool read();
	void close();
	void clear();
	void readFamily(Stats& stats, U32 family, bool bAdd = true);
	CBuffData* GetBuff(S32 BuffId);
private:
	BUFFDATAMAP m_DataMap;
};

class CBuffData
{
public:
	//Flags
	enum Flags
	{
		Flags_CalcStats			= BIT(0),		// 计算属性，反之则计算伤害
		Flags_PhyDamage			= BIT(1),		// 物理伤害
		Flags_HuoDamage         = BIT(2),		// 火伤害
		Flags_ShuiDamage		= BIT(3),		// 水伤害
		Flags_JinDamage         = BIT(4),		// 电攻击
		Flags_OtherDamage		= BIT(5),		// 其他攻击
		Flags_Heal              = BIT(6),		// HP
		Flags_PlusStats         = BIT(7),       // 叠层标识
	};

	enum DamageType
	{
		Damage_Phy				= BIT(1),
		Damage_Huo				= BIT(2),
		Damage_Shui				= BIT(3),
		Damage_Jin              = BIT(4),
		Damage_Other			= BIT(5),
		Damage_Heal				= BIT(6),
		Damage_Spl				= Damage_Huo | Damage_Shui | Damage_Jin,
		Damage_All              = Damage_Phy | Damage_Spl  | Damage_Other | Damage_Heal,
	};
	//初始回合
	CBuffData():m_BuffId(0),m_sName(""),m_sStateInfo(""), m_BuffEffect1(0),m_BuffEffect2(0),
		m_Stats(), m_LimitCount(1), m_LimitTimes(2), m_Flags(0), m_Group(0), \
		m_RemoveGroup(0), m_ImmunitGroup(0), m_LvUpExp(0)
	{
	}

	~CBuffData()
	{
	}

	void Plus(const CBuffData& _orig, const CBuffData& _plus);
	void PlusDone();
	void percentNpc(CNpcData* pData);

	inline U32 GetBuffId()          { return m_BuffId; };
	inline void Clear()             { memset(this, 0, sizeof(CBuffData)); };
	inline Stats& GetStats()        { return m_Stats; }
	inline bool	 IsFlags(U32 flags) { return m_Flags & flags; }
	// dinline bool
	inline U64	GetEffect1()         { return m_BuffEffect1; }
	inline U64	GetEffect2()         { return m_BuffEffect2; }
	inline bool isGroup(U32 group)   { return m_Group & group; }

	inline U32  GetDamageType()      {  return m_Flags & Damage_All; }
	const char* getName(){return m_sName.c_str();}
	Stats m_Stats;
	U32 m_BuffId;
	std::string m_sName;
	std::string m_sStateInfo; //状态描述
	U64 m_BuffEffect1;//状态效果
	U64 m_BuffEffect2;//状态效果
	U32 m_Flags; //状态标识(特殊处理状态位)
	U64 m_Group;//状态组
	U64 m_RemoveGroup;//移除组
	U64 m_ImmunitGroup;//免疫组
	U32 m_LimitCount;
	U32 m_LimitTimes;
	U64 m_LvUpExp;
};

extern CBuffData g_BuffData;

extern CBuffDataRes* g_BuffDataMgr;
#endif
