#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "Common/PacketType.h"
#include "common/PlayerStruct.h"
#include "Common/FixedStallBase.h"
#include "DBUtility.h"
#include "TBLStallList.h"

// ----------------------------------------------------------------------------
// 保存商铺信息
DBError TBLStallList::SaveInfo(stFixedStall* info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!info)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if(!StrSafeCheck(info->StallName, 32))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if(!StrSafeCheck(info->StallerName, 32))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szPayTaxTime[32] = {0};
		GetDBTimeString(getZeroTime(info->PayTaxTime), szPayTaxTime);

		char szAttornStartTime[32] = {0};
		GetDBTimeString(info->AttornStartTime, szAttornStartTime);

		char szBuyTime[32] = {0};
		GetDBTimeString(info->BuyTime, szBuyTime);

		char szShoutLastTime[32] = {0};
		GetDBTimeString(info->ShoutLastTime, szShoutLastTime);

		unsigned long OutSize = _countof(info->Notice)*2;
		TempAlloc<char> pOut(OutSize);
		SafeDBString(info->Notice, _countof(info->Notice), pOut, OutSize);

		OutSize = _countof(info->ShoutContent)*2;
		TempAlloc<char> szShoutContent(OutSize);
		SafeDBString(info->ShoutContent, _countof(info->ShoutContent), szShoutContent, OutSize);

		M_SQL(GetConn(),"EXECUTE Sp_SaveStallInfo %d,'%s',%d,'%s',"\
			"%d,%d,%d,"\
			"%d,%d,'%s',%d,"\
			"%d,'%s',%d,%d,%d,"\
			"'%s',%d,'%s',%d,"\
			"%d,%d,'%s',"\
			"%d,%d,'%s'",
			info->StallID, info->StallName, info->StallerID, info->StallerName,
			info->PreStallerID, info->StallType, info->StallStatus,
			info->TradeTax, info->StallTax, szPayTaxTime, info->IsAttorn,
			info->AttornPrice, szAttornStartTime, info->FixedMoney, info->FlowMoney, info->Flourish,
			pOut.ptr, info->IsShoutAll, szShoutContent.ptr, info->ShoutInterval,
			info->ShoutItemID, info->ShoutItemNum, szShoutLastTime,
			info->BuySellItems, info->BuySellPets, szBuyTime);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(info->StallID)
	return err;
}

// ----------------------------------------------------------------------------
// 加载商铺信息
DBError TBLStallList::LoadAllInfo(stFixedStall* info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!info)
			throw ExceptionResult(DBERR_PLAYER_UNDELETE);

		U32 i;
		M_SQL(GetConn(),"EXECUTE Sp_LoadStallInfo");
		while(GetConn()->More())
		{
			i = GetConn()->GetInt();
			if(i >= MAX_FIXEDSTALLS)
				continue;
			info[i].StallID = i;
			strcpy_s(info[i].StallName, 32, GetConn()->GetString());
			info[i].StallerID = GetConn()->GetInt();
			strcpy_s(info[i].StallerName, 32, GetConn()->GetString());
			info[i].PreStallerID = GetConn()->GetInt();
			info[i].StallType = GetConn()->GetInt();
			info[i].StallStatus = GetConn()->GetInt();
			info[i].TradeTax = GetConn()->GetInt();
			info[i].StallTax = GetConn()->GetInt();
			info[i].PayTaxTime = getZeroTime(GetConn()->GetTime());
			info[i].IsAttorn = (GetConn()->GetInt() != 0);
			info[i].AttornPrice = GetConn()->GetInt();
			info[i].AttornStartTime = GetConn()->GetTime();
			info[i].FixedMoney = GetConn()->GetInt();
			info[i].FlowMoney = GetConn()->GetInt();
			info[i].Flourish = GetConn()->GetInt();
			strcpy_s(info[i].Notice, 128,GetConn()->GetString());
			info[i].IsShoutAll = (GetConn()->GetInt() != 0);
			strcpy_s(info[i].ShoutContent, 128, GetConn()->GetString());
			info[i].ShoutInterval = GetConn()->GetInt();
			info[i].ShoutItemID = GetConn()->GetInt();
			info[i].ShoutItemNum = GetConn()->GetInt();
			info[i].ShoutLastTime = GetConn()->GetTime();
			info[i].BuySellItems = GetConn()->GetInt();
			info[i].BuySellPets = GetConn()->GetInt();
			info[i].BuyTime = GetConn()->GetTime();
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存商铺出售栏物品
DBError TBLStallList::SaveSellItem(U16 stallid, stFixedStallItem* item)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		stFixedStallItem* pInfo = (stFixedStallItem*)item;
		char szRecord[8000] = {0};
		char szRow[1600] = {0};
		bool isFirst;
		int count = (MAX_FIXEDSTALL_SELLITEM % 5) == 0? MAX_FIXEDSTALL_SELLITEM / 5 : (MAX_FIXEDSTALL_SELLITEM / 5 + 1);
		for(int c = 0; c < count; c++)
		{
			strcpy_s(szRecord, 8000, "EXECUTE Sp_SaveStallSellItem '");
			isFirst = true;
			for(int i = c * 5; i < 5 * (c + 1); i++)
			{
				sprintf_s(szRow, 1600, "%s''%d'',''%I64d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d''",
					isFirst ? "":";",
					pInfo[i].Price,
					pInfo[i].UID,
					pInfo[i].ItemID,
					pInfo[i].Quantity,
					pInfo[i].LapseTime,
					pInfo[i].RemainUseTimes,

					pInfo[i].LockedLeftTime,
					pInfo[i].BindPlayer,
					pInfo[i].BindFriend,
					pInfo[i].ActiveFlag & 0x01,
					pInfo[i].ActiveFlag & 0x02,
					pInfo[i].ActiveFlag & 0x04,
					pInfo[i].Quality,
					pInfo[i].CurWear,
					pInfo[i].CurMaxWear,
					pInfo[i].CurAdroit,
					pInfo[i].CurAdroitLv,
					pInfo[i].ActivatePro,
					pInfo[i].RandPropertyID,

					pInfo[i].IDEProValue[0],
					pInfo[i].IDEProValue[1],
					pInfo[i].IDEProValue[2],
					pInfo[i].IDEProValue[3],
					pInfo[i].IDEProValue[4],
					pInfo[i].IDEProValue[5],
					pInfo[i].IDEProValue[6],
					pInfo[i].IDEProValue[7],
					pInfo[i].IDEProValue[8],
					pInfo[i].IDEProValue[9],
					pInfo[i].IDEProValue[10],
					pInfo[i].IDEProValue[11],
					pInfo[i].EquipStrengthens,

					pInfo[i].EquipStrengthenLV[0],
					pInfo[i].EquipStrengthenLV[1],
					pInfo[i].EquipStrengthenLV[2],
					pInfo[i].EquipStrengthenLV[3],
					pInfo[i].EquipStrengthenLV[4],
					pInfo[i].EquipStrengthenLV[5],
					pInfo[i].EquipStrengthenLV[6],
					pInfo[i].EquipStrengthenLV[7],
					pInfo[i].EquipStrengthenLV[8],
					pInfo[i].EquipStrengthenLV[9],
					pInfo[i].EquipStrengthenLV[10],
					pInfo[i].EquipStrengthenLV[11],
					pInfo[i].EquipStrengthenLV[12],
					pInfo[i].EquipStrengthenLV[13],
					pInfo[i].EquipStrengthenLV[14],

					pInfo[i].WuXingID,
					pInfo[i].WuXingPro,
					pInfo[i].SkillAForEquip,
					pInfo[i].SkillBForEquip,
					pInfo[i].EmbedSlot[0],
					pInfo[i].EmbedSlot[1],
					pInfo[i].EmbedSlot[2],

					pInfo[i].EmbedSlot[3],
					pInfo[i].EmbedSlot[4],
					pInfo[i].EmbedSlot[5],
					pInfo[i].EmbedSlot[6],
					pInfo[i].EmbedSlot[7],
					pInfo[i].reserve[0],
					pInfo[i].reserve[1],
					pInfo[i].reserve[2],
					pInfo[i].reserve[3],
					pInfo[i].reserve[4],
					
					pInfo[i].dbVersion,
					pInfo[i].PromoteLv[0],
					pInfo[i].PromoteLv[1],
					pInfo[i].PromoteLv[2],
					pInfo[i].PromoteLv[3],
					pInfo[i].UnuseField[0],
					pInfo[i].UnuseField[1],
					pInfo[i].UnuseField[2],
					pInfo[i].UnuseField[3],
					pInfo[i].UnuseField[4],
					pInfo[i].UnuseField[5],
					pInfo[i].UnuseField[6],

					stallid, i);
				strcat_s(szRecord, szRow);
				isFirst = false;
			}

			strcat_s(szRecord, "'");
			M_SQL(GetConn(),szRecord);
			GetConn()->Exec();
			err = DBERR_NONE;
		}
	}
	DBCATCH(stallid)
	return err;
}

// ----------------------------------------------------------------------------
// 加载商铺出售栏物品数据
DBError TBLStallList::LoadAllSellItem(stFixedStallItem* pItem)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pItem)
			throw ExceptionResult(DBERR_PLAYER_UNDELETE);

		U32 i, StallID, Pos;
		M_SQL(GetConn(),"EXECUTE Sp_LoadStallSellItem");
		while(GetConn()->More())
		{
			StallID = GetConn()->GetInt();
			Pos = GetConn()->GetInt();
			if(StallID >= MAX_FIXEDSTALLS || Pos >= MAX_FIXEDSTALL_SELLITEM)
				continue;
			i = StallID * MAX_FIXEDSTALL_SELLITEM + Pos;
			pItem[i].StallID = StallID;
			pItem[i].Pos = Pos;
			pItem[i].Price = GetConn()->GetInt();
			pItem[i].UID = GetConn()->GetBigInt();
			pItem[i].ItemID = GetConn()->GetInt();
			pItem[i].Quantity = GetConn()->GetInt();
			pItem[i].LapseTime = GetConn()->GetInt();
			pItem[i].RemainUseTimes = GetConn()->GetInt();
			pItem[i].LockedLeftTime = GetConn()->GetInt();
			pItem[i].BindFriend = GetConn()->GetInt();
			pItem[i].BindFriend = GetConn()->GetInt();
			pItem[i].ActiveFlag |= GetConn()->GetInt();
			pItem[i].ActiveFlag |= (GetConn()->GetInt() << 1);
			pItem[i].ActiveFlag |= (GetConn()->GetInt() << 2);
			pItem[i].Quality = GetConn()->GetInt();
			pItem[i].CurWear = GetConn()->GetInt();
			pItem[i].CurMaxWear = GetConn()->GetInt();
			pItem[i].CurAdroit = GetConn()->GetInt();
			pItem[i].CurAdroitLv = GetConn()->GetInt();
			pItem[i].ActivatePro = GetConn()->GetInt();
			pItem[i].RandPropertyID = GetConn()->GetInt();
			for(int j = 0; j < MAX_IDEPROS; j++)
				pItem[i].IDEProValue[j] = GetConn()->GetInt();
			pItem[i].EquipStrengthens = GetConn()->GetInt();
			for(int j = 0; j < MAX_EQUIPSTENGTHENS; j++)
			{
				pItem[i].EquipStrengthenLV[j] = GetConn()->GetInt();
			}
			pItem[i].WuXingID = GetConn()->GetInt();
			pItem[i].WuXingPro = GetConn()->GetInt();
			pItem[i].SkillAForEquip = GetConn()->GetInt();
			pItem[i].SkillBForEquip = GetConn()->GetInt();
			for(int j = 0; j < MAX_EMBEDSLOTS; j++)
				pItem[i].EmbedSlot[j] = GetConn()->GetInt();
			for(int j = 0; j < 5; j++)
				pItem[i].reserve[j] = GetConn()->GetInt();

			pItem[i].dbVersion = GetConn()->GetInt();
			for(int j = 0; j < 4; j++)
				pItem[i].PromoteLv[j] = GetConn()->GetInt();
			for(int j = 0; j < 7; j++)
				pItem[i].UnuseField[j] = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存商铺收购栏物品
DBError TBLStallList::SaveBuyItem(U16 stallid, stFixedStallItem* item)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		stFixedStallItem* pInfo = (stFixedStallItem*)item;
		char szRecord[8000] = {0};
		char szRow[1600] = {0};
		bool isFirst;
		for(int c = 0; c < 3; c++)
		{
			strcpy_s(szRecord, 8000, "EXECUTE Sp_SaveStallBuyItem '");
			isFirst = true;
			for(int i = c * 4; i < 4 * (c + 1); i++)
			{
				sprintf_s(szRow, 1600, "%s''%d'',''%d'',''%d'',''%I64d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d''",
					isFirst ? "":";",
					pInfo[i].Price,
					pInfo[i].BuyNum,
					pInfo[i].LeftNum,
					pInfo[i].UID,
					pInfo[i].ItemID,
					pInfo[i].Quantity,
					pInfo[i].LapseTime,
					pInfo[i].RemainUseTimes,

					pInfo[i].LockedLeftTime,
					pInfo[i].BindPlayer,
					pInfo[i].BindFriend,
					pInfo[i].ActiveFlag & 0x01,
					pInfo[i].ActiveFlag & 0x02,
					pInfo[i].ActiveFlag & 0x04,
					pInfo[i].Quality,
					pInfo[i].CurWear,
					pInfo[i].CurMaxWear,
					pInfo[i].CurAdroit,
					pInfo[i].CurAdroitLv,
					pInfo[i].ActivatePro,
					pInfo[i].RandPropertyID,

					pInfo[i].IDEProValue[0],
					pInfo[i].IDEProValue[1],
					pInfo[i].IDEProValue[2],
					pInfo[i].IDEProValue[3],
					pInfo[i].IDEProValue[4],
					pInfo[i].IDEProValue[5],
					pInfo[i].IDEProValue[6],
					pInfo[i].IDEProValue[7],
					pInfo[i].IDEProValue[8],
					pInfo[i].IDEProValue[9],
					pInfo[i].IDEProValue[10],
					pInfo[i].IDEProValue[11],
					pInfo[i].EquipStrengthens,

					pInfo[i].EquipStrengthenLV[0],
					pInfo[i].EquipStrengthenLV[1],
					pInfo[i].EquipStrengthenLV[2],
					pInfo[i].EquipStrengthenLV[3],
					pInfo[i].EquipStrengthenLV[4],
					pInfo[i].EquipStrengthenLV[5],
					pInfo[i].EquipStrengthenLV[6],
					pInfo[i].EquipStrengthenLV[7],
					pInfo[i].EquipStrengthenLV[8],
					pInfo[i].EquipStrengthenLV[9],
					pInfo[i].EquipStrengthenLV[10],
					pInfo[i].EquipStrengthenLV[11],
					pInfo[i].EquipStrengthenLV[12],
					pInfo[i].EquipStrengthenLV[13],
					pInfo[i].EquipStrengthenLV[14],

					pInfo[i].WuXingID,
					pInfo[i].WuXingPro,
					pInfo[i].SkillAForEquip,
					pInfo[i].SkillBForEquip,
					pInfo[i].EmbedSlot[0],
					pInfo[i].EmbedSlot[1],
					pInfo[i].EmbedSlot[2],

					pInfo[i].EmbedSlot[3],
					pInfo[i].EmbedSlot[4],
					pInfo[i].EmbedSlot[5],
					pInfo[i].EmbedSlot[6],
					pInfo[i].EmbedSlot[7],
					pInfo[i].reserve[0],
					pInfo[i].reserve[1],
					pInfo[i].reserve[2],
					pInfo[i].reserve[3],
					pInfo[i].reserve[4],

					pInfo[i].dbVersion,
					pInfo[i].PromoteLv[0],
					pInfo[i].PromoteLv[1],
					pInfo[i].PromoteLv[2],
					pInfo[i].PromoteLv[3],
					pInfo[i].UnuseField[0],
					pInfo[i].UnuseField[1],
					pInfo[i].UnuseField[2],
					pInfo[i].UnuseField[3],
					pInfo[i].UnuseField[4],
					pInfo[i].UnuseField[5],
					pInfo[i].UnuseField[6],

					stallid, i);
				strcat_s(szRecord, szRow);
				isFirst = false;
			}

			strcat_s(szRecord, "'");
			M_SQL(GetConn(),szRecord);
			GetConn()->Exec();
			err = DBERR_NONE;
		}
	}
	DBCATCH(stallid);
	return err;
}

// ----------------------------------------------------------------------------
// 加载商铺收购物品栏数据
DBError TBLStallList::LoadAllBuyItem(stFixedStallItem* pItem)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pItem)
			throw ExceptionResult(DBERR_PLAYER_UNDELETE);

		U32 i, StallID, Pos;
		M_SQL(GetConn(),"EXECUTE Sp_LoadStallBuyItem");
		while(GetConn()->More())
		{
			StallID = GetConn()->GetInt();
			Pos = GetConn()->GetInt();
			if(StallID >= MAX_FIXEDSTALLS || Pos >= MAX_FIXEDSTALL_BUYITEM)
				continue;
			i = StallID * MAX_FIXEDSTALL_BUYITEM + Pos;
			pItem[i].StallID = StallID;
			pItem[i].Pos = Pos;
			pItem[i].Price = GetConn()->GetInt();
			pItem[i].BuyNum = GetConn()->GetInt();
			pItem[i].LeftNum = GetConn()->GetInt();
			pItem[i].UID = GetConn()->GetBigInt();
			pItem[i].ItemID = GetConn()->GetInt();
			pItem[i].Quantity = GetConn()->GetInt();
			pItem[i].LapseTime = GetConn()->GetInt();
			pItem[i].RemainUseTimes = GetConn()->GetInt();
			pItem[i].LockedLeftTime = GetConn()->GetInt();
			pItem[i].BindFriend = GetConn()->GetInt();
			pItem[i].BindFriend = GetConn()->GetInt();
			pItem[i].ActiveFlag |= GetConn()->GetInt();
			pItem[i].ActiveFlag |= (GetConn()->GetInt() << 1);
			pItem[i].ActiveFlag |= (GetConn()->GetInt() << 2);
			pItem[i].Quality = GetConn()->GetInt();
			pItem[i].CurWear = GetConn()->GetInt();
			pItem[i].CurMaxWear = GetConn()->GetInt();
			pItem[i].CurAdroit = GetConn()->GetInt();
			pItem[i].CurAdroitLv = GetConn()->GetInt();
			pItem[i].ActivatePro = GetConn()->GetInt();
			pItem[i].RandPropertyID = GetConn()->GetInt();
			for(int j = 0; j < MAX_IDEPROS; j++)
				pItem[i].IDEProValue[j] = GetConn()->GetInt();
			pItem[i].EquipStrengthens = GetConn()->GetInt();
			for(int j = 0; j < MAX_EQUIPSTENGTHENS; j++)
			{
				pItem[i].EquipStrengthenLV[j] = GetConn()->GetInt();
			}
			pItem[i].WuXingID = GetConn()->GetInt();
			pItem[i].WuXingPro = GetConn()->GetInt();
			pItem[i].SkillAForEquip = GetConn()->GetInt();
			pItem[i].SkillBForEquip = GetConn()->GetInt();
			for(int j = 0; j < MAX_EMBEDSLOTS; j++)
				pItem[i].EmbedSlot[j] = GetConn()->GetInt();
			for(int j = 0; j < 5; j++)
				pItem[i].reserve[j] = GetConn()->GetInt();

			pItem[i].dbVersion = GetConn()->GetInt();
			for(int j = 0; j < 4; j++)
				pItem[i].PromoteLv[j] = GetConn()->GetInt();
			for(int j = 0; j < 7; j++)
				pItem[i].UnuseField[j] = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存商铺出售灵兽栏
DBError TBLStallList::SaveSellPet(U16 stallid, stFixedStallPet* pet)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		stFixedStallPet* pInfo = (stFixedStallPet*)pet;
		char szRecord[8100] = {0};
		char szRow[2048] = {0};
		bool isFirst;
		int count = (MAX_FIXEDSTALL_SELLPET % 4) == 0? MAX_FIXEDSTALL_SELLPET / 4 : (MAX_FIXEDSTALL_SELLPET /4 + 1);
		for(int c = 0; c < count; c++)
		{
			strcpy_s(szRecord, 8100, "EXECUTE Sp_SaveStallSellPet '");
			isFirst = true;
			for(int i = c * 4; i < 4 * (c + 1); i++)
			{
				sprintf_s(szRow, 2048, "%s''%d'',''%I64d'',''%d'',''%d'',''%I64d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%I64d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',"\
					"''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d'',''%d''",
					isFirst ? "":";",
					pInfo[i].Price,
					pInfo[i].id,
					pInfo[i].petDataId,
					pInfo[i].petEggId,
					pInfo[i].partnerId,
					pInfo[i].sex,
					pInfo[i].style,

					pInfo[i].mature,
					pInfo[i].status,
					pInfo[i].generation,
					pInfo[i].title,
					pInfo[i].titleEnabled,
					pInfo[i].level,
					pInfo[i].lives,
					pInfo[i].curHP,
					pInfo[i].curMP,
					pInfo[i].curPP,

					pInfo[i].staminaPts,
					pInfo[i].ManaPts,
					pInfo[i].strengthPts,
					pInfo[i].intellectPts,
					pInfo[i].agilityPts,
					pInfo[i].pneumaPts,
					pInfo[i].statsPoints,
					pInfo[i].insight,
					pInfo[i].petTalent,
					pInfo[i].IsIdentify,
					pInfo[i].chengZhangLv,
					pInfo[i].qianLi,
					pInfo[i].quality,

					pInfo[i].staminaGift,
					pInfo[i].ManaGift,
					pInfo[i].strengthGift,
					pInfo[i].intellectGift,
					pInfo[i].agilityGift,
					pInfo[i].pneumaGift,
					pInfo[i].happiness,
					pInfo[i].exp,
					pInfo[i].charPoints,
					pInfo[i].LastBreedLv,
					pInfo[i].BabyPetDataId,
					pInfo[i].BreedStartTime,
					pInfo[i].Combat[0],

					pInfo[i].Combat[1],
					pInfo[i].Combat[2],
					pInfo[i].Combat[3],
					pInfo[i].Combat[4],
					pInfo[i].Combat[5],
					pInfo[i].Combat[6],
					pInfo[i].Combat[7],
					pInfo[i].Combat[8],
					pInfo[i].Combat[9],
					pInfo[i].Combat[10],
					pInfo[i].Combat[11],
					pInfo[i].Combo[0],
					pInfo[i].Combo[1],

					pInfo[i].Combo[2],
					pInfo[i].Combo[3],
					pInfo[i].Combo[4],
					pInfo[i].Combo[5],
					pInfo[i].Combo[6],
					pInfo[i].Combo[7],
					pInfo[i].Combo[8],
					pInfo[i].Combo[9],
					pInfo[i].Combo[10],
					pInfo[i].Combo[11],
					pInfo[i].activeSkillSlot[0],
					pInfo[i].activeSkillSlot[1],
					pInfo[i].randBuffId,

					pInfo[i].mLeftTime,
					pInfo[i].LockedLeftTime,
					pInfo[i].characters[0],
					pInfo[i].characters[1],
					pInfo[i].characters[2],
					pInfo[i].characters[3],
					pInfo[i].characters[4],
					pInfo[i].equipitem[0],
					pInfo[i].equipitem[1],
					pInfo[i].equipitem[2],
					pInfo[i].equipitem[3],

					pInfo[i].UnuseField[0],
					pInfo[i].UnuseField[1],
					pInfo[i].UnuseField[2],
					pInfo[i].UnuseField[3],
					pInfo[i].UnuseField[4],
					pInfo[i].UnuseField[5],
					pInfo[i].UnuseField[6],
					pInfo[i].UnuseField[7],
					pInfo[i].UnuseField[8],
					pInfo[i].UnuseField[9],
					pInfo[i].UnuseField[10],
					pInfo[i].UnuseField[11],
					pInfo[i].UnuseField[12],

					pInfo[i].UnuseField[13],
					pInfo[i].UnuseField[14],
					pInfo[i].UnuseField[15],
					pInfo[i].UnuseField[16],
					pInfo[i].UnuseField[17],
					pInfo[i].UnuseField[18],
					pInfo[i].UnuseField[19],
					pInfo[i].UnuseField[20],
					pInfo[i].UnuseField[21],
					pInfo[i].UnuseField[22],
					pInfo[i].UnuseField[23],
					pInfo[i].UnuseField[24],
					pInfo[i].UnuseField[25],

					pInfo[i].UnuseField[26],
					pInfo[i].UnuseField[27],
					pInfo[i].UnuseField[28],
					pInfo[i].UnuseField[29],
					pInfo[i].UnuseField[30],
					pInfo[i].dbVersion,

					stallid, i);
				strcat_s(szRecord, szRow);
				isFirst = false;
			}

			strcat_s(szRecord, "'");
			M_SQL(GetConn(),szRecord);
			GetConn()->Exec();
			err = DBERR_NONE;
		}
	}
	DBCATCH(stallid);
	return err;
}

// ----------------------------------------------------------------------------
// 加载商铺出售灵兽栏数据
DBError TBLStallList::LoadAllSellPet(stFixedStallPet* pPet)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pPet)
			throw ExceptionResult(DBERR_PLAYER_UNDELETE);

		U32 i, StallID, Pos;
		M_SQL(GetConn(),"EXECUTE Sp_LoadStallSellPet");
		while(GetConn()->More())
		{
			StallID = GetConn()->GetInt();
			Pos = GetConn()->GetInt();
			if(StallID >= MAX_FIXEDSTALLS || Pos >= MAX_FIXEDSTALL_SELLPET)
				continue;
			i = StallID * MAX_FIXEDSTALL_SELLPET + Pos;
			pPet[i].StallID = StallID;
			pPet[i].Pos = Pos;
			pPet[i].Price = GetConn()->GetInt();
			pPet[i].id = GetConn()->GetBigInt();
			pPet[i].petDataId = GetConn()->GetInt();
			pPet[i].petEggId = GetConn()->GetInt();
			pPet[i].partnerId = GetConn()->GetBigInt();
			pPet[i].sex = GetConn()->GetInt();
			pPet[i].style = GetConn()->GetInt();
			pPet[i].mature = (GetConn()->GetInt() != 0);
			pPet[i].status = GetConn()->GetInt();
			pPet[i].generation = GetConn()->GetInt();
			pPet[i].title = GetConn()->GetBigInt();
			pPet[i].titleEnabled = GetConn()->GetInt();
			pPet[i].level = GetConn()->GetInt();
			pPet[i].lives = GetConn()->GetInt();
			pPet[i].curHP = GetConn()->GetInt();
			pPet[i].curMP = GetConn()->GetInt();
			pPet[i].curPP = GetConn()->GetInt();
			pPet[i].staminaPts = GetConn()->GetInt();
			pPet[i].ManaPts = GetConn()->GetInt();
			pPet[i].strengthPts = GetConn()->GetInt();
			pPet[i].intellectPts = GetConn()->GetInt();
			pPet[i].agilityPts = GetConn()->GetInt();
			pPet[i].pneumaPts = GetConn()->GetInt();
			pPet[i].statsPoints = GetConn()->GetInt();
			pPet[i].insight = GetConn()->GetInt();
			pPet[i].petTalent = GetConn()->GetInt();
			pPet[i].IsIdentify = (GetConn()->GetInt() != 0);
			pPet[i].chengZhangLv = GetConn()->GetInt();
			pPet[i].qianLi = GetConn()->GetInt();
			pPet[i].quality = GetConn()->GetInt();
			pPet[i].staminaGift = GetConn()->GetInt();
			pPet[i].ManaGift = GetConn()->GetInt();
			pPet[i].strengthGift = GetConn()->GetInt();
			pPet[i].intellectGift = GetConn()->GetInt();
			pPet[i].agilityGift = GetConn()->GetInt();
			pPet[i].pneumaGift = GetConn()->GetInt();
			pPet[i].happiness = GetConn()->GetInt();
			pPet[i].exp = GetConn()->GetInt();
			pPet[i].charPoints = GetConn()->GetInt();
			pPet[i].LastBreedLv = GetConn()->GetInt();
			pPet[i].BabyPetDataId = GetConn()->GetInt();
			pPet[i].BreedStartTime = GetConn()->GetInt();
			for(int j = 0; j < stPetInfo::MaxPetSkills; j++)
				pPet[i].Combat[j] = GetConn()->GetInt();
			for(int j = 0; j < stPetInfo::MaxPetSkills; j++)
				pPet[i].Combo[j] = GetConn()->GetInt();
			for(int j = 0; j < stPetInfo::Type_Count; j++)
				pPet[i].activeSkillSlot[j] = GetConn()->GetInt();
			pPet[i].randBuffId = GetConn()->GetInt();
			pPet[i].mLeftTime = GetConn()->GetInt();
			pPet[i].LockedLeftTime = GetConn()->GetInt();
			for(int j = 0; j < stPetInfo::MaxCharType; j++)
				pPet[i].characters[j] = GetConn()->GetInt();
			for(int j = 0; j < stPetInfo::MaxPetEquips; j++)
				pPet[i].equipitem[j] = GetConn()->GetInt();	

			for(int j = 0; j < 31; j++)
				pPet[i].UnuseField[j] = GetConn()->GetInt();

			pPet[i].dbVersion = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 更新商铺收购物品栏指定槽位数据
DBError TBLStallList::UpdateBuyItem(stFixedStallItem* pItem)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_SMALLINT_CHECK(pItem->StallID);
		if(pItem->Pos >= MAX_FIXEDSTALL_BUYITEM)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_UpdateStallBuyItem %d,%d,%d,%d,%d,%I64d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			pItem->StallID, pItem->Pos,
			pItem->Price,
			pItem->BuyNum,
			pItem->LeftNum,
			pItem->UID,
			pItem->ItemID,
			pItem->Quantity,
			pItem->LapseTime,
			pItem->RemainUseTimes,

			pItem->LockedLeftTime,
			pItem->BindPlayer,
			pItem->BindFriend,
			pItem->ActiveFlag & 0x01,
			pItem->ActiveFlag & 0x02,
			pItem->ActiveFlag & 0x04,
			pItem->Quality,
			pItem->CurWear,
			pItem->CurMaxWear,
			pItem->CurAdroit,
			pItem->CurAdroitLv,
			pItem->ActivatePro,
			pItem->RandPropertyID,
			pItem->IDEProValue[0],
			pItem->IDEProValue[1],
			pItem->IDEProValue[2],
			pItem->IDEProValue[3],
			pItem->IDEProValue[4],
			pItem->IDEProValue[5],
			pItem->IDEProValue[6],
			pItem->IDEProValue[7],
			pItem->IDEProValue[8],
			pItem->IDEProValue[9],
			pItem->IDEProValue[10],
			pItem->IDEProValue[11],
			pItem->EquipStrengthens,
			
			pItem->EquipStrengthenLV[0],
			pItem->EquipStrengthenLV[1],
			pItem->EquipStrengthenLV[2],
			pItem->EquipStrengthenLV[3],
			pItem->EquipStrengthenLV[4],
			pItem->EquipStrengthenLV[5],
			pItem->EquipStrengthenLV[6],
			pItem->EquipStrengthenLV[7],
			pItem->EquipStrengthenLV[8],
			pItem->EquipStrengthenLV[9],
			pItem->EquipStrengthenLV[10],
			pItem->EquipStrengthenLV[11],
			pItem->EquipStrengthenLV[12],
			pItem->EquipStrengthenLV[13],
			pItem->EquipStrengthenLV[14],

			pItem->WuXingID,
			pItem->WuXingPro,
			pItem->SkillAForEquip,
			pItem->SkillBForEquip,
			pItem->EmbedSlot[0],
			pItem->EmbedSlot[1],
			pItem->EmbedSlot[2],
			pItem->EmbedSlot[3],
			pItem->EmbedSlot[4],
			pItem->EmbedSlot[5],
			pItem->EmbedSlot[6],
			pItem->EmbedSlot[7],
			pItem->reserve[0],
			pItem->reserve[1],
			pItem->reserve[2],
			pItem->reserve[3],
			pItem->reserve[4],

			pItem->dbVersion,
			pItem->PromoteLv[0],
			pItem->PromoteLv[1],
			pItem->PromoteLv[2],
			pItem->PromoteLv[3],
			pItem->UnuseField[0],
			pItem->UnuseField[1],
			pItem->UnuseField[2],
			pItem->UnuseField[3],
			pItem->UnuseField[4],
			pItem->UnuseField[5],
			pItem->UnuseField[6]);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 更新商铺出售物品栏指定槽位数据
DBError TBLStallList::UpdateSellItem(stFixedStallItem* pItem)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_SMALLINT_CHECK(pItem->StallID);
		if(pItem->Pos >= MAX_FIXEDSTALL_SELLITEM)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_UpdateStallSellItem %d,%d,%d,%I64d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			pItem->StallID, pItem->Pos,
			pItem->Price,
			pItem->UID,
			pItem->ItemID,
			pItem->Quantity,
			pItem->LapseTime,
			pItem->RemainUseTimes,
			pItem->LockedLeftTime,
			pItem->BindPlayer,
			pItem->BindFriend,
			pItem->ActiveFlag & 0x01,
			pItem->ActiveFlag & 0x02,
			pItem->ActiveFlag & 0x04,
			pItem->Quality,
			pItem->CurWear,
			pItem->CurMaxWear,
			pItem->CurAdroit,
			pItem->CurAdroitLv,
			pItem->ActivatePro,
			pItem->RandPropertyID,
			pItem->IDEProValue[0],
			pItem->IDEProValue[1],
			pItem->IDEProValue[2],
			pItem->IDEProValue[3],
			pItem->IDEProValue[4],
			pItem->IDEProValue[5],
			pItem->IDEProValue[6],
			pItem->IDEProValue[7],
			pItem->IDEProValue[8],
			pItem->IDEProValue[9],
			pItem->IDEProValue[10],
			pItem->IDEProValue[11],
			pItem->EquipStrengthens,
			pItem->EquipStrengthenLV[0],
			pItem->EquipStrengthenLV[1],
			pItem->EquipStrengthenLV[2],
			pItem->EquipStrengthenLV[3],
			pItem->EquipStrengthenLV[4],
			pItem->EquipStrengthenLV[5],
			pItem->EquipStrengthenLV[6],
			pItem->EquipStrengthenLV[7],
			pItem->EquipStrengthenLV[8],
			pItem->EquipStrengthenLV[9],
			pItem->EquipStrengthenLV[10],
			pItem->EquipStrengthenLV[11],
			pItem->EquipStrengthenLV[12],
			pItem->EquipStrengthenLV[13],
			pItem->EquipStrengthenLV[14],
			pItem->WuXingID,
			pItem->WuXingPro,
			pItem->SkillAForEquip,
			pItem->SkillBForEquip,
			pItem->EmbedSlot[0],
			pItem->EmbedSlot[1],
			pItem->EmbedSlot[2],
			pItem->EmbedSlot[3],
			pItem->EmbedSlot[4],
			pItem->EmbedSlot[5],
			pItem->EmbedSlot[6],
			pItem->EmbedSlot[7],
			pItem->reserve[0],
			pItem->reserve[1],
			pItem->reserve[2],
			pItem->reserve[3],
			pItem->reserve[4],
			
			pItem->dbVersion,
			pItem->PromoteLv[0],
			pItem->PromoteLv[1],
			pItem->PromoteLv[2],
			pItem->PromoteLv[3],
			pItem->UnuseField[0],
			pItem->UnuseField[1],
			pItem->UnuseField[2],
			pItem->UnuseField[3],
			pItem->UnuseField[4],
			pItem->UnuseField[5],
			pItem->UnuseField[6]);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 更新商铺出售灵兽栏指定槽位数据
DBError TBLStallList::UpdateSellPet(stFixedStallPet* pPet)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_SMALLINT_CHECK(pPet->StallID);
		if(pPet->Pos >= MAX_FIXEDSTALL_SELLPET)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_UpdateStallSellPet %d,%d,%d,%I64d,%d,%d,%I64d,%d,%d,"\
			"%d,%d,%d,%I64d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"\
			"%d,%d,%d,%d,%d,%d,%d,%d",
			pPet->StallID, pPet->Pos,
			pPet->Price,
			pPet->id,
			pPet->petDataId,
			pPet->petEggId,
			pPet->partnerId,
			pPet->sex,
			pPet->style,

			pPet->mature,
			pPet->status,
			pPet->generation,
			pPet->title,
			pPet->titleEnabled,
			pPet->level,
			pPet->lives,
			pPet->curHP,
			pPet->curMP,
			pPet->curPP,
			pPet->staminaPts,
			pPet->ManaPts,
			pPet->strengthPts,
			pPet->intellectPts,
			pPet->agilityPts,
			pPet->pneumaPts,
			pPet->statsPoints,
			pPet->insight,
			pPet->petTalent,
			pPet->IsIdentify,
			pPet->chengZhangLv,
			pPet->qianLi,
			pPet->quality,

			pPet->staminaGift,
			pPet->ManaGift,
			pPet->strengthGift,
			pPet->intellectGift,
			pPet->agilityGift,
			pPet->pneumaGift,
			pPet->happiness,
			pPet->exp,
			pPet->charPoints,
			pPet->LastBreedLv,
			pPet->BabyPetDataId,
			pPet->BreedStartTime,
			pPet->Combat[0],
			pPet->Combat[1],
			pPet->Combat[2],
			pPet->Combat[3],
			pPet->Combat[4],
			pPet->Combat[5],
			pPet->Combat[6],
			pPet->Combat[7],
			pPet->Combat[8],
			pPet->Combat[9],
			pPet->Combat[10],
			pPet->Combat[11],
			pPet->Combo[0],
			pPet->Combo[1],

			pPet->Combo[2],
			pPet->Combo[3],
			pPet->Combo[4],
			pPet->Combo[5],
			pPet->Combo[6],
			pPet->Combo[7],
			pPet->Combo[8],
			pPet->Combo[9],
			pPet->Combo[10],
			pPet->Combo[11],
			pPet->activeSkillSlot[0],
			pPet->activeSkillSlot[1],
			pPet->randBuffId,
			pPet->mLeftTime,
			pPet->LockedLeftTime,
			pPet->characters[0],
			pPet->characters[1],
			pPet->characters[2],
			pPet->characters[3],
			pPet->characters[4],
			pPet->equipitem[0],
			pPet->equipitem[1],
			pPet->equipitem[2],
			pPet->equipitem[3],
			
			pPet->UnuseField[0],
			pPet->UnuseField[1],
			pPet->UnuseField[2],
			pPet->UnuseField[3],
			pPet->UnuseField[4],
			pPet->UnuseField[5],
			pPet->UnuseField[6],
			pPet->UnuseField[7],
			pPet->UnuseField[8],
			pPet->UnuseField[9],
			pPet->UnuseField[10],
			pPet->UnuseField[11],
			pPet->UnuseField[12],

			pPet->UnuseField[13],
			pPet->UnuseField[14],
			pPet->UnuseField[15],
			pPet->UnuseField[16],
			pPet->UnuseField[17],
			pPet->UnuseField[18],
			pPet->UnuseField[19],
			pPet->UnuseField[20],
			pPet->UnuseField[21],
			pPet->UnuseField[22],
			pPet->UnuseField[23],
			pPet->UnuseField[24],
			pPet->UnuseField[25],

			pPet->UnuseField[26],
			pPet->UnuseField[27],
			pPet->UnuseField[28],
			pPet->UnuseField[29],
			pPet->UnuseField[30],
			pPet->dbVersion);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}