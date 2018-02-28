#ifndef __BUFF_DATA__
#define __BUFF_DATA__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>
#include <memory>

struct CBuffData;
struct Stats;

class CBuffDataRes:public CBaseDataRes<CBuffData>
{

public:
	CBuffDataRes();
	virtual ~CBuffDataRes();

	bool read();
};

struct CBuffData
{
	CBuffData():m_BuffId(0), m_sDes(""), m_BuffType(0), m_BuffVal(0), m_BattleFlag(0)
	{};
	~CBuffData(){};

	void getStats(Stats& stats);

	inline bool IsAddBusinsessPer()			{ return m_BuffType == 1; }
	inline bool IsAddOfficerBusPer()		{ return m_BuffType == 2; }
	inline bool IsRedDisPatchPer()			{ return m_BuffType == 3; }
	inline bool IsAddAchievementPer()		{ return m_BuffType == 5; }
	inline bool IsRedMoneyPer()				{ return m_BuffType == 6; }
	inline bool IsAddOfficerTecPer()		{ return m_BuffType == 10; }
	inline bool IsAddTechnologyPer()		{ return m_BuffType == 11; }
	inline bool IsAddMilitaryPer()			{ return m_BuffType == 13; }
	inline bool IsAddOfficerMilPer()		{ return m_BuffType == 14; }
	inline bool IsAddAttackPer()			{ return m_BuffType == 15; }
	inline bool IsAddCriticalPer()			{ return m_BuffType == 16; }
	inline bool IsAddHpPer()				{ return m_BuffType == 17; }
	inline bool IsAddDodgePer()			    { return m_BuffType == 18; }
	inline bool IsEquipAddAgi()				{ return m_BuffType == 28; }
	inline bool IsEquipAddInt()				{ return m_BuffType == 29; }
	inline bool IsEquipAddStr()				{ return m_BuffType == 30; }
	inline bool IsRedCardLevelPer()			{ return m_BuffType == 34; }
	inline bool IsAddHp()					{ return m_BuffType == 35; }
	inline bool IsAddAttack()				{ return m_BuffType == 36; }


	U32 m_BuffId;
	std::string	m_sDes;
	U8  m_BuffType;
	F32	m_BuffVal;
	U8	m_BattleFlag;
};
extern CBuffDataRes* g_BuffDataMgr;

struct Stats
{
	F32 Bus_Pc;//商业开发效果1
	F32 OffBus_gPc;//官位商业值提升2
	F32 DevTime_Pc;//内政开发时间减少3
	F32 PvpHP_gPc;//比武时血量提升4
	F32 Ach_Pc;//功勋获得量提升5
	F32 LvCost_Pc;//升级银两减少6
	F32	Queen_gPc;	//礼官效果提升7
	F32 PvpAtk_gPc;//比武时伤害提升8
	F32 TecCost_gPc;//研究科技费用减少9
	F32 OffTec_gPc;//官位工业值提升10
	F32 Tec_Pc;//工业开发效果11
	F32 TecTime_gPc;//研究科技时间减少12
	F32	Mil_Pc;//军事开发效果提升13
	F32	OffMil_gPc;//官位军事值提升14
	F32 Atk_gPc;//士兵攻击提升15
	F32 Cri_Pc;//士兵暴击提升16
	F32 HP_gPC;//士兵血量提升17
	F32 Dog_gPC;//伤害减半18
	F32 Skill[9];//技能19
	F32 Agi;//政治 28
	F32 Int;//学识29
	F32 Str;//统率30
	F32 Bus;//商业31
	F32 Tec;//工业32
	F32 Mil;//军事33
	F32 ClvCost_gPc;//大臣升级银两减少34
	F32 SolHp;//士兵血量35
	F32 SolAtk;//士兵攻击36
	F32 FlowerSuc_gPc;//养花大成功率37
	F32 Culture_gPc;//养花修养值38
	F32 Rite_Pc;//学习礼仪值39
	F32 PBattle_Pc;//处理宫务宫斗值40
	F32 FlowerSuc_Pc;//养花大成功率41
	F32 Charm;//魅力值42
	F32	PrinceAbi_gPc;//所生皇子全能力提升43
	F32 DevCost_gPc;//内政开发费用44
	F32 AddMoney_gPc;//银两小时产出45
	F32 AddOre_gPc;//矿石小时产出46
	F32 DevNum;//个内政队列47
	F32 Bus_gPc;//开发商业效果48
	F32 Tec_gPc;//开发工业效果49
	F32 Agi_gPc;//提升所有大臣政治50
	F32 Ach_gPc;//增加所有大臣功勋获得51
	F32 Land_gPc;//提升内政开发上限52
	F32 AddRes_gPc;//提升银两、矿石、士兵{0}%小时产出53
	F32	StrongHold_gP;//增加{ 0 }%征服点产出54
	F32 CardNum;//增加{ 0 }个大臣席位55
	F32 Str_gPc;////提升所有大臣{0}%统率56
	F32 OffRank;//开放官品57
	F32 TechNum;//增加{0}个科技队列58
	F32 ApplyNum;//增加{0}个奏章上限59
	F32 Rite_gPc;//增加{0}%妃子礼仪学习效果60
	F32 Int_gPc;//提升所有大臣{0}%学识61
	F32 PrinceNum;//增加{0}个上书房皇子席位62
	F32 Cri_gPc;//增加士兵{0}%暴击率	63
	F32 RedAch_gPc;//资质突破消耗功勋减少{0}%	64
	F32 CallTroop_gPc;//增加{0}%兵力征召	65
	F32 Mil_gPc;//提升{0}%开发军事效果		66
	F32 BattleNum;//增加1个副将位		67
	F32 PvpAgi_gPc;//临时政治提升     68
	F32 PvpInt_gPc;//临时学识提升     69
	F32 PvpStr_gPc;//临时军事提升	  70
	F32 PvpHp_gPc;//回复最大血量	  71
	F32 PvpStats_gPc;//对手全属性降低	  72
	F32 Impose_gPc;//征收令上限			  73
	F32 OrgPlayerNum;//联盟最大人数			74
	F32 OrgContribute_gPc;//联盟捐献个人贡献提升{0}%	75
	F32 OrgContributeExp_gPc;//联盟捐献经验提升{0}%s	76
	F32 OrgCharm;//联盟玩家妃子魅力		77
	F32 OrgConquest_gPc; //联盟玩家征服点获得提升{ 0 }%		78
	F32 OrgImposeMoney_gPc;//联盟玩家银两征收提升{0}%		79
	F32 OrgOre_gPc;//联盟玩家矿石征收提升{0}%		80
	F32 OrgTroop_gPc;//联盟玩家征兵数量提升{0}%		81
	F32 OrgManagerNum;//联盟精英成员数量			82

	Stats();
	void Clear();

	Stats& operator+=(const Stats&);
	Stats& operator-=(const Stats&);
};

typedef std::shared_ptr<Stats> StatsRef;
extern Stats g_Stats;
#endif//__BUFF_DATA__