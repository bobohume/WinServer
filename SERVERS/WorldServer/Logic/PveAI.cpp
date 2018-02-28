#include "PveAI.h"
#include "PvpMgr.h"
#include "Common/mRandom.h"

AIStateBase PveAI::m_Fsm[] =
{
	AIStateBase(&AIBase::PveAINone_onEnter, &AIBase::PveAINone_onExec, &AIBase::PveAINone_onExit),
	AIStateBase(&AIBase::PveAINone_onEnter, &AIBase::PveAIAtk_onExec,  &AIBase::PveAIAtk_onExit),
};

PveAI::PveAI(S64 nID, S8 nType):AIBase(), m_PlayerID(nID), m_Type(nType)
{
	m_CurrentAIState = PveAI::NPC_AI_ATK;
	m_pState = &m_Fsm[m_CurrentAIState];
}

PveAI::~PveAI()
{

}

void PveAI::Update()
{
	if (m_CurrentAIState != m_NextAIState)
	{
		(this->*(m_pState->onExit))();
		m_CurrentAIState = m_NextAIState;
		m_pState = &m_Fsm[m_CurrentAIState];
		(this->*(m_pState->onEnter))();;
	}
	if (m_CurrentAIState == m_NextAIState)
	{
		(this->*(m_pState->onExec))();
	}
}

void PveAI::PveAINone_onEnter()
{
}

void PveAI::PveAINone_onExec()
{
	SetNextAIState(PveAI::NPC_AI_ATK);
}

void PveAI::PveAINone_onExit()
{
}

void PveAI::PveAIAtk_onEnter()
{
}

void PveAI::PveAIAtk_onExec()
{
	DATA_VEC& DataDeq = PVEMGR->GetBattle(m_PlayerID, m_Type);
	if(!DataDeq.empty())
	{
		PvpInfoRef pData = DataDeq.back();
		if (pData && pData->Peerless >= 100)
		{
			PVEMGR->Battle(m_PlayerID, 3, m_Type);
			return;
		}
	}
	
	S32 nAction = gRandGen.randI(0, 2);
	PVEMGR->Battle(m_PlayerID, nAction, m_Type);
}

void PveAI::PveAIAtk_onExit()
{
}