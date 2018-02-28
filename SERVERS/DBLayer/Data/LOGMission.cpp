#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGMission.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// ----------------------------------------------------------------------------
// 保存任务的数据日志
DBError LOGMission::Save(void* Info)
{
	stMissionLog* pInfo = (stMissionLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreateMissionLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',%d",
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
			pInfo->MissionID);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}