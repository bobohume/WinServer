//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FireRain.com, Inc.
//-----------------------------------------------------------------------------

#pragma once

#include "Base/types.h"    
#include <iostream>
// ========================================================================================================================================
//  Stats
// ========================================================================================================================================

enum TIP_KIND
{
	TIP_DAMAGE=0,
	TIP_NO_HIT,
	TIP_SKILL,
	TIP_TRIG_STAT,
	TIP_TRAP,
	TIP_DOWN,
	TIP_NO_WALK,
	TIP_ITEM_FULL,
	TIP_ABORT_ITEM,
	TIP_WALK_ON,
	TIP_RECOVER_STATE,
	TIP_WATCH,
	TIP_ITEM_STOLE,
	TIP_ITEM_COMBINE,
	TIP_ITEM_STRENG,
	TIP_ITEM_GET,
	TIP_EXP_GET,
	TIP_LV_UP,
	TIP_MONSTER_UP,
	TIP_IMMUNITY,//免疫
	TIP_JOUK,//闪避
	TIP_FORCE_ATK,//暴击
	TIP_DIZZY,//晕
	TIP_DEATH,//致死
	TIP_COM_EFFECT,//通用特效
};


class CNpcData;

struct Stats
{
	enum DerateOrSwellType
	{
		DerateOrSwellType_nullity			= 0,
		DerateOrSwellType_availability,
	};
	S32			HP;					// 生命
	F32			HP_gPc;				// 生命百分比

	S32			MaxHP;				// 最大生命
	F32			MaxHP_gPc;			// 最大生命百分比

	S32			Str;				// 体力
	F32			Str_gPc;			// 体力百分比

	S32			MaxStr;				// 最大体力
	F32			MaxStr_gPc;			// 最大体力百分比

	F32			Cri_gPc;            // 暴击几率百分比

	F32			Doe_gPc;            // 普攻闪避几率

	F32         DoeR_gPc;           // 远程类武器闪避几率

	F32         Spe;                // 速度

	S32			PhyDamage;			// 物理伤害
	F32			PhyDamage_gPc;		// 物理伤害百分比

	S32			PhyDefence;			// 物理防御
	F32			PhyDefence_gPc;		// 物理防御百分比

	S32			MuDamage;			// 木伤害
	F32			MuDamage_gPc;		// 木伤害百分比

	S32			MuDefence;			// 木伤害减免
	F32			MuDefence_gPc;		// 木伤害减免百分比

	S32			HuoDamage;			// 火伤害
	F32			HuoDamage_gPc;		// 火伤害百分比

	S32			HuoDefence;			// 火伤害减免
	F32			HuoDefence_gPc;		// 火伤害减免百分比

	S32			TuDamage;			// 土伤害
	F32			TuDamage_gPc;		// 土伤害百分比

	S32			TuDefence;			// 土伤害减免
	F32			TuDefence_gPc;		// 土伤害减免百分比

	S32			JinDamage;			// 金伤害
	F32			JinDamage_gPc;		// 金伤害百分比

	S32			JinDefence;			// 金伤害减免
	F32			JinDefence_gPc;		// 金伤害减免百分比

	S32			ShuiDamage;			// 水伤害
	F32			ShuiDamage_gPc;		// 水伤害百分比

	S32			ShuiDefence;		// 水伤害减免
	F32			ShuiDefence_gPc;	// 水伤害减免百分比

	F32         Sheng_Damage_gPc;   // 种族伤害百分比
	F32         Fo_Damage_gPc;
	F32         Xian_Damage_gPc;
	F32         Jing_Damage_gPc;
	F32         Gui_Damage_gPc;
	F32         Guai_Damage_gPc;
	F32         Yao_Damage_gPc;
	F32         Mo_Damage_gPc;
	F32         Shou_Damage_gPc;
	F32         Long_Damage_gPc;
	S32         DeriveHP;//吸血
	S32         RecoverHP;//回血
	F32         RecoverHp_gPc;//回血百分比add
	S32         See;//视野
	S32         ReduceStr;//没回合消耗体力add

	S32         PhyImmunit;          // 物理伤害减免fer
	F32         PhyImmunit_gPc;      //

	U32         RemoteAtk;
	F32         RemoteAtk_gPc;

	F32         Reflect_gPc;





	Stats();
	void Clear();

	Stats& operator+=(const Stats&);
	Stats& operator-=(const Stats&);
	Stats& operator*=(const F32);

	// 最终计算
	void Calc(const Stats& _add);
	// 数值增强（用于状态、技能的增强）
	void Plus(const Stats& _orig, const Stats& _plus);
	// 增强后的值，+=和-=实际上是加上增强后的值
	void PlusDone();
	// 按比例叠加源的属性，有些值无需（不能）叠加
	void PlusSrc(const Stats& _orig, S32 ratePhy,  S32 rateSpl, S32 ratePnm,S32 rateQua);
	// 属性叠加，与+=相比增加了叠加数
	Stats& Add(const Stats& _add, S32 _count = 1);
	// npc按比列附加属性
	void percentNpc(CNpcData* pData);

	void percent(const F32 val);

	/*void printS()
	{
		using namespace std;
		cout<<"    HP:"    <<HP<<"    生命百分比:"    <<HP_gPc<<"    最大生命:"    <<MaxHP<<"    最大生命百分比:"    <<MaxHP_gPc<<endl;
		cout<<"    体力:"    <<Str<<"    体力百分比"    <<Str_gPc<<"    最大体力:"    <<MaxStr<<"    最大体力百分比"    <<MaxStr_gPc<<endl;
		cout<<"    暴击几率百分比:"    <<Cri_gPc<<"    普攻闪避几率"    <<Doe_gPc<<"    远程类武器闪避几率"    <<DoeR_gPc<<"    速度"    <<Spe<<endl;
		cout<<"    物理伤害"    <<PhyDamage<<"    物理伤害百分比"    <<PhyDamage_gPc<<"    物理防御"    <<PhyDefence<<"    物理防御百分比"    <<PhyDefence_gPc<<endl;
		cout<<"    木伤害"    <<MuDamage<<"    木伤害百分比"    <<MuDamage_gPc<<"    木防御"    <<MuDefence<<"    木防御百分比"    <<MuDefence_gPc<<endl;
		cout<<"    火伤害"    <<HuoDamage<<"    火伤害百分比"    <<HuoDamage_gPc<<"    火防御"    <<HuoDefence<<"    火防御百分比"    <<HuoDefence_gPc<<endl;
		cout<<"    土伤害"    <<TuDamage<<"    土伤害百分比"    <<TuDamage_gPc<<"    土防御"    <<TuDefence<<"    土防御百分比"    <<TuDefence_gPc<<endl;
		cout<<"    金伤害"    <<JinDamage<<"    金伤害百分比"    <<JinDamage_gPc<<"    金防御"    <<JinDefence<<"    金防御百分比"    <<JinDefence_gPc<<endl;
		cout<<"    水伤害"    <<ShuiDamage<<"    水伤害百分比"    <<ShuiDamage_gPc<<"    水防御"    <<ShuiDefence_gPc<<"    水防御百分比"    <<endl;
		cout<<"    种族伤害百分比"    <<Sheng_Damage_gPc<<"    Fo_Damage_gPc"    <<Fo_Damage_gPc<<"    Xian_Damage_gPc"    <<Xian_Damage_gPc<<endl;

		cout<<"    Jing_Damage_gPc"    <<Jing_Damage_gPc<<"    Gui_Damage_gPc"    <<Gui_Damage_gPc<<"    Yao_Damage_gPc"    <<Yao_Damage_gPc<<endl;
		cout<<"    Mo_Damage_gPc"    <<Mo_Damage_gPc<<"    Shou_Damage_gPc"    <<Shou_Damage_gPc<<"    Long_Damage_gPc"    <<Long_Damage_gPc<<
			endl;
		cout<<"    DeriveHP吸血"    <<DeriveHP<<"    RecoverHP"    <<RecoverHP<<"    RecoverHp_gPc"    <<RecoverHp_gPc<<"    See视野"    <<See<<"    没回合消耗体力add"    <<ReduceStr<<endl;
		cout<<"    物理伤害减免fer"    <<PhyImmunit<<"    PhyImmunit_gPc"    <<PhyImmunit_gPc<<"    RemoteAtk"    <<RemoteAtk<<"    RemoteAtk_gPc"    <<
			RemoteAtk_gPc<<"    Reflect_gPc"    <<Reflect_gPc<<endl;

	}*/
};

extern Stats g_Stats;

