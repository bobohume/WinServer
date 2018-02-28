#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGOrg.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// ----------------------------------------------------------------------------
// 保存家族的数据日志
DBError LOGOrg::Save(void* Info)
{
	stOrgLog* pInfo = (stOrgLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreateOrgLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
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
			pInfo->info.orgInfo.orgID,
			pInfo->info.orgInfo.masterID,
			pInfo->info.orgInfo.money,
			pInfo->info.memberInfo.playerID,
			pInfo->info.memberInfo.money,
			pInfo->info.memberInfo.leagueMoney,
			pInfo->info.opInfo.itemID,
			pInfo->info.opInfo.itemAmount,
			pInfo->info.opInfo.money,
			pInfo->info.opInfo.result);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}