#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGPlayer.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// ----------------------------------------------------------------------------
// 保存角色数据日志
DBError LOGPlayer::Save(void* Info)
{
	stPlayerLog* pInfo = (stPlayerLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreatePlayerLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',"\
			"%d,%I64d,%I64d,%I64d,%I64d,%I64d",
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
			pInfo->DataType,
			pInfo->Val[0],
			pInfo->Val[1],
			pInfo->Val[2],
			pInfo->Val[3],
			pInfo->Val[4]);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}