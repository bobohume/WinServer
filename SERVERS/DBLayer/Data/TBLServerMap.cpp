#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLServerMap.h"

DBError TBLServerMap::Load(std::vector<stZoneInfo>& ZoneInfo)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"SELECT MapID,MapType,MaxPlayerNum,ServicePort,MapSq FROM Tbl_ServerMap");		
		while(GetConn()->More())
		{
			stZoneInfo info;
			info.ZoneId			= GetConn()->GetInt();
			info.MapType        = GetConn()->GetInt();
			info.MaxPlayerNum	= GetConn()->GetInt();
			info.ServicePort	= GetConn()->GetInt();
			info.Sequence		= GetConn()->GetInt();
			ZoneInfo.push_back(info);
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}