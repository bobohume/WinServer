#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGSpirit.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// ----------------------------------------------------------------------------
// 保存元神的数据日志
DBError LOGSpirit::Save(void* Info)
{
	stSpiritLog* pInfo = (stSpiritLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreateSpiritLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',"\
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
			pInfo->SpiritNo,
			pInfo->SpiritNo1);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存元神的详细数据日志
DBError LOGSpiritDetail::Save(void* Info)
{
	stSpiritDetailLog* pInfo = (stSpiritDetailLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if(pInfo->SpiritNo == 0)
		{
			g_Log.WriteWarn("LOGSpiritDetail spiritno = 0");
			return DBERR_NONE;
		}

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreateSpiritDetailLog  %I64d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			pInfo->SpiritNo,
			pInfo->info.mDataId,
			pInfo->info.mLevel,
			pInfo->info.mJingJieLevel,
			pInfo->info.mTalentExtPts,
			pInfo->info.mTalentLeftPts,
			pInfo->info.mTalentSkillPts[0],
			pInfo->info.mTalentSkillPts[1],
			pInfo->info.mTalentSkillPts[2],
			pInfo->info.mTalentSkillPts[3],
			pInfo->info.mTalentSkillPts[4],
			pInfo->info.mTalentSkillPts[5],
			pInfo->info.mTalentSkillPts[6],
			pInfo->info.mTalentSkillPts[7],
			pInfo->info.mTalentSkillPts[8],
			pInfo->info.mTalentSkillPts[9],
			pInfo->info.mTalentSkillPts[10],
			pInfo->info.mTalentSkillPts[11],
			pInfo->info.mTalentSkillPts[12],
			pInfo->info.mTalentSkillPts[13],
			pInfo->info.mTalentSkillPts[14],
			pInfo->info.mTalentSkillPts[15],
			pInfo->info.mTalentSkillPts[16],
			pInfo->info.mTalentSkillPts[17],
			pInfo->info.mTalentSkillPts[18],
			pInfo->info.mTalentSkillPts[19],
			pInfo->info.mTalentSkillPts[20],
			pInfo->info.mTalentSkillPts[21],
			pInfo->info.mTalentSkillPts[22],
			pInfo->info.mTalentSkillPts[23],
			pInfo->info.mTalentSkillPts[24],
			pInfo->info.mTalentSkillPts[25],
			pInfo->info.mTalentSkillPts[26],
			pInfo->info.mTalentSkillPts[27],
			pInfo->info.mTalentSkillPts[28],
			pInfo->info.mTalentSkillPts[29],
			pInfo->info.mTalentSkillPts[30],
			pInfo->info.mTalentSkillPts[31],
			pInfo->info.mTalentSkillPts[32],
			pInfo->info.mTalentSkillPts[33],
			pInfo->info.mTalentSkillPts[34],
			pInfo->info.mTalentSkillPts[35],
			pInfo->info.mOpenedSkillFlag,
			pInfo->info.mSkillId[0],
			pInfo->info.mSkillId[1],
			pInfo->info.mSkillId[2],
			pInfo->info.mSkillId[3],
			pInfo->info.mSkillId[4],
			pInfo->info.mSkillId[5],
			pInfo->info.mEquipId[0],
			pInfo->info.mEquipId[1],
			pInfo->info.mEquipId[2],
			pInfo->info.mCleanPtsCount,
			pInfo->info.mValueBuffId,
			pInfo->info.mCostBuffId,
			pInfo->info.mCurrExp,
			pInfo->info.LockedLeftTime);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}