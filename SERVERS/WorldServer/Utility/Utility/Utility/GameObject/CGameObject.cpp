//
//  CGameObject.cpp
//  HelloLua
//
//  Created by BaoGuo on 14-1-18.
//
//

#include "CGameObject.h"
#include "CNpcObject.h"
#include <cstdlib>
#include <ctime>
#include "Common/mRandom.h"

const float EPSINON=0.0001;
CGameObject::CGameObject():CGameBase(), mMask(0), m_Level(1),\
                        m_Family(0), m_Exp(0)
{
    m_BuffTable.SetObj(this);
}

CGameObject::~CGameObject()
{
}

U32 CGameObject::getObjectType()
{
    return GAME_OBJECT;
}

void CGameObject::DoAddHp(S32 hp)
{
    if (m_BuffTable.GetEffect1()&Effect1_CantAffortAtk &&  GetStats().HP>1 && GetStats().HP+hp<=1 )
    {
        addHP(-GetStats().HP+1); //只剩一点血 //二命
      
    }else
        addHP(hp);
    
    if(GAME_PLAYER == (en_GameObjectType)getObjectType())
    {
        if (m_BuffTable.GetEffect1() & Effect1_Slumber)//睡眠  受攻击清醒
        {
            //setMask(ROUND_END_MASK);
            //RemoveBuff(4001049);
            m_BuffTable.AddBuff(Buff::Origin_Buff, 4001049, this,-1);  //移除
           // m_BuffTable.r
        }
    }
    else
    {
        m_BuffTable.AddBuff(Buff::Origin_Buff, 4001049, this,-1); //移除
    }
    stPoint grid = getGridPosition();
    if(GetStats().HP == 0)
    {
        if (m_BuffTable.GetEffect1() & Effect1_NotDeath)
            return;  //不死
        
        setMask(DEATH_MASK);
        return;
    }
    
}

void CGameObject::setMask(U32 mask)
{
    if(mMask & mask)
    {
        return;
    }
    
    mMask |= mask;
}

void CGameObject::removeMask(U32 mask)
{
    if(mMask & mask)
    {
        mMask = mMask & (~mask);
    }
}

void CGameObject::timeSignal(float dt)
{
    m_BuffTable.Process(dt);
    
    if (m_BuffTable.GetEffect1() & Effect1_Stun || m_BuffTable.GetEffect1()& Effect1_NoFeet)//昏迷 禁足
    {  
        setMask(ROUND_END_MASK);  // todo 动作
    }
    
    CalcStats();
}

void CGameObject::Attack(CGameObject* obj)
{
    setMask(ATTACK_MASK);
    //释放普通攻击
    SpellCast(this, obj, NORMAL_ATTACK_ID);
}

void CGameObject::Spell(CGameObject *obj, U32 SkillId)
{
    setMask(SPELL_MASK);
    SpellCast(this, obj, SkillId);
}

bool  CGameObject::operator < (const CGameObject& other)
{
    return getGridPosition() < other.getGridPosition();
}

bool CGameObject::AddBuff(Buff::Origin _origin, U32 _buffId, CGameObject* _src, S32 _count /* = 1 */, U32 _time /* = 0 */, Vector<U32> *_list /* = NULL */ , U32 rate /* = 10000*/)
{
    return m_BuffTable.AddBuff(_origin, _buffId, _src, _count, _time, _list, rate);
}

bool CGameObject::RemoveBuff(U32 _buffId, Vector<U32> *_list /* = NULL */)
{
    return m_BuffTable.RemoveBuff(_buffId, _list);
}

bool CGameObject::RemoveBuffBySrc(U32 _buffId, CGameObject* src, U32 _count /* = 1 */, U32 mask /* = Buff::Mask_Remove_Sys */)
{
    return m_BuffTable.RemoveBuffBySrc(_buffId, src, _count, mask);
}

void CGameObject::CalcStats()
{
    m_BuffTable.CalculateStats();
}

bool CGameObject::CalculateDamage(Stats& stats, CGameObject* pSource, U32 buffId, Vector<U32>* pPlusList)
{
    // 先清除属性
	stats.Clear();
    
	if(0 == buffId)
		return false;
    
	CBuffData* pBuffData = g_BuffDataMgr->GetBuff(buffId);
	if(!pBuffData)
		return false;
    
	// 状态本身属性
	stats.Plus(stats, pBuffData->GetStats());

	if(pPlusList)
	{
		CBuffData* pPlus;
		for (S32 i=0; i<pPlusList->size(); i++)
		{
			pPlus = g_BuffDataMgr->GetBuff((*pPlusList)[i]);
			if(pPlus)
				stats.Plus(stats, pPlus->m_Stats);
		}
	}
	stats.PlusDone();
	// 是否叠加施放者属性
	//if(pBuffData->IsFlags(CBuffData::Flags_PlusSrc) && pSource)
	//	stats.PlusSrc(pSource->m_BuffTable.GetStats(), pBuffData->m_PlusSrcRatePhy,  pBuffData->m_PlusSrcRateSpl, pBuffData->m_PlusSrcRatePnm,pBuffData->m_PlusSrcRateQua);
	return true;
}

void CGameObject::setFamily(U8 val)
{
    if(m_Family == val)
        return;
    
    m_Family = val;
}

void CGameObject::SpellDamege(CGameObject* pSource, CGameObject* pTarget, U32 buffId, S32 buffCount/*= 1*/, Vector<U32>* pPlusList/*= NULL*/)
{
    if(!pTarget || !CalculateDamage(g_Stats, pSource, buffId, pPlusList))
        return;
    
    if(buffCount > 1)
        g_Stats *= buffCount;

    pTarget->ApplyDamage(g_Stats, pSource, buffId);
}


void CGameObject::SpellCast(CGameObject* pSource, CGameObject* pTarget, U32 buffId, S32 buffCount/*= 1*/)
{
    if(!pSource || !pTarget)
        return;
    
    CBuffData* pData = g_BuffDataMgr->GetBuff(buffId);
    if(!pData)
        return;
    U32 damageType = pData->GetDamageType();
    if(damageType == 0)
    {
        pTarget->AddBuff(Buff::Origin_Buff, buffId, pSource);
    }
    else
    {
        if(buffId == NORMAL_ATTACK_ID)
            SpellDamege(pSource, pTarget, buffId);
        else
            pTarget->AddBuff(Buff::Origin_Buff, buffId, pSource);
    }
   
}



bool CGameObject::isDamageable() const
{
    //交互npc没有伤害
    return true;
}

bool CGameObject::HitFilter(CGameObject* pSource, const Stats& stats, U32 exFlags /* = 0 */)
{
	// 不可反射的伤害必定命中（即反弹伤害必然命中）
    
    if(pSource)
    {
        // 必中时忽略闪避
        //if(pSource->GetBuffTable().GetEffect1() & CBuffData::Effect_GuaranteeHit)
        //    return true;
        S32 randNm=gRandGen.randI(0,100);
		if((getMax(abs(int(GetStats().Doe_gPc * 100)),0) >= randNm))
		{
			//闪避掉了
			return false;
		}
    }
    // 禁止闪避
    //if(m_BuffTable.GetEffect() & BuffData::Effect_AntiDodge)
    //    return true;
    return true;
}

bool CGameObject::CriticalFilter(CGameObject* pSource, const Stats& stats, U32 damType)
{
    if(pSource)
    {
		//if(GetBuffTable().GetEffect1() & BITU64(CBuffData::Effect_DisableFiter))
		//	return false;
        // 必暴时忽略防暴
        //if(pSource->GetBuffTable().GetEffect() & BuffData::Effect_GuaranteeCritical)
        //    return true;
		// 治疗限定了暴击几率
		if(damType & CBuffData::Damage_Heal)
			return (gRandGen.randI(1,10000) <= 500);
		
		S32 Cri_gPc = (GetStats().Cri_gPc) * 100;
        
        if(Cri_gPc >= gRandGen.randI(1,100))
            return true;
    }
    return false;
}

bool CGameObject::OneshortFilter(CGameObject* pSource, const Stats& stats,U32 damtype)
{
    if (pSource->GetBuffTable().GetEffect1()& Effect1_Deadly) //致死
        return true;
    else
        return false;
}

S32 CGameObject::getDamage(S32& dam, S32& _dam, S32& damImt)
{
    if(dam <= _dam)
    {
        dam = 1;
        return dam;
    }
    else
        dam -= _dam;
    
    //伤害减免
    if(dam <= damImt)
        dam = 1;
    else
        dam -= damImt;
    
    return dam;
}

S32 CGameObject::DamageReduce(const Stats& stats, CGameObject* pSource, U32 damType, CBuffData* pData, S32& reduce)
{
    static S32 dam, damPhy, damHuo, damShui, damJin, _dam, _damPhy, __damImt, __damHuo, __damShui, __damJin;
    F32 familyAddDamage = 0.0f;
    S32 _NoDam = 0;
    // 无敌
    if(m_BuffTable.GetEffect1() & Effect1_PhyDefMax)
	{
        dam = mClamp(dam, 0, 1);
        return dam;
	}
    
    // 计算初始伤害
    damPhy  = damType & CBuffData::Damage_Phy ? stats.PhyDamage + pSource->GetStats().PhyDamage : 0;
    //damSpl = damType & CBuffData::Damage_Mu ? stats.MuDamage + pSource->GetStats().MuDamage : 0;
    damHuo  = damType & CBuffData::Damage_Huo ? stats.HuoDamage + pSource->GetStats().HuoDamage: 0;
    //damSpl += damType & CBuffData::Damage_Tu ? stats.TuDamage  + pSource->GetStats().TuDamage: 0;
    damShui = damType & CBuffData::Damage_Jin ? stats.JinDamage + pSource->GetStats().JinDamage : 0;
    damJin = damType & CBuffData::Damage_Shui ? stats.ShuiDamage + pSource->GetStats().ShuiDamage: 0;
    
    
    _damPhy  = damType & CBuffData::Damage_Phy ? GetStats().PhyDefence : 0;
    //伤害减免
    //_damSpl  = damType & CBuffData::Damage_Mu ? GetStats().MuDefence : 0;
    __damHuo  = damType & CBuffData::Damage_Huo ? GetStats().HuoDefence : 0;
    //_damSpl += damType & CBuffData::Damage_Tu ? GetStats().TuDefence : 0;
    __damShui += damType & CBuffData::Damage_Jin ? GetStats().JinDefence : 0;
    __damJin += damType & CBuffData::Damage_Shui ? GetStats().ShuiDefence : 0;
   
    //伤害减免
    __damImt = stats.PhyImmunit + pSource->GetStats().PhyImmunit;
    if(damPhy != 0)
    {
        if(GetBuffTable().GetEffect1() & Effect1_ImmunitPhy) //免伤
            damPhy=1;
        else
            damPhy = getDamage(damPhy, _damPhy, __damImt);
    }
    
    if(damHuo !=0)
    {
        damHuo = getDamage(damHuo, _NoDam, __damHuo);
        damHuo = getMax(damHuo * (1.0f + GetStats().HuoDefence_gPc), 1.0f);
    }
    
    if(damShui != 0)
    {
        damShui = getDamage(damShui, _NoDam, __damShui);
        damShui = getMax(damShui * (1.0f + GetStats().ShuiDefence_gPc), 1.0f);
    }
    
    if(damJin != 0)
    {
        damJin = getDamage(damJin, _NoDam, __damJin);
        damJin = getMax(damJin * (1.0f + GetStats().JinDefence_gPc), 1.0f);
    }

    dam = damPhy + damHuo + damShui + damJin;
    
    //种族伤害加成
    familyAddDamage = m_BuffTable.getFamilyAddDamage(getFamily());
    
    if(familyAddDamage != 0.0f)
    {
        dam = floor(getMax((dam)* (1.0f + familyAddDamage), 0.0f));
    }
    
    // 伤害和治疗调整
    return dam;
}

void CGameObject::ApplyDamage(const Stats& stats, CGameObject* pSource, U32 buffId, U32 exFlags /* = 0; */)
{
    U32 damType, combatMask;
    S32 dam = 0, reduce = 0;
    bool bCritical = false;//是否暴击
    
    CBuffData* pData = g_BuffDataMgr->GetBuff(buffId);
    if(!pData || isDeath())
        return;
    
    // ------------------------------------- 是否受到伤害 ------------------------------------- //
    if(!isDamageable())
        return;
    
	if((getObjectType()&GAME_NPC))
	{
	}
    
	// ------------------------------------- 反射 ------------------------------------- //
    /*if ((m_BuffTable.GetEffect() & CBuffData::Effect_Reflect) && !pData->IsFlags(BuffData::Flags_DoNotReflect) && !(BuffData::Flags_DoNotReflect & exFlags) && pSource)
	{
		pSource->ApplyDamage(stats, pSource, buffId, BuffData::Flags_DoNotReflect | exFlags);
		return;
	}*/
    
    // ------------------------------------- 免疫伤害 ------------------------------------- //
    combatMask = 0;
    damType = pData->GetDamageType();
	// 注意魔免、物免都要去除元力伤害
   
    if(GetBuffTable().GetEffect1() & Effect1_ImmunitSpl)
        damType &= ~(CBuffData::Damage_Spl);
    // 发送免疫消息
	//属性攻击免疫保留
    //....
	//免疫伤害
    //if(pData->GetDamageType() != damType)
    //    combatMask |= CombatLogEvent::CombatMask_Immunity;
    
    if(0 == damType)
    {
        
        // 已完全免疫，直接退出
        return;
    }
    
    // ------------------------------------- 计算命中闪避 ------------------------------------- //
	// 如果只是治疗，不会计算闪避
	if((damType & (CBuffData::Damage_Phy | CBuffData::Damage_Spl)) && !HitFilter(pSource, stats, exFlags))
    {
       
        return;
    }
    
    // ------------------------------------- 计算暴击 ------------------------------------- //
    if(CriticalFilter(pSource, stats, damType))
    {
            
       
        bCritical = true;
    }
	//--------------------------------------计算必杀-如果出必杀就跳过战斗公式计算---------------
	if(OneshortFilter(pSource,stats,damType)) //致死
	{
        setMask(DEATH_MASK);
	}
	else
        // ------------------------------------- 计算减免 ------------------------------------- //
	{
		dam = DamageReduce(stats, pSource, damType, pData, reduce);
        //减少耐久
		//onReduce(dam, reduce, pSource);
        // ------------------------------------- 仇恨系统 ------------------------------------- //
        
        // ------------------------------------- 状态相关触发 ------------------------------------- //
        if(m_BuffTable.onTriggerGroup(pData, pSource))
            return;
	}
    
    if(bCritical)
    {
        dam *= 2;
    }
    
    // ------------------------------------- 可否致死 ------------------------------------- //

    
	// ------------------------------------- 应用伤害 ------------------------------------- //
    
    
    stPoint ownGrid=pSource->getGridPosition();
    stPoint grid=getGridPosition();
    DoAddHp(-dam);
    
    //战斗提示
    
    if(pSource->GetStats().DeriveHP>0)
        pSource->addHP(pSource->GetStats().DeriveHP);
    
	// ------------------------------------- 吸血 ------------------------------------- //
}

void CGameObject::addHP(S32 val)
{
    if(val != 0 && (val > 0 ? GetStats().HP != GetStats().MaxHP : GetStats().HP != 0))
    {
        m_BuffTable.addHP(val);
    }
}

void CGameObject::setHP(S32 val)
{
    if(GetStats().HP != val)
    {
        m_BuffTable.setHP(val);
    }
}