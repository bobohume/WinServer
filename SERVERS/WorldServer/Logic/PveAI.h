#ifndef __PVEAI_H__
#define __PVEAI_H__

#include <memory>
#include "AIFsm.h"

class PveAI : public AIBase
{
public:
	typedef AIBase Parent;

	enum NpcAIState
	{
		NPC_AI_NONE,
		NPC_AI_ATK,
		NPC_AI_MAX,
	};
private:
	static AIStateBase	m_Fsm[NPC_AI_MAX];
	S64 m_PlayerID;
	S8  m_Type;
public:
	PveAI(S64 nID, S8 nType);
	virtual ~PveAI();

	virtual void		Update();
	virtual void		PveAINone_onEnter();
	virtual void		PveAINone_onExec();
	virtual void		PveAINone_onExit();

	virtual void		PveAIAtk_onEnter();
	virtual void		PveAIAtk_onExec();
	virtual void		PveAIAtk_onExit();
};

typedef std::tr1::shared_ptr<PveAI>	PveAIRef;
#endif//__PVEAI_H__
