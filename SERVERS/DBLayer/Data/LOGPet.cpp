#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGPet.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// ----------------------------------------------------------------------------
// 保存宠物的数据日志
DBError LOGPet::Save(void* Info)
{
	stPetLog* pInfo = (stPetLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreatePetLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',"\
			"%I64d,%d,%d,%d,%d,%I64d,%I64d",
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
			pInfo->PetId,
			pInfo->Val[0],
			pInfo->Val[1],
			pInfo->Val[2],
			pInfo->Val[3],
			pInfo->PetNo,
			pInfo->PetNo1);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存宠物的详细数据日志
DBError LOGPetDetail::Save(void* Info)
{
	stPetDetailLog* pInfo = (stPetDetailLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if(pInfo->PetNo == 0)
		{
			g_Log.WriteWarn("LOGPetDetail PetNo = 0");
			return DBERR_NONE;
		}

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);

		unsigned long OutSize = MAX_NAME_LENGTH*2;
		TempAlloc<char> pOut(OutSize);
		SafeDBString(pInfo->info.name, MAX_NAME_LENGTH, pOut, OutSize);

		M_SQL(GetConn(),"EXECUTE Sp_CreatePetDetailLog  %I64d,%I64d,%d,'%s',%d,%I64d,%d,%d,%d,%d,"\
			"%d,%I64d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			pInfo->PetNo,
			pInfo->info.id,
			pInfo->info.petDataId,
			(char*)pOut,
			pInfo->info.petEggId,
			pInfo->info.partnerId,
			pInfo->info.sex,
			pInfo->info.style,
			pInfo->info.mature,
			pInfo->info.status,
			pInfo->info.generation,
			pInfo->info.title,
			pInfo->info.titleEnabled,
			pInfo->info.level,
			pInfo->info.lives,
			pInfo->info.curHP,
			pInfo->info.curMP,
			pInfo->info.curPP,
			pInfo->info.staminaPts,
			pInfo->info.ManaPts,
			pInfo->info.strengthPts,
			pInfo->info.intellectPts,
			pInfo->info.agilityPts,
			pInfo->info.pneumaPts,
			pInfo->info.statsPoints,
			pInfo->info.insight,
			pInfo->info.petTalent,
			pInfo->info.IsIdentify,
			pInfo->info.chengZhangLv,
			pInfo->info.qianLi,
			pInfo->info.quality,
			pInfo->info.staminaGift,
			pInfo->info.ManaGift,
			pInfo->info.strengthGift,
			pInfo->info.intellectGift,
			pInfo->info.agilityGift,
			pInfo->info.pneumaGift,
			pInfo->info.happiness,
			pInfo->info.exp,
			pInfo->info.charPoints,
			pInfo->info.LastBreedLv,
			pInfo->info.BabyPetDataId,
			pInfo->info.BreedStartTime,
			pInfo->info.Combat[0],
			pInfo->info.Combat[1],
			pInfo->info.Combat[2],
			pInfo->info.Combat[3],
			pInfo->info.Combat[4],
			pInfo->info.Combat[5],
			pInfo->info.Combat[6],
			pInfo->info.Combat[7],
			pInfo->info.Combat[8],
			pInfo->info.Combat[9],
			pInfo->info.Combat[10],
			pInfo->info.Combat[11],
			pInfo->info.Combo[0],
			pInfo->info.Combo[1],
			pInfo->info.Combo[2],
			pInfo->info.Combo[3],
			pInfo->info.Combo[4],
			pInfo->info.Combo[5],
			pInfo->info.Combo[6],
			pInfo->info.Combo[7],
			pInfo->info.Combo[8],
			pInfo->info.Combo[9],
			pInfo->info.Combo[10],
			pInfo->info.Combo[11],
			pInfo->info.randBuffId,
			pInfo->info.mLeftTime,
			pInfo->info.LockedLeftTime,
			pInfo->info.characters[0],
			pInfo->info.characters[1],
			pInfo->info.characters[2],
			pInfo->info.characters[3],
			pInfo->info.characters[4],
			pInfo->info.equipitem[0],
			pInfo->info.equipitem[1],
			pInfo->info.equipitem[2],
			pInfo->info.equipitem[3]);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}