#include "NoviceGuide.h"
#include "TimeSetBase.h"
#include "TimeSetMgr.h"
#include "../Script/lua_tinker.h"

CNoviceGuide* CNoviceGuide::Instance()
{
	static CNoviceGuide s_NoviceGuide;
	return &s_NoviceGuide;
}

bool CNoviceGuide::IsFirstSupportRequest(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, SUPPORT);
}

bool CNoviceGuide::IsFirstSkillFast(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, SKILL);
}

bool CNoviceGuide::IsFirstChoosePrincess(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, CHOOSEPRINCESS);
}

bool CNoviceGuide::IsFirstCourtAffairFast(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, COURTAFFAIRFAST);
}

bool CNoviceGuide::IsFirstRiteFast(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, RITEFAST);
}

bool CNoviceGuide::IsFirstPlantFast(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, PLANTFAST);
}

bool CNoviceGuide::IsFirstDragonSpirit(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, DRAGONSPIRIT);
}

bool CNoviceGuide::IsFirstConquer(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, CONQUER);
}

bool CNoviceGuide::IsFirstVisit(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, VISIT);
}

bool CNoviceGuide::IsFirstVisitAndHold(U32 nPlayerId)
{
	return IsFirstAndHoldStatus(nPlayerId, VISIT);
}

bool CNoviceGuide::IsFirstAddPrince(U32 nPlayerId)
{
	return IsFirstAndChangeStatus(nPlayerId, ADDPRINCE);
}

bool CNoviceGuide::IsFirstAndChangeStatus(U32 nPlayerId, enNoviceGuideType type)
{
	bool bFirst = true;
	U32 Flag = 0;
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerId, NOVICEGUIDE);
	if (pTime)
	{
		bFirst = pTime->Flag1 & (1 << type) ? false : true;
		Flag = pTime->Flag1 | (1 << type);
	}
	else
	{
		Flag = (1 << type);
	}

	if (bFirst)
	{
		TIMESETMGR->AddTimeSet(nPlayerId, NOVICEGUIDE, 0, "", Flag);
	}

	return bFirst;
}

bool CNoviceGuide::IsFirstAndHoldStatus(U32 nPlayerId, enNoviceGuideType type)
{
	bool bFirst = true;
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerId, NOVICEGUIDE);
	if (pTime)
	{
		bFirst = pTime->Flag1 & (1 << type) ? false : true;
	}

	return bFirst;
}
