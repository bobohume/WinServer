#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "DBUtility.h"
#include "TblGoldHistory.h"

DBError TBLGoldHistory::QueryGoldCostMonth(stdext::hash_map<int,int>& TopGold)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"SELECT top(100) PlayerID, sum(Gold) FROM [Tbl_Gold_History] where DATEDIFF(month, getdate(), LogDate)=0 AND[OpType] in (1,15) group by PlayerID");
		while(GetConn()->More())
		{
			int playerID = GetConn()->GetInt();
			int Gold	 = GetConn()->GetInt();
			TopGold.insert(std::make_pair(playerID,Gold));
		}
		err = DBERR_NONE;
	}
	DBECATCH();
	return err;

}

DBError TBLGoldHistory::QueryGoldCostTotal(stdext::hash_map<int,int>& TopGold)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"SELECT top(100) PlayerID, sum(Gold) \
			FROM [Tbl_Gold_History] where [OpType] in (1,15) group by PlayerID");
		while(GetConn()->More())
		{
			int playerID = GetConn()->GetInt();
			int Gold	 = GetConn()->GetInt();
			TopGold.insert(std::make_pair(playerID,Gold));
		}
		err = DBERR_NONE;
	}
	DBECATCH();
	return err;

}

