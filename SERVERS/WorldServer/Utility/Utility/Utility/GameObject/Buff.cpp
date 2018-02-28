#include "Buff.h"
#include "CGameObject.h"
#include "CommLib/CommLib.h"
#include "Common/PlayerStruct.h"
#include "CNpcObject.h"
#include "Common/OrgBase.h"

Buff::Buff():
m_BuffData(NULL),
m_Origin(Origin_Base),
m_Self(true),
m_Count(3),
m_RemainTimes(0),
m_LimitTimes(0),
m_bRemove(0)
{
}

Buff::~Buff()
{
    //CC_SAFE_DELETE(m_effect);
}

void Buff::Remove(CGameObject* pObj)
{
    m_bRemove = true;
}

bool Buff::Process(CGameObject* pObj, float dt)
{
    if(m_bRemove)
        return false;
    if(m_Count == 0 || m_BuffData->m_LimitTimes == 0 || pObj->isDeath())
	{
		Remove(pObj);
		return false;
	}
    
    if(pObj->isRoundEnd() && m_bNeedProcess)
    {
        ProcessTick(pObj);
        --m_RemainTimes;
        m_bNeedProcess = false;
    }
    else if(!pObj->isRoundEnd())
    {
        m_bNeedProcess = true;
    }
    
    if (0==m_RemainTimes && pObj->GetStats().HP == 0 && pObj->GetBuffTable().GetEffect1()& Effect1_NotDeath )//不死
    {
            pObj->setMask(CGameObject::DEATH_MASK);
    }
    
    
	if(0 == m_RemainTimes)
		Remove(pObj);
	return (bool)m_RemainTimes;
}

void Buff::ProcessTick(CGameObject* pObj)
{
   
	if(m_BuffData)
	{
		g_Stats.Clear();
		if(m_BuffData->GetDamageType())
        {
            //if(g_ClientGameplayState->hasGameObject(m_pSource))
            {
                CGameObject::SpellDamege(m_pSource, pObj, m_BuffData->GetBuffId());
            }
        }
        else if(m_BuffStats.RecoverHP)
        {
            pObj->addHP(m_BuffStats.RecoverHP);
        }
        else if(m_BuffStats.RecoverHp_gPc)
        {
            pObj->addHP(pObj->GetStats().MaxHP * m_BuffStats.RecoverHp_gPc);
        }else if(pObj->GetBuffTable().GetEffect1()&Effect1_Poisoning) //中毒
        {
            pObj->addHP(pObj->GetStats().MaxHP*0.1);
        }else if (pObj->GetBuffTable().GetEffect1()&Effect1_KnockDown)//击倒 倒地
        {
            //pObj->GetStats().PhyDefence=0;
        }
        
        if (pObj->GetBuffTable().GetEffect2()&Effect2_CrossByHp) //穿越
        {
           pObj->addHP(-pObj->GetStats().MaxHP * 0.1);
        }
   
	}
}

void Buff::onTrigger(U32 mask, CGameObject* carrier, CGameObject* trigger)
{
	
}

// ========================================================================================================================================
//  BuffTable
// ========================================================================================================================================
BuffTable::BuffTable() :
m_pObj(NULL),
m_EffectMask1(0),
m_EffectMask2(0),
m_Refresh(false)
{
}

BuffTable::~BuffTable()
{
	Clear();
}

void BuffTable::Clear()
{
	BuffTableMap::iterator itr;
	for(itr = m_General.begin(); itr != m_General.end(); ++itr)
		delete itr->second;
	S32 i;
	for(i = m_Buff.size()-1; i >= 0; --i)
    {
		SAFE_DELETE(m_Buff[i]);
    }
    
	m_General.clear();
	m_Buff.clear();
	m_Stats.Clear();
	m_Summation.Clear();
	m_Refresh = false;
	m_EffectMask1 = 0;
    m_EffectMask2 = 0;
}

F32  BuffTable::getFamilyAddDamage(U8 family)
{
    enFamily enFam = (enFamily)family;
    
    if(Family_Sheng == enFam)
    {
        return m_Stats.Sheng_Damage_gPc;
    }
    else if(Family_Fo == enFam)
    {
        return m_Stats.Fo_Damage_gPc;
    }
    else if(Family_Xian == enFam)
    {
        return m_Stats.Xian_Damage_gPc;
    }
    else if(Family_Jing == enFam)
    {
        return m_Stats.Jing_Damage_gPc;
    }
    else if(Family_Gui == enFam)
    {
        return m_Stats.Gui_Damage_gPc;
    }
    else if(Family_Guai == enFam)
    {
        return m_Stats.Guai_Damage_gPc;
    }
    else if(Family_Yao == enFam)
    {
        return m_Stats.Yao_Damage_gPc;
    }
    else if(Family_Mo == enFam)
    {
        return m_Stats.Mo_Damage_gPc;
    }
    else if(Family_Shou == enFam)
    {
        return m_Stats.Shou_Damage_gPc;
    }
    else if(Family_Long == enFam )
    {
        return m_Stats.Long_Damage_gPc;
    }
}

bool BuffTable::onTriggerGroup(CBuffData* pBuffData, CGameObject* src)
{
    if(!pBuffData->m_Group)
        return false;
    
    for(S32 i=m_Buff.size()-1; i>=0; --i)
    {
        if(!m_Buff[i] || m_Buff[i]->m_bRemove)
            continue;
        // 只移除相同来源的状态
        //if(pBuffData->IsFlags(BuffData::Flags_RemoveSameSrc) && m_Buff[i]->m_pSource != src)
        //    continue;
        
        U32 Group = m_Buff[i]->m_BuffData->m_Group;
        if((pBuffData->m_RemoveGroup & Group))
        {
            m_Buff[i]->Remove(m_pObj);
        }
        if(pBuffData->m_ImmunitGroup & Group)
        {
            m_Buff[i]->Remove(m_pObj);
        }
    }
    return false;
}

// 注意这个函数不能改变已有BUFF的位置，否则将会有严重的错误
bool BuffTable::AddBuff(Buff::Origin _origin, U32 _buffId, CGameObject* _src, S32 _count /* = 1 */, U32 _time /* = 0 */, Vector<U32> *_list /* = NULL */ , U32 rate /* = 10000*/)
{
	CBuffData* pBuffData = g_BuffDataMgr->GetBuff(_buffId);
	if(!pBuffData || !m_pObj || !_count)
		return false;
    
    //免疫伤害
	//if (CalcAddAbnormity(_src,pBuffData,rate))
	{
		//免疫伤害
		//return false;
	}
    
	// 如果是重复性施放
	BuffTableType* pTable = NULL;
	S32 newCount = _count;
	U32 pos = 0;
	if(_origin == Buff::Origin_Buff)
	{
		// 免疫
        {
            
        }
        
		// 直接产生伤害
		{
            
        }
        
		// 确定属于哪个列表
        pTable = &m_Buff;
        
		pos = pTable->size();
		// 遍历所有状态，查找是否有相同对象施放了相同的状态
		for(S32 i=pTable->size()-1; i>=0; --i)
		{
			Buff* pBuff = (*pTable)[i];
			if(pBuff->m_BuffData->GetBuffId() == _buffId)
			{
				pos = i;
				// 叠加删除前的状态个数
				newCount = mClamp(_count + pBuff->m_Count, 0, pBuffData->m_LimitCount);
				// 只是状态叠加减少
				if(_count < 0 && newCount > 0){
					RemoveBuff((*pTable), pBuff->m_BuffData->GetBuffId(), i, mAbs(_count), Buff::Mask_Remove_Sys);
					return true;
				}
				else{
					RemoveBuff((*pTable), pBuff->m_BuffData->GetBuffId(), i, pBuff->m_Count, Buff::Mask_Remove_Sys);
					break;
				}
			}
		}
		if(newCount <= 0)
			return true;
        
        // 有组触发，且要求删除自己
        if(onTriggerGroup(pBuffData, _src))
			return true;
        
        
	}
    
	Buff *pBuff = new Buff;
	pBuff->m_BuffData = pBuffData;
	pBuff->m_Origin = _origin;
	pBuff->m_pSource = _src;
	pBuff->m_Self = (m_pObj == _src);
    
	// 计算属性增强
	g_BuffData.Clear();
	g_BuffData.Plus(g_BuffData, *pBuffData);
    
    //这段为了计算自身属性
    if(_origin == Buff::Origin_Level)
    {
        if(GAME_MONSTER == (en_GameObjectType)_src->getObjectType())
        {
            CalculateNpcStats(g_BuffData);
        }
    }
    
    //这一段计算强化单独附加的属性
	if (_list && _origin == Buff::Origin_Equipment)
	{
		CBuffData* pPlus;
		for (S32 i=0; i<_list->size(); i++)
		{
			pPlus = g_BuffDataMgr->GetBuff((*_list)[i]);
			if (pPlus)
			{
				g_BuffData.Plus(g_BuffData, *pPlus);
				pBuff->m_Plus.push_back(pPlus->m_BuffId);
			}
		}
	}
    
	g_BuffData.PlusDone();
	pBuff->m_BuffStats.Clear();
	pBuff->m_BuffStats += g_BuffData.GetStats();
	// DOT HOT 可能需要叠加人物属性
	//if(pBuffData->IsFlags(BuffData::Flags_PlusSrc) && _src)
	//	pBuff->m_BuffStats.PlusSrc(_src->GetStats(), pBuffData->m_PlusSrcRatePhy,  pBuffData->m_PlusSrcRateSpl, pBuffData->m_PlusSrcRatePnm,pBuffDatåa->m_PlusSrcRateQua);
    
	// 被天赋增强的属性
	pBuff->m_Count = mClamp(newCount, 1, 10);
	pBuff->m_RemainTimes = pBuff->m_LimitTimes = g_BuffData.m_LimitTimes;
    
	if(pTable)
	{
		pTable->insert(pos);
		(*pTable)[pos] = pBuff;
	}
	else
	{
		m_General.insert(BuffTableMap::value_type(_buffId, pBuff));
	}
    
	if(pBuffData->IsFlags(CBuffData::Flags_CalcStats))
	{
		m_Summation.Add(pBuff->m_BuffStats, pBuffData->IsFlags(CBuffData::Flags_PlusStats) ? pBuff->m_Count : 1);
		m_Refresh = true;
	}

	// 调整剩余时间
	if(_time > 0 && _time < pBuff->m_RemainTimes)
	{
		pBuff->m_RemainTimes = _time;
	}

    
	return true;
}

bool BuffTable::RemoveBuff(U32 _buffId, Vector<U32> *_list /* = NULL */)
{
	// buff通过其他方法删除
	CBuffData* pBuffData = g_BuffDataMgr->GetBuff(_buffId);
	if(!pBuffData || !m_pObj)
		return false;
    
	BuffTableMap::iterator itr = m_General.lower_bound(_buffId);
	BuffTableMap::iterator end = m_General.upper_bound(_buffId);
	for ( ; itr!=end; ++itr)
	{
		if (!itr->second)
			continue;
		if (_list)
		{
			if (itr->second && itr->second->m_Plus.size() == _list->size() && !memcpy(_list->address(), itr->second->m_Plus.address(), _list->size() * sizeof(U32)))
				break;
		}
		else
		{
			if (itr->second && (itr->second->m_Plus.empty()))
				break;
		}
	}
	if(itr == end || !itr->second)
	{
		DOASSERT(false, "BuffTable::RemoveBuff !");
		return false;
	}
	// 移除属性
	Buff* pBuff = itr->second;
	if(pBuff->m_BuffData->IsFlags(CBuffData::Flags_CalcStats))
	{
		m_Summation.Add(pBuff->m_BuffStats, pBuffData->IsFlags(CBuffData::Flags_PlusStats) ? -(S32)pBuff->m_Count : -1);
		m_Refresh = true;
	}
    
    //删除特效
	delete pBuff;
	m_General.erase(itr);
	return true;
}

bool BuffTable::RemoveBuff(BuffTableType& _buffTab, U32 _buffId, U32 index, U32 _count /* = 1 */, U32 mask /* = Buff::Mask_Remove_Sys */)
{
	if(index >= _buffTab.size() || !m_pObj)
		return false;
	Buff* pBuff = _buffTab[index];
	if (!pBuff)
	{
		DOASSERT( false, "BuffTable::RemoveBuff !");
		return false;
	}
    
	// 叠加删除前的状态个数
	U32 newCount = pBuff->m_Count>_count ? (pBuff->m_Count-_count) : 0;
	// 只是状态叠加减少，此时不进行触发
	if(newCount > 0)
	{
		if(pBuff->m_BuffData->IsFlags(CBuffData::Flags_CalcStats) && pBuff->m_BuffData->IsFlags(CBuffData::Flags_PlusStats))
		{
			m_Summation.Add(pBuff->m_BuffStats, -(S32)_count);
			m_Refresh = true;
		}
		pBuff->m_Count = newCount;
		return true;
	}
    
	// 计算属性的状态
	if(pBuff->m_BuffData->IsFlags(CBuffData::Flags_CalcStats))
	{
		m_Summation.Add(pBuff->m_BuffStats, pBuff->m_BuffData->IsFlags(CBuffData::Flags_PlusStats) ? -(S32)pBuff->m_Count : -1);
		m_Refresh = true;
	}
    
    
	delete pBuff;
	_buffTab.erase(index);
	return true;
}


bool BuffTable::RemoveBuffBySrc(U32 _buffId, CGameObject* src, U32 _count /* = 1 */, U32 mask /* = Buff::Mask_Remove_Sys */)
{
	CBuffData* pBuffData = g_BuffDataMgr->GetBuff(_buffId);
	if(!pBuffData || !m_pObj)
		return false;
    
	BuffTableType* pBuffTable;
    pBuffTable = &m_Buff;
    
	for (S32 i=pBuffTable->size()-1; i>=0; --i)
	{
		if(_buffId == (*pBuffTable)[i]->m_BuffData->GetBuffId())
		{
            if((*pBuffTable)[i]->m_pSource == (CGameBase*)src)
            {
            
            }
			if (_count < (*pBuffTable)[i]->m_Count)
				RemoveBuff(*pBuffTable, (*pBuffTable)[i]->m_BuffData->GetBuffId(), i, _count, mask);
			else
				(*pBuffTable)[i]->Remove(m_pObj);
			// 同一源的同系状态只可能存在一个
			return true;
		}
	}
	return false;
}

void BuffTable::ClearBuff()
{
	for(S32 i = m_Buff.size()-1; i >= 0; --i)
		m_Buff[i]->Remove(m_pObj);
}

void BuffTable::Process(float dt)
{
	if(!m_pObj)
		return;
    
	// effectMask需要实时刷新
	U64 effectMask1 = 0;
    U64 effectMask2 = 0;
	// 注意触发的处理时机，应避免对状态表同时进行增减操作
	Buff* pBuff;
	for(S32 i=m_Buff.size()-1; i>=0; --i)
	{
		pBuff = m_Buff[i];
		if(!pBuff)
		{
			m_Buff.erase(i);
			continue;
		}
        
		if(!pBuff->Process(m_pObj, dt))
			RemoveBuff(m_Buff, pBuff->m_BuffData->GetBuffId(), i, pBuff->m_Count);
		else
		{
			effectMask1 |= pBuff->m_BuffData->GetEffect1();
            effectMask2 |= pBuff->m_BuffData->GetEffect2();
		}
	}
    
	
	m_EffectMask1 = effectMask1;
    m_EffectMask2 = effectMask2;
}

bool BuffTable::CalculateNpcStats(CBuffData& pData)
{
    CNpcObject* pNpc = dynamic_cast<CNpcObject*>(m_pObj);
    if(pNpc && pNpc->mDataBlock)
    {
        pData.percentNpc(pNpc->mDataBlock);
        m_Refresh = true;
        return true;
    }

	return false;
}

bool BuffTable::CalculateStats()
{
    if(m_Refresh)
	{
		static S32 hp;
		static S32 str;
		hp = m_Stats.HP;
        str = m_Stats.Str;
        m_Stats.Calc(m_Summation);
		m_Refresh = false;
		setHP(hp);
        setStr(str);
		return true;
	}
	return false;
}


bool BuffTable::checkBuff(U32 id, S32 count, bool seriesId /* = false */) const
{
	count = mAbs(count);
	if(count <= 0 || id == 0)
		return true;
    
	Buff* pBuff;
	for(S32 i=m_Buff.size()-1; i>=0; --i)
	{
		pBuff = m_Buff[i];
		if(pBuff && (pBuff->m_BuffData->GetBuffId()) == id)
        {
			if(pBuff->m_Count >= count)
				return true;
			else
				return false;
        }
	}
    
    return false;
}

U32 BuffTable::getBuffCount(U32 _buffId) const
{
	if(!_buffId)
		return 0;
    
	Buff* pBuff;
	for(S32 i=m_Buff.size()-1; i>=0; --i)
	{
		pBuff = m_Buff[i];
		if(pBuff && ( pBuff->m_BuffData->GetBuffId() == _buffId))
			return pBuff->m_Count;
	}
  
	return 0;
}

bool BuffTable::CalcAddAbnormity(CGameObject* pSource, CBuffData *pData, U32 rate)
{
	if (!pSource)
		return false;
	if (!pData)
		return false;
    
	return true;
}

