#pragma once

#include "BaseData.h"

struct VIPData;

class CVIPDataRes : public CBaseDataRes<VIPData>
{
public:
	CVIPDataRes() = default;
	~CVIPDataRes();

	bool read();

};

struct VIPData 
{
	VIPData() : m_VipLevel(0), m_DrawGold(0), m_ImposeTopLimit(0), m_VIMTopLimit(0), m_StrengthTopLimit(0), m_ActivityTopLimit(0), 
		m_ImposePlus(0), m_PrinceNumberTopLimit(0), m_ApplyTopLimit(0), m_VSAReducePercent(0), m_CanJumpBattleAnim(0), m_VIPGiftId(0){}

	U8 m_VipLevel;	//VIP等级
	U32 m_DrawGold;	//累计充值
	U8 m_ImposeTopLimit;	//征收令上限
	U8 m_VIMTopLimit;		//精力上限
	U8 m_StrengthTopLimit;	//体力上限
	U8 m_ActivityTopLimit;	//活力上限
	U8 m_ImposePlus;		//征收加成
	U8 m_PrinceNumberTopLimit;	//皇子席位数量上限
	U8 m_ApplyTopLimit;		//奏章数量上限
	F32 m_VSAReducePercent;	//精活体减少时间
	U8 m_CanJumpBattleAnim;		//是否可以跳过战斗动画
	U32 m_VIPGiftId;			//vip礼包id
};

