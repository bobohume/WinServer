//#include <time.h>
//#include <assert.h>
//#include <sstream>
//#include "DBLib/dbLib.h"
//#include "WinTcp/dtServerSocket.h"
//#include "Common/DumpHelper.h"
//#include "zlib/zlib.h"
//#include "base/SafeString.h"
//#include "Common/MemGuard.h"
//#include "PlayerMgr.h"
//#include "TeamManager.h"
//#include "WorldServer.h"
//#include "ExportScript.h"
//#include "base/WorkQueue.h"
//#include "topmanager.h"
//#include "CopymapManager.h"
//#include "PlayerOperator.h"
//#include "DBLayer\Data\TBLPlayer.h"
//
//stPlayerData* CPlayerOperator::loadPlayer( int playerId )
//{
//	stPlayerData* pPlayerData = new stPlayerData();
//
//	TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());
//	DBError err = tb_player.Load_BLOB(playerId, pPlayerData);
//	if(err != DBERR_NONE)
//	{
//		g_Log.WriteError("载入玩家[%d]角色数据出错:%s", playerId, getDBErr(err));
//		delete pPlayerData;
//		return NULL;
//	}
//	else
//	{
//		g_Log.WriteLog("载入玩家[%d]角色数据成功!", playerId);
//	}
//
//	return pPlayerData;
//}
//
//stPlayerData* CPlayerOperator::loadPlayer( std::string playerName )
//{
//	stPlayerData* pPlayerData = new stPlayerData();
//	TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());
//	DBError err = tb_player.Load_BLOB(playerName.c_str(), pPlayerData);
//	if(err != DBERR_NONE)
//	{
//		g_Log.WriteError("载入玩家[%s]角色数据出错:%s", playerName.c_str(), getDBErr(err));
//		delete pPlayerData;
//		return NULL;
//	}
//	else
//	{
//		g_Log.WriteLog("载入玩家[%s]角色数据成功!", playerName.c_str());
//	}
//	return pPlayerData;
//}
//
//ERROR_CODE CPlayerOperator::savePlayer( stPlayerData* pPlayerData )
//{
//	return savePlayer((stPlayerStruct*)pPlayerData);
//}
//
//ERROR_CODE CPlayerOperator::savePlayer( stPlayerStruct* pPlayerData )
//{
//	TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());
//	DBError err = tb_player.Save_BLOB(pPlayerData);
//	if(err != DBERR_NONE)
//	{
//		g_Log.WriteError("保存玩家[%d]角色数据出错:%s",  pPlayerData->BaseData.PlayerId, getDBErr(err));
//		return err;
//	}
//	else
//	{
//		g_Log.WriteLog("保存玩家[%d]角色数据成功!", pPlayerData->BaseData.PlayerId);
//		return NONE_ERROR;
//	}
//}