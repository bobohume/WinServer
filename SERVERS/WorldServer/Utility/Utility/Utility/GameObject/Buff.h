//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FireRain.com, Inc.
//-----------------------------------------------------------------------------

#pragma once
#include "Base/types.h"
#include "Stats.h"
#include "../../Data/BuffData.h"
#include "BASE/tVector.h"
#include <vector>
#include <hash_map>
#include "CGameBase.h"
#include "BASE/mMathFn.h"
class CGameObject;
class Player;

class Buff
{
public:

	enum Origin
	{
		Origin_Base = 0,				// 自身属性
		Origin_Buff,					// 状态
		Origin_Equipment,				// 装备
		Origin_Level,                   // 等级附加buff，npc这个会有npc加成，目前加成只有等级。
	};

	enum
	{
		Mask_Remove_Sys = BIT(0),		// 跟其他移除标志不同即可
		Mask_Remove_Common = BIT(1),	// 普通
	};

	CBuffData*	m_BuffData;				// 状态
	Stats		m_BuffStats;			// 状态经过加成后的最终属性

	Origin		m_Origin;				// 这个状态的来源
	bool		m_Self;					// 这个状态的释放者是否为自身
	U32			m_Count;				// 叠加数
	U32			m_RemainTimes;			// 剩余次数
	U32			m_LimitTimes;			// 总次数
	bool        m_bRemove;			// 移除标志对应BuffData::Trigger_RemoveA~D
	bool        m_bNeedProcess;

	Vector<U32> m_Plus;					// 记录相关天赋对该状态的加成
	SimObjectPtr<CGameObject> m_pSource;	// 释放这个状态的对象

	Buff();
	~Buff();

	//删除特效
	void Remove(CGameObject* pObj);
	bool Process(CGameObject* pObj, float dt);//回合推进
	void ProcessTick(CGameObject* pObj);//触发伤害buff

	// 触发(预留)
	void onTrigger(U32 mask, CGameObject* carrier/*本体*/, CGameObject* trigger);
private:
};


// ========================================================================================================================================
//  BuffTable
// ========================================================================================================================================
class BuffTable
{
public:
	typedef Vector<Buff*> BuffTableType;
	typedef stdext::hash_multimap<U32, Buff*> BuffTableMap;

private:
	// 是否需要刷新
	bool m_Refresh;
	// 人物最终属性
	Stats m_Stats;

	// 人物属性总和（未经最终计算）
	Stats m_Summation;

	// 这些不会推进，只是记录某些信息（触发的内置CD等）
	// 其中Buff::m_Plus和主角的PlusTable一致
	// 为节约内存，合并到一个hash表
	BuffTableMap m_General;

	// 这些状态会推进
	BuffTableType m_Buff;

	// 拥有这些状态的对象
	CGameObject* m_pObj;

	// 所有状态效果的集合
	U64 m_EffectMask1;
	U64 m_EffectMask2;

public:

	BuffTable();
	~BuffTable();
	void Clear();
	inline void SetObj(CGameObject* pObj) {m_pObj = pObj;}
	inline bool GetRefresh() { return m_Refresh; }
	inline void SetRefresh(bool val) { m_Refresh = val; }
	inline const Stats& GetStats() { return m_Stats; }
	inline const Stats& GetSummation() {return m_Summation; }
	inline U64 GetEffect1() const { return m_EffectMask1; }
	inline U64 GetEffect2() const { return m_EffectMask2; }
	F32  getFamilyAddDamage(U8 family);

	bool onTriggerGroup(CBuffData* pBuffData, CGameObject* src);
	bool AddBuff(Buff::Origin _origin, U32 _buffId, CGameObject* _src, S32 _count = 1, U32 _time = 0, Vector<U32> *_list = NULL, U32 rate = 10000);
	// 去除m_General中的数值
	bool RemoveBuff(U32 _buffId, Vector<U32> *_list = NULL);

	// 去除m_Buff、m_Debuff、m_System中的状态
	bool RemoveBuff(BuffTableType& _buffTab, U32 _buffId, U32 index, U32 _count = 1, U32 mask = Buff::Mask_Remove_Sys);

	bool RemoveBuffBySrc(U32 _buffId, CGameObject* src, U32 _count = 1, U32 mask = Buff::Mask_Remove_Sys);
	void ClearBuff();												// 清除所有buff

	void Process(float dt);
	//npc加强
	bool CalculateNpcStats(CBuffData& pData);
	bool CalculateStats();

	bool checkBuff(U32 id, S32 count, bool seriesId = false) const;	// 检查m_Buff、m_Debuff、m_System中的状态
	U32 getBuffCount(U32 _buffId) const;	// 检查m_Buff、m_Debuff、m_System中的状态

	template<class T, class V>
	static bool greateSort(T Itr, V val)
	{
		return Itr.first < val;
	}

	template<class V, class T>
	static bool greateSort1(V val, T Itr)
	{
		return Itr.first < val;
	}

	// 设置函数
	inline void setHP(S32 val) { m_Stats.HP = mClamp(val, 0, m_Stats.MaxHP);}
	inline void setStr(S32 val) { m_Stats.Str = mClamp(val, 0, m_Stats.MaxStr);}
	inline void addHP(S32 val) { m_Stats.HP = mClamp(m_Stats.HP + val, 0, m_Stats.MaxHP);}
	inline void addStr(S32 val) { m_Stats.Str = mClamp(m_Stats.Str + val, 0, m_Stats.MaxStr);}
	inline void setMaxHP(S32 val) {m_Stats.MaxHP=val;}
	inline void addMaxHP(S32 val) {m_Stats.MaxHP+=val;}

	// 取得m_Buff、m_Debuff
	const inline BuffTableType& GetBuff()  const { return m_Buff; }

	// buff内部接口（随机触发其他负面属性）
	bool CalcAddAbnormity(CGameObject* pSource, CBuffData *pData, U32 rate);
};


