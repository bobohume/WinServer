#include "CPlayerObject.h"
#include "../Data/InvestiGate.h"
#include "BASE/tVector.h"
#include <cmath>


const char* Player::ACTION_ANIMATION = "Action.ccbi";


Player::Player():CGameObject(), m_GateId(5000001), m_PlayerId(0)
{
 
}

Player::~Player()
{
}

void Player::Attack(CGameObject* obj)
{
    Parent::Attack(obj);
    setMask(OPERATOR_END_MASK);
}

void Player::Walk(stPoint grid)
{
    Parent::Walk(grid);
    setMask(OPERATOR_END_MASK);
}

void Player::Spell(CGameObject* obj, U32 SkillId)
{
    Parent::Spell(obj, SkillId);
    setMask(OPERATOR_END_MASK);
}

void Player::timeSignal(float dt)
{
    Parent::timeSignal(dt);
}

const char* Player::getName()
{
    return "You";
}
