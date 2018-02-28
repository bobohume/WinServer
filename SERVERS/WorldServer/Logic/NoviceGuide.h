#pragma once

#include "BASE/types.h"

enum enNoviceGuideType
{
	SUPPORT = 0,	//宠幸
	SKILL,			//科技
	CHOOSEPRINCESS,	//翻牌
	COURTAFFAIRFAST,//宫务加速
	RITEFAST,		//礼仪加速
	PLANTFAST,		//养花加速
	DRAGONSPIRIT,	//龙魂单抽
	CONQUER,		//征服单抽
	VISIT,			//寻访
	ADDPRINCE,		//第一次生皇子
};

class CNoviceGuide
{
public:
	static CNoviceGuide* Instance();

	//第一次宠幸
	bool IsFirstSupportRequest(U32 nPlayerId);
	//第一次研究科技加速
	bool IsFirstSkillFast(U32 nPlayerId);
	//第一次翻牌
	bool IsFirstChoosePrincess(U32 nPlayerId);
	//第一次宫务加速
	bool IsFirstCourtAffairFast(U32 nPlayerId);
	//第一次礼仪加速
	bool IsFirstRiteFast(U32 nPlayerId);
	//第一次养花加速
	bool IsFirstPlantFast(U32 nPlayerId);
	//第一次龙魂单抽
	bool IsFirstDragonSpirit(U32 nPlayerId);
	//第一次征服单抽
	bool IsFirstConquer(U32 nPlayerId);
	//第一次寻访
	bool IsFirstVisit(U32 nPlayerId);
	//第一次寻访，不改变寻访状态
	bool IsFirstVisitAndHold(U32 nPlayerId);
	//第一次生皇子
	bool IsFirstAddPrince(U32 nPlayerId);
private:
	bool IsFirstAndChangeStatus(U32 nPlayerId, enNoviceGuideType type);

	bool IsFirstAndHoldStatus(U32 nPlayerId, enNoviceGuideType type);
};

#define NGUIDE CNoviceGuide::Instance()