//
//  CGameObject.h
//  HelloLua
//
//  Created by BaoGuo on 14-2-20.
//
//

#ifndef HelloLua_CGameObject_h
#define HelloLua_CGameObject_h
#include "Base/types.h"
#include <set>
#include <hash_map>
#include <hash_set>
#include "stdlib.h"
#include "string.h"
#include "CGameBase.h"
#include "Buff.h"


//对象类
class CGameObject:public CGameBase
{
public:
	const static U32 DEATH_MASK         = BIT(0);
	const static U32 PLAY_ACTION_MASK   = BIT(1);
	const static U32 WALK_MASK          = BIT(2);
	const static U32 ATTACK_MASK        = BIT(3);
	const static U32 SPELL_MASK         = BIT(4);
	const static U32 ROUND_END_MASK     = BIT(5);//用于推动npc逻辑，回合制
	const static U32 OPERATOR_END_MASK  = BIT(6);//用于是否可以操作
	const static U32 NEXT_MASK          = BIT(7);


	enum{
		NORMAL_ATTACK_ID    = 4700001,
		MAX_ATTACK_DISTANCE = 1,
		MAX_LEVEL           = 200,
		MAX_PATH_SIZE       = 20,//此处只记最大房间的所拥有的格子数就可以了
	};

	typedef CGameBase Parent;
	typedef std::set<stPoint> GRIDSET;
	typedef GRIDSET::iterator GRIDITR;;

	virtual ~CGameObject();
	virtual U32 getObjectType();
	virtual void timeSignal(float dt);
	virtual void Attack(CGameObject* obj);
	virtual void Spell(CGameObject* obj, U32 SkillId);
	virtual void Walk(stPoint grid);
	virtual void DoAddHp(S32 hp);
	virtual void CalcStats();
	virtual void death() {};

	void setMask(U32 mask);
	void removeMask(U32 mask);
	virtual bool isDeath() { return mMask & DEATH_MASK; }
	void setIcon(const char* fileName);
	inline bool isPlayAction() {return mMask & PLAY_ACTION_MASK; }
	inline bool isWalking() { return mMask & WALK_MASK; }
	inline bool isAttacking() { return mMask & ATTACK_MASK; }
	inline bool isSpelling()  { return mMask & SPELL_MASK; }
	inline bool isRoundEnd() { return mMask & ROUND_END_MASK; }
	inline bool isOperator() { return mMask & OPERATOR_END_MASK; }
	inline void PlayActionOver() { removeMask(PLAY_ACTION_MASK); }

	void RoundEnd(bool bFlag = true);
	//begin buff
	BuffTable& GetBuffTable() { return m_BuffTable; };
	const Stats& GetStats() { return m_BuffTable.GetStats(); }
	bool AddBuff(Buff::Origin _origin, U32 _buffId, CGameObject* _src, S32 _count = 1, U32 _time = 0, Vector<U32> *_list = NULL, U32 rate = 10000);
	bool RemoveBuff(U32 _buffId, Vector<U32> *_list = NULL);
	bool RemoveBuffBySrc(U32 _buffId, CGameObject* src, U32 _count = 1, U32 mask = Buff::Mask_Remove_Sys);

	void setHP(S32 val);

	inline void setStr(S32 val) { if(GetStats().Str != val) {m_BuffTable.setStr(val);}}
	void addHP(S32 val);
	inline void addStr(S32 val) { if(val != 0 && (val > 0 ? GetStats().Str != GetStats().MaxStr : GetStats().Str != 0)) {m_BuffTable.addStr(val);}}
	inline U8   getFamily() { return m_Family; }
	inline U32  getLevel()  { return m_Level; }
	inline U64  getExp()    { return m_Exp; }
	void setFamily(U8 val);
	virtual void addExp(S64 exp) {};
	virtual void addLevel(S32 level) {};
	//end buff
	static void SpellDamege(CGameObject* pSource, CGameObject* pTarget, U32 buffId, S32 buffCount = 1, Vector<U32>* pPlusList = NULL);
	static bool	CalculateDamage	(Stats& stats, CGameObject* pSource, U32 buffId, Vector<U32>* pPlusList);
	static void SpellCast(CGameObject* pSource, CGameObject* pTarget, U32 buffId, S32 buffCount = 1);
	void ApplyDamage(const Stats& stats, CGameObject* pSource, U32 buffId, U32 exFlags = 0);
	S32 DamageReduce(const Stats& stats, CGameObject* pSource, U32 damType, CBuffData* pData, S32& reduce);
	S32 getDamage(S32& dam, S32& _dam, S32& damImt);
	virtual bool isDamageable() const;
	//是否命中
	bool HitFilter(CGameObject* pSource, const Stats& stats, U32 exFlags = 0);
	//是否暴击
	bool CriticalFilter(CGameObject* pSource, const Stats& stats, U32 damType);
	//必杀
	bool OneshortFilter(CGameObject* pSource, const Stats& stats, U32 damType);
	bool operator < (const CGameObject& other);
protected:
	CGameObject();
	BuffTable m_BuffTable;//数值属性，状态列表 
	U32 mMask;
	U32 m_Level;
	U8  m_Family;
	U64 m_Exp;
};

#endif
