#include "TBLExecution.h"
#include "dblib/dbLib.h"

DBError TBLExecution::Commint()
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),m_sql.c_str());
		if(m_pFunction)
		{
			err = DBERR_NONE;
			m_pFunction(GetId(), err, (void *)GetConn());
		}
		else
		{
			err = DBERR_NONE;
			GetConn()->Exec();
		}
	}
	DBECATCH();
	return err;
}