#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "DBUtility.h"
#include "TBLActorKeyMap.h"

DBError TBLActorKeyMap::load( int playerId, stKeyMapBase& keyMaps )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		bool isFound = false;
		M_SQL(GetConn(), "SELECT soultion FROM Tbl_PlayerKeyMap WHERE playerId=%d", playerId );
		while( GetConn()->More() )
		{
			keyMaps.keyMapSoultion = GetConn()->GetInt();
			isFound = true;
		}

		if(isFound )
		{
			GetConn()->SetBlobOpt();
			M_SQL(GetConn(), "SELECT keys FROM Tbl_PlayerKeyMap WHERE playerId=%d", playerId );
			if(GetConn()->Eval())
			{
				int len = sizeof( keyMaps.keyMaps );
				GetConn()->GetBlob( len, (PBYTE)keyMaps.keyMaps );				
			}
		}
		err = DBERR_NONE;
	}
	DBCATCH( playerId );
	return err;
}

DBError TBLActorKeyMap::save( int playerId, stKeyMapBase& keyMaps )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{				
		M_SQL(GetConn(), "EXECUTE Sp_UpdatePlayerKeyMap %d,%d", playerId, keyMaps.keyMapSoultion );
		GetConn()->Exec();

		GetConn()->SetBlobOpt();
		M_SQL(GetConn(), "SELECT keys FROM Tbl_PlayerKeyMap WHERE playerId=%d", playerId );
		if(GetConn()->Eval())
			GetConn()->SetBlob( "Tbl_PlayerKeyMap", "keys", (PBYTE)keyMaps.keyMaps, sizeof( keyMaps.keyMaps ) );
		err = DBERR_NONE;
	}
	DBCATCH( playerId );
	return err;
}