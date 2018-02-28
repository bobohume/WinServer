#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLBanList.h"
#include "DBUtility.h"

// ----------------------------------------------------------------------------
// 删除过期的Ban
DBError TBLBanList::DeleteForOverTime(int curTime)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"DELETE FROM Tbl_BanList WHERE loginBan<%d AND chatBan<%d", curTime);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(curTime)
	return err;
}

// ----------------------------------------------------------------------------
// 删除目标类型的Ban
DBError TBLBanList::Delete(const char* target, int type)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!StrSafeCheck(target, 32))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"DELETE FROM Tbl_BanList WHERE target='%s' AND [type]=%d", target, type);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(type)
	return err;
}

// ----------------------------------------------------------------------------
// 更新Ban数据
DBError TBLBanList::Update(stBan& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		if(!StrSafeCheck(Info.target, _countof(Info.target)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if(!StrSafeCheck(Info.loginBanReason, _countof(Info.loginBanReason)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		if(!StrSafeCheck(Info.chatBanReason, _countof(Info.chatBanReason)))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_UpdateBan '%s',%d,%d,%d,'%s','%s'",
				Info.target,Info.type, Info.loginBan, Info.chatBan,
				Info.loginBanReason, Info.chatBanReason);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(Info.type)
	return err;
}

// ----------------------------------------------------------------------------
// 查询Ban列表
DBError TBLBanList::Load(std::vector<stBan>& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"SELECT target,[type],loginBan,chatBan,loginBanReason,chatBanReason FROM Tbl_BanList");
		stBan ban;
		while(GetConn()->More())
		{			
			strcpy_s(ban.target, 32, GetConn()->GetString());
			ban.type				= GetConn()->GetInt();
			ban.loginBan			= GetConn()->GetInt();
			ban.chatBan				= GetConn()->GetInt();
			strcpy_s(ban.loginBanReason, 50, GetConn()->GetString());
			strcpy_s(ban.chatBanReason, 50, GetConn()->GetString());
			Info.push_back(ban);
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}