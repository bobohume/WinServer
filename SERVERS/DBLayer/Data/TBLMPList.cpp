#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLMPList.h"
#include "DBUtility.h"
#include <string>

// ----------------------------------------------------------------------------
// 载入师傅列表
DBError TBLMPList::Load(int playerID, VecLearnPrentice& info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		U_INT_CHECK(playerID);
		M_SQL(GetConn(),"SELECT playerId FROM Tbl_MPList WHERE masterId=%d", playerID);
		stLearnPrentice stList;
		while (GetConn()->More())
		{			
			stList.playerId = GetConn()->GetInt();
			info.push_back(stList);
		}		
		err = DBERR_NONE;
	}
	DBCATCH(playerID)
	return err;
}

// ----------------------------------------------------------------------------
// 保存师傅列表
DBError TBLMPList::Save(int playerId,int masterPlayerId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"INSERT Tbl_MPList(playerId,masterId) VALUES(%d,%d)",playerId,masterPlayerId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

DBError TBLMPList::Count(MasterCount& masterCount)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"SELECT masterId, COUNT(*) FROM Tbl_MPList GROUP BY masterId");
		std::pair<int, int> count;
		while (GetConn()->More())
		{			
			count.first = GetConn()->GetInt();
			count.second = GetConn()->GetInt();
			masterCount.push_back(count);
		}		
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}