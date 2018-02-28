#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGLeague.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// ----------------------------------------------------------------------------
// 保存家族的数据日志
DBError LOGLeague::Save(void* Info)
{
	stLeagueLog* pInfo = (stLeagueLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreateLeagueLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',"\
			"%d,%d,%d,%d",
			pInfo->eid,
			pInfo->sourceid,
			pInfo->targettype,
			pInfo->targetid,
			pInfo->optype,
			pInfo->zoneid,
			pInfo->posx,
			pInfo->posy,
			pInfo->posz,
			szLogTime,
			pInfo->info.leagueInfo.leagueID,
			pInfo->info.leagueInfo.masterOrg,
			pInfo->info.leagueInfo.subMasterOrg,
			pInfo->info.orgInfo.orgID);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}