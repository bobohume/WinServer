#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGActivity.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// ----------------------------------------------------------------------------
// 保存活动等等一些数据日志
DBError LOGActivity::Save(void* Info)
{
	stActivityLog* pInfo = (stActivityLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreateActivityLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',"\
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
			pInfo->ActivityID,
			pInfo->Val[0],
			pInfo->Val[1],
			pInfo->Val[2]);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}