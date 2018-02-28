//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FireRain.com, Inc.
//-----------------------------------------------------------------------------

#include "Stats.h"
#include <math.h>
#include "../../Data/NpcData.h"

// ========================================================================================================================================
//  Stats
// ========================================================================================================================================
Stats g_Stats;

Stats::Stats()
{
	Clear();
}

void Stats::Clear()
{
	dMemset(this, 0, sizeof(Stats));
}

// 做A+=B和A-=B时，注意A::HP和A::HP_Pc没有做运算。所以一般来说A的初始值都为0
Stats& Stats::operator+=(const Stats& _add)
{
	HP += _add.HP;
	HP_gPc += _add.HP_gPc;

	MaxHP += _add.MaxHP;
	MaxHP_gPc += _add.MaxHP_gPc;

	Str += _add.Str;
	Str_gPc += _add.Str_gPc;

	MaxStr += _add.MaxStr;
	MaxStr_gPc += _add.MaxStr_gPc;

	Cri_gPc += _add.Cri_gPc;

	Doe_gPc += _add.Doe_gPc;
	DoeR_gPc += _add.DoeR_gPc;

	Spe += _add.Spe;

	PhyDamage += _add.PhyDamage;
	PhyDamage_gPc += _add.PhyDamage_gPc;

	PhyDefence += _add.PhyDefence;
	PhyDefence_gPc += _add.PhyDefence_gPc;

	MuDamage += _add.MuDamage;
	MuDamage_gPc += _add.MuDamage_gPc;

	MuDefence += _add.MuDefence;
	MuDefence_gPc += _add.MuDefence_gPc;

	HuoDamage += _add.HuoDamage;
	HuoDamage_gPc += _add.HuoDamage_gPc;

	HuoDefence += _add.HuoDefence;
	HuoDefence_gPc += _add.HuoDefence_gPc;

	TuDamage += _add.TuDamage;
	TuDamage_gPc += _add.TuDamage_gPc;

	TuDefence += _add.TuDefence;
	TuDefence_gPc += _add.TuDefence_gPc;

	JinDamage += _add.JinDamage;
	JinDamage_gPc += _add.JinDamage_gPc;

	JinDefence += _add.JinDefence;
	JinDefence_gPc += _add.JinDefence_gPc;

	ShuiDamage += _add.ShuiDamage;
	ShuiDamage_gPc += _add.ShuiDamage_gPc;

	ShuiDefence += _add.ShuiDefence;
	ShuiDefence_gPc += _add.ShuiDefence_gPc;

	Sheng_Damage_gPc += _add.Sheng_Damage_gPc;
	Fo_Damage_gPc += _add.Fo_Damage_gPc;
	Xian_Damage_gPc += _add.Xian_Damage_gPc;
	Jing_Damage_gPc += _add.Jing_Damage_gPc;
	Gui_Damage_gPc += _add.Gui_Damage_gPc;
	Guai_Damage_gPc += _add.Guai_Damage_gPc;
	Yao_Damage_gPc += _add.Yao_Damage_gPc;
	Mo_Damage_gPc += _add.Mo_Damage_gPc;
	Shou_Damage_gPc += _add.Shou_Damage_gPc;
	Long_Damage_gPc += _add.Long_Damage_gPc;

	DeriveHP += _add.DeriveHP;
	RecoverHP += _add.RecoverHP;
	RecoverHp_gPc += _add.RecoverHp_gPc;
	See += _add.See;
	ReduceStr += _add.ReduceStr;
	PhyImmunit += _add.PhyImmunit;
	PhyImmunit_gPc += _add.PhyImmunit_gPc;
	RemoteAtk += _add.RemoteAtk;
	RemoteAtk_gPc += _add.RemoteAtk_gPc;
	Reflect_gPc += _add.Reflect_gPc;

	return *this;
}

Stats& Stats::operator-=(const Stats& _add)
{
	HP -= _add.HP;
	HP_gPc -= _add.HP_gPc;

	MaxHP -= _add.MaxHP;
	MaxHP_gPc -= _add.MaxHP_gPc;

	Str -= _add.Str;
	Str_gPc -= _add.Str_gPc;

	MaxStr -= _add.MaxStr;
	MaxStr_gPc -= _add.MaxStr_gPc;

	Cri_gPc -= _add.Cri_gPc;

	Doe_gPc -= _add.Doe_gPc;
	DoeR_gPc -= _add.DoeR_gPc;

	Spe -= _add.Spe;

	PhyDamage -= _add.PhyDamage;
	PhyDamage_gPc -= _add.PhyDamage_gPc;

	PhyDefence -= _add.PhyDefence;
	PhyDefence_gPc -= _add.PhyDefence_gPc;

	MuDamage -= _add.MuDamage;
	MuDamage_gPc -= _add.MuDamage_gPc;

	MuDefence -= _add.MuDefence;
	MuDefence_gPc -= _add.MuDefence_gPc;

	HuoDamage -= _add.HuoDamage;
	HuoDamage_gPc -= _add.HuoDamage_gPc;

	HuoDefence -= _add.HuoDefence;
	HuoDefence_gPc -= _add.HuoDefence_gPc;

	TuDamage -= _add.TuDamage;
	TuDamage_gPc -= _add.TuDamage_gPc;

	TuDefence -= _add.TuDefence;
	TuDefence_gPc -= _add.TuDefence_gPc;

	JinDamage -= _add.JinDamage;
	JinDamage_gPc -= _add.JinDamage_gPc;

	JinDefence -= _add.JinDefence;
	JinDefence_gPc -= _add.JinDefence_gPc;

	ShuiDamage -= _add.ShuiDamage;
	ShuiDamage_gPc -= _add.ShuiDamage_gPc;

	ShuiDefence -= _add.ShuiDefence;
	ShuiDefence_gPc -= _add.ShuiDefence_gPc;

	Sheng_Damage_gPc -= _add.Sheng_Damage_gPc;
	Fo_Damage_gPc -= _add.Fo_Damage_gPc;
	Xian_Damage_gPc -= _add.Xian_Damage_gPc;
	Jing_Damage_gPc -= _add.Jing_Damage_gPc;
	Gui_Damage_gPc -= _add.Gui_Damage_gPc;
	Guai_Damage_gPc -= _add.Guai_Damage_gPc;
	Yao_Damage_gPc -= _add.Yao_Damage_gPc;
	Mo_Damage_gPc -= _add.Mo_Damage_gPc;
	Shou_Damage_gPc -= _add.Shou_Damage_gPc;
	Long_Damage_gPc -= _add.Long_Damage_gPc;

	DeriveHP -= _add.DeriveHP;
	RecoverHP -= _add.RecoverHP;
	RecoverHp_gPc -= _add.RecoverHp_gPc;
	See -= _add.See;
	ReduceStr -= _add.ReduceStr;
	PhyImmunit -= _add.PhyImmunit;
	PhyImmunit_gPc -= _add.PhyImmunit_gPc;
	RemoteAtk -= _add.RemoteAtk;
	RemoteAtk_gPc -= _add.RemoteAtk_gPc;
	Reflect_gPc -= _add.Reflect_gPc;

	return *this;
}

Stats& Stats::operator*=(const F32 scale)
{
	MaxHP *= scale;

	//可能会出问题
	MaxHP_gPc *=scale;

	PhyDamage *= scale;
	PhyDefence *= scale;
	MuDamage *= scale;
	MuDefence *= scale;
	HuoDamage *= scale;
	HuoDefence *= scale;
	TuDamage *= scale;
	TuDefence *= scale;
	JinDamage *= scale;
	JinDefence *= scale;
	ShuiDamage *= scale;
	ShuiDefence *= scale;
	DeriveHP *= scale;
	RecoverHP *= scale;

	return *this;
}

void Stats::Calc(const Stats& _add)
{
	Clear();
	MaxHP = getMax((_add.MaxHP) * (1.0f + _add.MaxHP_gPc), 1.0f);
	MaxStr = getMax((_add.MaxStr) * (1.0f + _add.MaxStr_gPc), 1.0f);

	RemoteAtk = getMax((_add.RemoteAtk)* (1.0f + _add.RemoteAtk_gPc), 0.0f);
	PhyDamage = getMax((_add.PhyDamage) * (1.0f + _add.PhyDamage_gPc), 0.0f);
	PhyDefence = getMax((_add.PhyDefence) * (1.0f + _add.PhyDefence_gPc), 0.0f);

	MuDamage = getMax((_add.MuDamage) * (1.0f + _add.MuDamage_gPc), 0.0f);
	HuoDamage = getMax((_add.HuoDamage) * (1.0f + _add.HuoDamage_gPc), 0.0f);
	TuDamage = getMax((_add.TuDamage) * (1.0f + _add.TuDamage_gPc ), 0.0f);
	JinDamage = getMax((_add.JinDamage) * (1.0f + _add.JinDamage_gPc ), 0.0f);
	ShuiDamage = getMax((_add.ShuiDamage) * (1.0f + _add.ShuiDamage_gPc ), 0.0f);

	MuDefence = _add.MuDefence;
	HuoDefence = _add.HuoDefence;
	TuDefence = _add.TuDefence;
	JinDefence = _add.JinDefence;
	ShuiDefence = _add.ShuiDefence;
	Doe_gPc = _add.Doe_gPc;

	MuDefence_gPc = _add.MuDefence_gPc;
	HuoDefence_gPc = _add.HuoDefence_gPc;
	TuDefence_gPc = _add.TuDefence_gPc;
	JinDefence_gPc = _add.JinDefence_gPc;
	ShuiDefence_gPc = _add.ShuiDefence_gPc;

	HP = getMax((_add.HP) * (1.0f + _add.HP_gPc), 1.0f);
	Str = getMax((_add.Str) * (1.0f + _add.Str_gPc), 1.0f);

	Sheng_Damage_gPc = _add.Sheng_Damage_gPc;
	Fo_Damage_gPc = _add.Fo_Damage_gPc;
	Xian_Damage_gPc = _add.Xian_Damage_gPc;
	Jing_Damage_gPc = _add.Jing_Damage_gPc;
	Gui_Damage_gPc = _add.Gui_Damage_gPc;
	Guai_Damage_gPc = _add.Guai_Damage_gPc;
	Yao_Damage_gPc = _add.Yao_Damage_gPc;
	Mo_Damage_gPc = _add.Mo_Damage_gPc;
	Shou_Damage_gPc = _add.Shou_Damage_gPc;
	Long_Damage_gPc = _add.Long_Damage_gPc;
	DeriveHP = _add.DeriveHP;
	RecoverHP = _add.RecoverHP;
	RecoverHp_gPc = _add.RecoverHp_gPc;
	See = _add.See;
	ReduceStr = _add.ReduceStr;
	PhyImmunit = _add.PhyImmunit;
	PhyImmunit_gPc = _add.PhyImmunit_gPc;
	Spe = _add.Spe;
	Reflect_gPc = _add.Reflect_gPc;
}

void Stats::Plus(const Stats& _orig, const Stats& _plus)
{
	//HP = _orig.HP + _plus.HP;
	//HP_gPc = _orig.HP_gPc + _plus.HP_gPc;

	MaxHP = _orig.MaxHP + _plus.MaxHP;
	MaxHP_gPc = _orig.MaxHP_gPc + _plus.MaxHP_gPc;

	//Str = _orig.Str + _plus.Str;
	//Str_gPc = _orig.Str_gPc + _plus.Str_gPc;

	MaxStr = _orig.MaxStr + _plus.MaxStr;
	MaxStr_gPc = _orig.MaxStr_gPc + _plus.MaxStr_gPc;

	Cri_gPc = _orig.Cri_gPc + _plus.Cri_gPc;

	Doe_gPc = _orig.Doe_gPc + _plus.Doe_gPc;
	DoeR_gPc = _orig.DoeR_gPc + _plus.DoeR_gPc;

	Spe = _orig.Spe + _plus.Spe;

	PhyDamage = _orig.PhyDamage + _plus.PhyDamage;
	PhyDamage_gPc = _orig.PhyDamage_gPc + _plus.PhyDamage_gPc;

	PhyDefence = _orig.PhyDefence + _plus.PhyDefence;
	PhyDefence_gPc = _orig.PhyDefence_gPc + _plus.PhyDefence_gPc;
	MuDamage = _orig.MuDamage + _plus.MuDamage;
	MuDamage_gPc = _orig.MuDamage_gPc + _plus.MuDamage_gPc;
	MuDefence = _orig.MuDefence + _plus.MuDefence;
	MuDefence_gPc = _orig.MuDefence_gPc + _plus.MuDefence_gPc;
	HuoDamage = _orig.HuoDamage + _plus.HuoDamage;
	HuoDamage_gPc = _orig.HuoDamage_gPc + _plus.HuoDamage_gPc;
	HuoDefence = _orig.HuoDefence + _plus.HuoDefence;
	HuoDefence_gPc = _orig.HuoDefence_gPc + _plus.HuoDefence_gPc;
	TuDamage = _orig.TuDamage + _plus.TuDamage;
	TuDamage_gPc = _orig.TuDamage_gPc + _plus.TuDamage_gPc;
	TuDefence = _orig.TuDefence + _plus.TuDefence;
	TuDefence_gPc = _orig.TuDefence_gPc + _plus.TuDefence_gPc;
	JinDamage = _orig.JinDamage + _plus.JinDamage;
	JinDamage_gPc = _orig.JinDamage_gPc + _plus.JinDamage_gPc;
	JinDefence = _orig.JinDefence + _plus.JinDefence;
	JinDefence_gPc = _orig.JinDefence_gPc + _plus.JinDefence_gPc;
	ShuiDamage = _orig.ShuiDamage + _plus.ShuiDamage;
	ShuiDamage_gPc = _orig.ShuiDamage_gPc + _plus.ShuiDamage_gPc;
	ShuiDefence = _orig.ShuiDefence + _plus.ShuiDefence;
	ShuiDefence_gPc = _orig.ShuiDefence_gPc + _plus.ShuiDefence_gPc;

	Sheng_Damage_gPc = _orig.Sheng_Damage_gPc + _plus.Sheng_Damage_gPc;
	Fo_Damage_gPc = _orig.Fo_Damage_gPc + _plus.Fo_Damage_gPc;
	Xian_Damage_gPc = _orig.Xian_Damage_gPc + _plus.Xian_Damage_gPc;
	Jing_Damage_gPc = _orig.Jing_Damage_gPc + _plus.Jing_Damage_gPc;
	Gui_Damage_gPc = _orig.Gui_Damage_gPc + _plus.Gui_Damage_gPc;
	Guai_Damage_gPc = _orig.Guai_Damage_gPc + _plus.Guai_Damage_gPc;
	Yao_Damage_gPc = _orig.Yao_Damage_gPc + _plus.Yao_Damage_gPc;
	Mo_Damage_gPc = _orig.Mo_Damage_gPc + _plus.Mo_Damage_gPc;
	Shou_Damage_gPc = _orig.Shou_Damage_gPc + _plus.Shou_Damage_gPc;
	Long_Damage_gPc = _orig.Long_Damage_gPc + _plus.Long_Damage_gPc;

	DeriveHP = _orig.DeriveHP + _plus.DeriveHP;
	RecoverHP = _orig.RecoverHP + _plus.RecoverHP;
	RecoverHp_gPc = _orig.RecoverHp_gPc + _plus.RecoverHp_gPc;
	See = _orig.See + _plus.See;
	ReduceStr = _orig.ReduceStr + _plus.ReduceStr;
	PhyImmunit = _orig.PhyImmunit + _plus.PhyImmunit;
	PhyImmunit_gPc = _orig.PhyImmunit_gPc + _plus.PhyImmunit_gPc;
	RemoteAtk = _orig.RemoteAtk + _plus.RemoteAtk;
	RemoteAtk_gPc = _orig.RemoteAtk_gPc + _plus.RemoteAtk_gPc;
	Reflect_gPc = _orig.Reflect_gPc + _plus.Reflect_gPc;
}

void Stats::percent(const F32 val)
{
	HP *= val;
	HP_gPc *= val;

	MaxHP *= val;// //mFloor(_add.MaxHP + _add.MaxHP * _add.MaxHP_Pc / GAMEPLAY_PERCENT_SCALE);
	MaxHP_gPc *= val;// val;//_add.MaxHP_gPc;

	Str *= val;
	Str_gPc *= val;

	MaxStr *= val;
	MaxStr_gPc *= val;

	Cri_gPc *= val;

	Doe_gPc *= val;
	DoeR_gPc *= val;

	Spe *= val;

	PhyDamage *= val;// mFloor(_add.PhyDamage + _add.PhyDamage * _add.PhyDamage_Pc / GAMEPLAY_PERCENT_SCALE);
	PhyDamage_gPc *= val;// _add.PhyDamage_gPc;
	PhyDefence *= val;// mFloor(_add.PhyDefence + _add.PhyDefence * _add.PhyDefence_Pc / GAMEPLAY_PERCENT_SCALE);
	PhyDefence_gPc *= val;// _add.PhyDefence_gPc;
	MuDamage *= val;// mFloor(_add.MuDamage + _add.MuDamage * _add.MuDamage_Pc / GAMEPLAY_PERCENT_SCALE);
	MuDamage_gPc *= val;// _add.MuDamage_gPc;
	MuDefence *= val;// mFloor(_add.MuDefence + _add.MuDefence * _add.MuDefence_Pc / GAMEPLAY_PERCENT_SCALE);
	MuDefence_gPc *= val;// _add.MuDefence_gPc;
	HuoDamage *= val;// mFloor(_add.HuoDamage + _add.HuoDamage * _add.HuoDamage_Pc / GAMEPLAY_PERCENT_SCALE);
	HuoDamage_gPc *= val;// _add.HuoDamage_gPc;
	HuoDefence *= val;// mFloor(_add.HuoDefence + _add.HuoDefence * _add.HuoDefence_Pc / GAMEPLAY_PERCENT_SCALE);
	HuoDefence_gPc *= val;// _add.HuoDefence_gPc;
	TuDamage *= val;// mFloor(_add.TuDamage + _add.TuDamage * _add.TuDamage_Pc / GAMEPLAY_PERCENT_SCALE);
	TuDamage_gPc *= val;// _add.TuDamage_gPc;
	TuDefence *= val;// mFloor(_add.TuDefence + _add.TuDefence * _add.TuDefence_Pc / GAMEPLAY_PERCENT_SCALE);
	TuDefence_gPc *= val;// _add.TuDefence_gPc;
	JinDamage *= val;// mFloor(_add.JinDamage + _add.JinDamage * _add.JinDamage_Pc / GAMEPLAY_PERCENT_SCALE);
	JinDamage_gPc *= val;// _add.JinDamage_gPc;
	JinDefence *= val;// mFloor(_add.JinDefence + _add.JinDefence * _add.JinDefence_Pc / GAMEPLAY_PERCENT_SCALE);
	JinDefence_gPc *= val;// _add.JinDefence_gPc;
	ShuiDamage *= val;// mFloor(_add.ShuiDamage + _add.ShuiDamage * _add.ShuiDamage_Pc / GAMEPLAY_PERCENT_SCALE);
	ShuiDamage_gPc *= val;// _add.ShuiDamage_gPc;
	ShuiDefence *= val;// mFloor(_add.ShuiDefence + _add.ShuiDefence * _add.ShuiDefence_Pc / GAMEPLAY_PERCENT_SCALE);
	ShuiDefence_gPc *= val;// _add.ShuiDefence_gPc;

	DeriveHP *= val;
	RecoverHP *= val;
	RecoverHp_gPc *= val;
	See *= val;
	ReduceStr *= val;
	PhyImmunit *= val;
	PhyImmunit_gPc *= val;
	RemoteAtk *= val;
	RemoteAtk_gPc *= val;
	Reflect_gPc *= val;
}

void Stats::percentNpc(CNpcData *pData)
{
	PhyDamage = floor(getMax((PhyDamage)* (1.0f + pData->m_BaseAtk_Pc), 0.0f));

	PhyDefence = floor(getMax((PhyDefence)* (1.0f + pData->m_BaseDef_Pc), 0.0f));
	MaxHP = floor(getMax((MaxHP)* (1.0f + pData->m_BaseHp_Pc), 0.0f));

	Doe_gPc = (getMax((Doe_gPc)* (1.0f + pData->m_BaseDoe_Pc), 0.0f));
	DoeR_gPc = (getMax((DoeR_gPc)* (1.0f + pData->m_BaseDoe_Pc), 0.0f));

	Cri_gPc = (getMax((Cri_gPc)* (1.0f + pData->m_BaseCri_Pc), 0.0f));
	Spe = floor(getMax((Spe)* (1.0f + pData->m_BaseSpe_Pc), 0.0f));
	See = pData->m_See;
}


void Stats::PlusDone()
{
}

void Stats::PlusSrc(const Stats& _orig, S32 ratePhy,  S32 rateSpl, S32 ratePnm,S32 rateQua)
{
	//*如果技能里有全局 在这里加 人物身上的 在Calc函数里加*//
	// 加成比例
	F32 _ratePhy = mClampF(ratePhy/GAMEPLAY_PERCENT_SCALE, 0.0f, 3.0f);
	F32 _rateSpl = mClampF(rateSpl/GAMEPLAY_PERCENT_SCALE, 0.0f, 3.0f);
	F32 _ratePnm = mClampF(ratePnm/GAMEPLAY_PERCENT_SCALE, 0.0f, 3.0f);
	F32 _rateQua = mClampF(rateQua/GAMEPLAY_PERCENT_SCALE, 0.0f, 3.0f);

	// 只是伤害属性按比例加成（事实上，某些属性必定受到全额加成）
	PhyDamage += _orig.PhyDamage * _ratePhy * (getMax(PhyDamage_gPc + GAMEPLAY_PERCENT_SCALE, 0.0f) / GAMEPLAY_PERCENT_SCALE);
	MuDamage += _orig.MuDamage * _rateSpl * (getMax(MuDamage_gPc + GAMEPLAY_PERCENT_SCALE, 0.0f) / GAMEPLAY_PERCENT_SCALE);
	HuoDamage += _orig.HuoDamage * _rateSpl * (getMax(HuoDamage_gPc + GAMEPLAY_PERCENT_SCALE, 0.0f) / GAMEPLAY_PERCENT_SCALE);
	TuDamage += _orig.TuDamage * _rateSpl * (getMax(TuDamage_gPc + GAMEPLAY_PERCENT_SCALE, 0.0f) / GAMEPLAY_PERCENT_SCALE);
	JinDamage += _orig.JinDamage * _rateSpl * (getMax(JinDamage_gPc + GAMEPLAY_PERCENT_SCALE, 0.0f) / GAMEPLAY_PERCENT_SCALE);
	ShuiDamage += _orig.ShuiDamage * _rateSpl * (getMax(ShuiDamage_gPc + GAMEPLAY_PERCENT_SCALE, 0.0f) / GAMEPLAY_PERCENT_SCALE);
}

Stats& Stats::Add(const Stats& _add, S32 _count /* = 1 */)
{
	HP += _add.HP * _count;
	HP_gPc += _add.HP_gPc * _count;

	MaxHP += _add.MaxHP * _count;
	MaxHP_gPc += _add.MaxHP_gPc * _count;

	Str += _add.Str * _count;
	Str_gPc += _add.Str_gPc * _count;

	MaxStr += _add.MaxStr * _count;
	MaxStr_gPc += _add.MaxStr_gPc * _count;

	Cri_gPc += _add.Cri_gPc * _count;

	Doe_gPc += _add.Doe_gPc * _count;
	DoeR_gPc += _add.DoeR_gPc * _count;

	Spe += _add.Spe * _count;

	PhyDamage += _add.PhyDamage * _count;
	PhyDamage_gPc += _add.PhyDamage_gPc * _count;

	PhyDefence += _add.PhyDefence * _count;
	PhyDefence_gPc += _add.PhyDefence_gPc * _count;

	MuDamage += _add.MuDamage * _count;
	MuDamage_gPc += _add.MuDamage_gPc * _count;

	MuDefence += _add.MuDefence * _count;
	MuDefence_gPc += _add.MuDefence_gPc * _count;

	HuoDamage += _add.HuoDamage * _count;
	HuoDamage_gPc += _add.HuoDamage_gPc * _count;

	HuoDefence += _add.HuoDefence * _count;
	HuoDefence_gPc += _add.HuoDefence_gPc * _count;

	TuDamage += _add.TuDamage * _count;
	TuDamage_gPc += _add.TuDamage_gPc * _count;

	TuDefence += _add.TuDefence * _count;
	TuDefence_gPc += _add.TuDefence_gPc * _count;

	JinDamage += _add.JinDamage * _count;
	JinDamage_gPc += _add.JinDamage_gPc * _count;

	JinDefence += _add.JinDefence * _count;
	JinDefence_gPc += _add.JinDefence_gPc * _count;

	ShuiDamage += _add.ShuiDamage * _count;
	ShuiDamage_gPc += _add.ShuiDamage_gPc * _count;

	ShuiDefence += _add.ShuiDefence * _count;
	ShuiDefence_gPc += _add.ShuiDefence_gPc * _count;

	Sheng_Damage_gPc += _add.Sheng_Damage_gPc * _count;
	Fo_Damage_gPc += _add.Fo_Damage_gPc * _count;
	Xian_Damage_gPc += _add.Xian_Damage_gPc * _count;
	Jing_Damage_gPc += _add.Jing_Damage_gPc * _count;
	Gui_Damage_gPc += _add.Gui_Damage_gPc * _count;
	Guai_Damage_gPc += _add.Guai_Damage_gPc * _count;
	Yao_Damage_gPc += _add.Yao_Damage_gPc * _count;
	Mo_Damage_gPc += _add.Mo_Damage_gPc * _count;
	Shou_Damage_gPc += _add.Shou_Damage_gPc * _count;
	Long_Damage_gPc += _add.Long_Damage_gPc * _count;
	DeriveHP += _add.DeriveHP * _count;
	RecoverHP += _add.RecoverHP * _count;
	RecoverHp_gPc += _add.RecoverHp_gPc * _count;
	See += _add.See * _count;
	ReduceStr += _add.ReduceStr * _count;
	PhyImmunit += _add.PhyImmunit * _count;
	PhyImmunit_gPc += _add.PhyImmunit_gPc * _count;
	RemoteAtk += _add.RemoteAtk * _count;
	RemoteAtk_gPc += _add.RemoteAtk_gPc * _count;
	Reflect_gPc += _add.Reflect_gPc * _count;

	return *this;
}
