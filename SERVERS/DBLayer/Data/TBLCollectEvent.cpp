#include "Define.h"
#include "Base/Log.h"
#include "Common/CollectEventBase.h"
#include "dblib/dbLib.h"
#include "TBLCollectEvent.h"
#include "DBUtility.h"

// ----------------------------------------------------------------------------
// 查询收集物品事件数据
DBError TBLCollectEvent::Load(std::vector<stCollectEvent>& info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"SELECT * FROM Tbl_CollectEvent WHERE Complete = 0");
		while (GetConn()->More())
		{
			stCollectEvent e;
			e.eventid		= GetConn()->GetInt();
			for(int i = 0; i < stCollectEvent::Max_Items; i++)
				e.itemnum[i] = GetConn()->GetInt();
			e.openTime = GetConn()->GetTime();
			e.bComplete = GetConn()->GetInt() == 0 ? false : true;
			info.push_back(e);
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存收集物品事件数据
DBError TBLCollectEvent::Save(const stCollectEvent& info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_SaveCollectEvent %d,%d,%d,%d,%d,%d,%d",
			info.eventid,
			info.itemnum[0],
			info.itemnum[1],
			info.itemnum[2],
			info.itemnum[3],
			info.itemnum[4],
			info.bComplete);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(info.eventid)
	return err;
}