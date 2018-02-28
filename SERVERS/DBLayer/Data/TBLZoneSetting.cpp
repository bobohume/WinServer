#include "Define.h"
#include "Base/Log.h"
#include "Common/ZoneSetting.h"
#include "dblib/dbLib.h"
#include "TBLZoneSetting.h"
#include "DBUtility.h"

// ----------------------------------------------------------------------------
// 查询载入地图配置项
DBError TBLZoneSetting::Load(stZoneSetting& info, int lineid, int zoneid)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		U_SMALLINT_CHECK(lineid);
		U_INT_CHECK(zoneid);

		M_SQL(GetConn(),"EXECUTE Sp_LoadZoneSetting %d,%d", lineid, zoneid);
		if (GetConn()->More())
		{
			info.lineid		= GetConn()->GetInt();
			info.zoneid		= GetConn()->GetInt();
			info.exprate	= GetConn()->GetInt();
			info.droprate	= GetConn()->GetInt();
            info.timeStart  = GetConn()->GetBigInt();
            info.timeEnd    = GetConn()->GetBigInt();

			for(int i = 0; i < MAX_RESERVE_FIELDS; i++)
				info.reserve[i] = GetConn()->GetInt();		
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存地图配置项
DBError TBLZoneSetting::Save(stZoneSetting* zoneset, int lineid, int zoneid)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_SMALLINT_CHECK(lineid);
		U_INT_CHECK(zoneid);

		M_SQL(GetConn(),"EXECUTE Sp_SaveZoneSetting %d,%d,%d,%d,%I64d,%I64d,%d,%d,%d,%d,%d,%d",
			lineid, zoneid, zoneset->exprate, zoneset->droprate,
            zoneset->timeStart,
            zoneset->timeEnd,
			zoneset->reserve[0],
			zoneset->reserve[1],
			zoneset->reserve[2],
			zoneset->reserve[3],
			zoneset->reserve[4],
			zoneset->reserve[5]
			);

		if(!GetConn()->More() || GetConn()->GetInt() != 0)
			throw ExceptionResult(DBERR_ZONESETTING_SAVE);

		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}