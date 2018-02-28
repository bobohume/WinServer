#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLWorld.h"
#include "Common/PlayerStruct.h"
#include "Common/ZoneSetting.h"

// ----------------------------------------------------------------------------
// 查询天劫数据
DBError TBLWorld::QueryDisaster(int& TopLevel, int& TopDisaster)
{
	TopLevel = Default_Player_Top_Level;
	TopDisaster	= 0;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"SELECT TopLevel,TopDisaster FROM Tbl_World");
		if(GetConn()->More())
		{
			TopLevel	= GetConn()->GetInt();
			TopDisaster	= GetConn()->GetInt();

			if (Default_Player_Top_Level > TopLevel)
			{
				TopLevel = Default_Player_Top_Level;
			}
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存天动数据TopDisaster
DBError TBLWorld::SaveDisaster(int TopDisaster)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_SaveTopDisaster %d",
			TopDisaster);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存天动数据TopLevel
DBError TBLWorld::SaveTopLevel(int TopLevel)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_SaveTopLevel %d",
			TopLevel);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

DBError TBLWorld::SaveWorldSetting(stWorldSetting worldSetting)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_SaveWorldSetting %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",worldSetting.TopLevel,
			worldSetting.TopDisaster,worldSetting.ActivityFlag,worldSetting.reserve[0],worldSetting.reserve[1],
			worldSetting.reserve[2],worldSetting.reserve[3],worldSetting.reserve[4],worldSetting.reserve[5],worldSetting.reserve[6],
			worldSetting.reserve[7],worldSetting.MaxStalls,worldSetting.TotalFlourish,worldSetting.Years5Assure,worldSetting.GodGivePath);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

//查询world配置表
DBError TBLWorld::QueryWorldSetting(stWorldSetting& worldSetting)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"SELECT * FROM Tbl_World");
		if(GetConn()->More())
		{
			worldSetting.TopLevel		= GetConn()->GetInt();
			worldSetting.TopDisaster	= GetConn()->GetInt();
			worldSetting.ActivityFlag	= GetConn()->GetInt();
			
			for (int i = 0; i < MAX_WORLDSETS; ++i)
			{
				worldSetting.reserve[i] = GetConn()->GetInt();
			}
			worldSetting.MaxStalls		= GetConn()->GetInt();
			worldSetting.TotalFlourish  = GetConn()->GetInt();
			worldSetting.Years5Assure	= GetConn()->GetInt();
			worldSetting.GodGivePath	= GetConn()->GetInt();
			if (0 == worldSetting.TopLevel)
			{
				worldSetting.TopLevel = Default_Player_Top_Level;
			}
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存商铺关键设置
DBError TBLWorld::SaveStallShop(int MaxStalls, int TotalFlourish)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_SaveStallShopSetting %d,%d", MaxStalls, TotalFlourish);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}
