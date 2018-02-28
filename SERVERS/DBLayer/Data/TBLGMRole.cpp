#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLGMRole.h"
#include "DBUtility.h"

DBError TBLGMRole::Load(std::vector<stGMRole>& gmroleInfo)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"SELECT gmFlag,gmCmd FROM Tbl_GMRole");
		stGMRole role;
		while(GetConn()->More())
		{
			role.GMFlag			= GetConn()->GetInt();
			strcpy_s(role.GMCmd, 60, GetConn()->GetString());
			gmroleInfo.push_back(role);
		}
		err = DBERR_NONE;
	}
	DBECATCH();
	return err;
}

// ----------------------------------------------------------------------------
// 更新GM权限标志
DBError TBLGMRole::Save(int gmflag, const char* gmcmd)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{			
		if(!StrSafeCheck(gmcmd, strlen(gmcmd)+1))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"INSERT Tbl_GMRole (gmFlag,gmCmd) VALUES(%d,'%s')", gmflag, gmcmd);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(gmflag)
	return err;
}

// ----------------------------------------------------------------------------
// 删除GM权限标志
DBError TBLGMRole::Delete(int gmflag, const char* gmcmd)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		if(!StrSafeCheck(gmcmd, strlen(gmcmd)+1))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"DELETE FROM Tbl_GMRole WHERE gmFlag=%d AND gmCmd='%s'", gmflag, gmcmd);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(gmflag)
	return err;
}