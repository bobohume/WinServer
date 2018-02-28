#include "BuffMgr.h"
#include "DBLayer/Common/Define.h"
#include "../WorldServer.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "../Script/lua_tinker.h"
#include "Common/MemGuard.h"
#include "../PlayerMgr.h"
#include "PlayerInfoMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "CardMgr.h"
#include "Org.h"

CBuffManager::CBuffManager()
{
}

CBuffManager::~CBuffManager()
{
}

CBuffManager* CBuffManager::Instance()
{
	static CBuffManager s_Mgr;
	return &s_Mgr;
}

void CBuffManager::ClearStatus(U32 nPlayerID)
{
	StatsRef pData = GetData(nPlayerID);
	if (pData)
		pData->Clear();
}

void CBuffManager::AddStatus(U32 nPlayerID, Stats& stats)
{
	StatsRef pData = GetData(nPlayerID);
	if (pData)
	{
		*pData.get() += stats;
	}
}

void CBuffManager::LoadDB(U32 nPlaydID)
{
	StatsRef pData = StatsRef(new Stats);
	AddData(nPlaydID, pData);

	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetPlayer(nPlaydID);
	if (pPlayer)
	{
		ORGMGR->AddOrgSkill(nPlaydID, pPlayer->OrgId);
	}
}