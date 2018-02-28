#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGItem.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// ----------------------------------------------------------------------------
// 保存物品的数据日志信息
DBError LOGItem::Save(void* Info)
{
	stItemLog* pInfo = (stItemLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		M_SQL(GetConn(),"EXECUTE Sp_CreateItemLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,"\
				"'%s',%I64d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%I64d,%I64d",
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
				pInfo->ItemUID,
				pInfo->ItemID,
				pInfo->SrcPos,
				pInfo->TagPos,
				pInfo->SrcNum,
				pInfo->TagNum,
				pInfo->ChangeNum,
				pInfo->ShopID,
				pInfo->Val0,
				pInfo->Val1,
				pInfo->ItemNo,
				pInfo->ItemNo1);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存物品的数据日志信息
DBError LOGItemDetail::Save(void* Info)
{
	stItemDetailLog* pInfo = (stItemDetailLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if(pInfo->ItemNo == 0)
		{
			g_Log.WriteError("LOGItemDetail itemno=0, type=%d", pInfo->optype);
			return DBERR_NONE;
		}

		//这里检查过滤掉物品ID非法的物品详细日志
		if(pInfo->info.ItemID <= 100000000 || pInfo->info.ItemID >= 150000000)
		{
			g_Log.WriteError("LOGItemDetail itemid error(itemid=%d, type=%d)", pInfo->info.ItemID, pInfo->optype);
			return DBERR_NONE;
		}

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		unsigned long OutSize = 64;

		M_SQL(GetConn(),"EXECUTE Sp_CreateItemDetailLog  %I64d,%I64d,%d,%d,"\
					"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
					"%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
					"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
					"%d,%d,%d,%d,%d,%d,%d,%d,%d",
			pInfo->ItemNo,
			pInfo->info.UID,
			pInfo->info.ItemID,
			pInfo->info.Quantity,
			pInfo->info.LapseTime,
			pInfo->info.RemainUseTimes,
			pInfo->info.BindPlayer,
			pInfo->info.BindFriend,
			pInfo->info.ActiveFlag & 0x01,
			pInfo->info.ActiveFlag & 0x02,
			pInfo->info.ActiveFlag & 0x04,
			pInfo->info.Quality,
			pInfo->info.CurWear,
			pInfo->info.CurMaxWear,
			pInfo->info.CurAdroit,
			pInfo->info.CurAdroitLv,
			pInfo->info.RandPropertyID,
			pInfo->info.ActivatePro,
			pInfo->info.IDEProValue[0],
			pInfo->info.IDEProValue[1],
			pInfo->info.IDEProValue[2],
			pInfo->info.IDEProValue[3],
			pInfo->info.IDEProValue[4],
			pInfo->info.IDEProValue[5],
			pInfo->info.IDEProValue[6],
			pInfo->info.IDEProValue[7],
			pInfo->info.IDEProValue[8],
			pInfo->info.IDEProValue[9],
			pInfo->info.IDEProValue[10],
			pInfo->info.IDEProValue[11],
			pInfo->info.EquipStrengthens,
			pInfo->info.EquipStrengthenLV[0],
			pInfo->info.EquipStrengthenLV[1],
			pInfo->info.EquipStrengthenLV[2],
			pInfo->info.EquipStrengthenLV[3],
			pInfo->info.EquipStrengthenLV[4],
			pInfo->info.EquipStrengthenLV[5],
			pInfo->info.EquipStrengthenLV[6],
			pInfo->info.EquipStrengthenLV[7],
			pInfo->info.EquipStrengthenLV[8],
			pInfo->info.EquipStrengthenLV[9],
			pInfo->info.EquipStrengthenLV[10],
			pInfo->info.EquipStrengthenLV[11],
			pInfo->info.EquipStrengthenLV[12],
			pInfo->info.EquipStrengthenLV[13],
			pInfo->info.EquipStrengthenLV[14],
			pInfo->info.WuXingID,
			pInfo->info.WuXingPro,
			pInfo->info.SkillAForEquip,
			pInfo->info.SkillBForEquip,
			pInfo->info.EmbedSlot[0],
			pInfo->info.EmbedSlot[1],
			pInfo->info.EmbedSlot[2],
			pInfo->info.EmbedSlot[3],
			pInfo->info.EmbedSlot[4],
			pInfo->info.EmbedSlot[5],
			pInfo->info.EmbedSlot[6],
			pInfo->info.EmbedSlot[7]);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}