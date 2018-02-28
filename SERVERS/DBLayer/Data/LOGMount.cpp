#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGMount.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// ----------------------------------------------------------------------------
// 保存骑宠的数据日志
DBError LOGMount::Save(void* Info)
{
	stMountLog* pInfo = (stMountLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreateMountLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',"\
			"%d,%d,%d,%d,%d,%I64d,%I64d",
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
			pInfo->Val[0],
			pInfo->Val[1],
			pInfo->Val[2],
			pInfo->Val[3],
			pInfo->Val[4],
			pInfo->MountNo,
			pInfo->MountNo1);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存骑宠的数据日志
DBError LOGMountDetail::Save(void* Info)
{
	stMountDetailLog* pInfo = (stMountDetailLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if(pInfo->MountNo == 0)
		{
			g_Log.WriteWarn("LOGMountDetail mountno = 0");
			return DBERR_NONE;
		}

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreateMountDetailLog  %I64d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			pInfo->MountNo,
			pInfo->info.mDataId,
			pInfo->info.mLevel,
			pInfo->info.mDuration,
			pInfo->info.mStartTime,
			pInfo->info.mSkillId[0],
			pInfo->info.mSkillId[1],
			pInfo->info.mSkillId[2],
			pInfo->info.mStatus,
			pInfo->info.LockedLeftTime);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}