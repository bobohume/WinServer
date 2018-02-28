#include "GameConfig.h"
#include "../../data/BuffData.h"

U64  getPlayerLevelUpExp(U32 nLevel)
{
	U32 nLevelBuffId = GAMECFG::PLAYER_LEVELUP_NEED_ID + nLevel;
	CBuffData* pBuffData = g_BuffDataMgr->GetBuff(nLevelBuffId);
	if(pBuffData)
		return pBuffData->m_LvUpExp;
	
	return 0xFFFFFFFFFFFFFFFF;
}