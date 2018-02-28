#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "DBUtility.h"
#include "TBLGlobalEvent.h"

TBLGlobalEvent::TBLGlobalEvent(DataBase* db)
:TBLBase(db)
{
}

DBError TBLGlobalEvent::Load(GlobalEvents& events)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{	
		GlobalEvent event;
		M_SQL(GetConn(), "SELECT * FROM Tbl_GlobalEvent");
		while (GetConn()->More())
		{			
			event.eventID = GetConn()->GetInt();
			event.eventData1 = GetConn()->GetInt();
			strcpy_s(event.eventData2, sizeof(event.eventData2), GetConn()->GetString());
			events.push_back(event);
		}
		err = DBERR_NONE;
	}
	DBSCATCH("TBLGlobalEvent::Load")
	return err;
}

DBError TBLGlobalEvent::Update(const GlobalEvent& event)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (event.eventID <= 0)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(), "EXECUTE Sp_UpdateGlobalEvent %d, %d, '%s'", 
			event.eventID, event.eventData1, event.eventData2);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(event.eventID)
	return err;
}