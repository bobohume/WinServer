    //
//  CNpcObject.cpp
//  HelloLua
//
//  Created by BaoGuo on 14-1-17.
//
//

#include "CNpcObject.h"
#include "CommLib/CommLib.h"

CNpcObject::CNpcObject():mDataBlock(NULL), mDataBlockId(0), mActiveSkillCd(0)
{
}

CNpcObject::~CNpcObject()
{
}

void CNpcObject::timeSignal(float dt)
{  
    Parent::timeSignal(dt);
}

void CNpcObject::death()
{
    m_BuffTable.ClearBuff();
}

void CNpcObject::Attack(CGameObject* obj)
{  
    Parent::Attack(obj);
    setMask(OPERATOR_END_MASK);
}

void CNpcObject::Spell(CGameObject *obj, U32 SkillId)
{
    
    Parent::Spell(obj, SkillId);
    setMask(OPERATOR_END_MASK);
}

void CNpcObject::Walk(stPoint grid)
{
    Parent::Walk(grid);
    setMask(OPERATOR_END_MASK);
}

/*S32 CNpcObject::randomMaster()
{
    S32 iNpcId = 0;
    S32 iGroupType = g_ClientGameplayState->m_GateData->m_MonsterGroup;
    std::vector<U8> FamilyVec;
    for(enMasterGroup group = GROUP_SHENG; group <= GROUP_LONG;)
    {
        if(group & iGroupType)
        {
            switch(group)
            {
                case GROUP_SHENG:
                {
                    FamilyVec.push_back(Family_Sheng);
                    break;
                }
                case GROUP_FO:
                {
                    FamilyVec.push_back(Family_Fo);
                    break;
                }
                case GROUP_XIAN:
                {
                    FamilyVec.push_back(Family_Xian);
                    break;
                }
                case GROUP_JING:
                {
                    FamilyVec.push_back(Family_Jing);
                    break;
                }
                case GROUP_GUI:
                {
                    FamilyVec.push_back(Family_Gui);
                    break;
                }
                case GROUP_GUAI:
                {
                    FamilyVec.push_back(Family_Guai);
                    break;
                }
                case GROUP_YAO:
                {
                    FamilyVec.push_back(Family_Yao);
                    break;
                }
                case GROUP_MO:
                {
                    FamilyVec.push_back(Family_Mo);
                    break;
                }
                case GROUP_SHOU:
                {
                    FamilyVec.push_back(Family_Shou);
                    break;
                }
                case GROUP_LONG:
                {
                    FamilyVec.push_back(Family_Long);
                    break;
                }
                default:
                    break;
            };
        }
        group = enMasterGroup(group << 1);
    }
    
    do
    {
        U32  nFamliyVal = gRandGen.randI(0, FamilyVec.size()- 1);
        nFamliyVal = FamilyVec[nFamliyVal];
        
        U32 nRandomVal = gRandGen.randI(0, g_NpcDataMgr->m_NpcVec[nFamliyVal].size() - 1);
        iNpcId = g_NpcDataMgr->m_NpcVec[nFamliyVal][nRandomVal];
        //Éý¼¶µÈ½×
        if(g_ClientGameplayState->m_GateData->m_MonsterLevel > 0)
        iNpcId += g_ClientGameplayState->m_GateData->m_MonsterLevel - 1;
        
        if(iNpcId % 10 > 6)
        {
            iNpcId = iNpcId % 10 * 10 + 6;
        }
        
        if(!g_NpcDataMgr->getNpcData(iNpcId))
        {
            iNpcId = 0;
        }
    }while(iNpcId == 0);
    
    FamilyVec.clear();
    
    return iNpcId;
}*/

void CNpcObject::addLevel(S32 level)
{
    m_Level += level;
}

const char* CNpcObject::getName()
{
    return mDataBlock->m_sName.c_str();
}
