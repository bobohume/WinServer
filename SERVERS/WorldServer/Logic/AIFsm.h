#ifndef __AIFsm_h
#define __AIFsm_h

#include "Base/types.h"

class AIStateBase;
//-----------------------------------------------------------------------------
// AI的有限状态机基础类，类似事件形式
//-----------------------------------------------------------------------------
class AIFsm
{
protected:
	S32				m_CurrentAIState;
	S32				m_NextAIState;
	AIStateBase*	m_pState;

public:
	AIFsm();

	virtual ~AIFsm();
	virtual void	Update();

	S32				GetCurrentAIState() { return m_CurrentAIState; }
	S32				GetNextAIState() { return m_NextAIState; }
	void			SetNextAIState(S32 state) { m_NextAIState = state; }
};

class AIBase :public AIFsm
{
public:
	AIBase();
	virtual ~AIBase();

	// ============================== AICharacter =============================
	// ============================== AICondition =============================
	// ============================== AIOperation =============================
	// ============================== PveAI ==============================
	virtual void		PveAINone_onEnter() = 0;
	virtual void		PveAINone_onExec() = 0;
	virtual void		PveAINone_onExit() = 0;

	virtual void		PveAIAtk_onEnter() = 0;
	virtual void		PveAIAtk_onExec() = 0;
	virtual void		PveAIAtk_onExit() = 0;
};

//-----------------------------------------------------------------------------
// AI状态的基础类，所有AI状态类都应该派生于此类
//-----------------------------------------------------------------------------
class AIStateBase
{
public:
	typedef void (AIBase::*AIEvent)(void);

	AIEvent onEnter;
	AIEvent onExec;
	AIEvent onExit;
	AIStateBase() :onEnter(NULL), onExec(NULL), onExit(NULL) {}
	AIStateBase(AIEvent _onEnter, AIEvent _onExec, AIEvent _onExit) : onEnter(_onEnter), onExec(_onExec), onExit(_onExit) {}
};
#endif//__AIFsm_h
