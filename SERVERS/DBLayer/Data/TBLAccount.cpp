#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLAccount.h"
#include "DBUtility.h"
#include "Common/PacketType.h"
#include "Common/PlayerStruct.h"

// ----------------------------------------------------------------------------
// 根据帐号名称查询帐号ID
DBError TBLAccount::QueryID(const char* AccountName, int& AccountID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!StrSafeCheck(AccountName, ACCOUNT_NAME_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"SELECT AccountID FROM Tbl_Account WHERE AccountName='%s'", AccountName);
		if(GetConn()->More())
			AccountID = GetConn()->GetInt();
		else
			throw ExceptionResult(DBERR_ACCOUNT_NOFOUND);
		err = DBERR_NONE;
	}
	DBSCATCH(AccountName)
	return err;
}

// ----------------------------------------------------------------------------
// 根据帐号ID查询帐号
TBLAccount::ALoginType TBLAccount::QueryAccountById(int AccountId)
{
	DBError err = DBERR_UNKNOWERR;
	ALoginType nLogin = Login_Max;
	try
	{
		//登录方式：快速登录，还是普通登录
		//0：帐号没注册，1：快速登录AccountId登录，2：普通帐号密码登录

		M_SQL(GetConn(),"EXECUTE Sp_CheckAccountExist '%d'", AccountId);
		if(GetConn()->More())
		{
			nLogin = (ALoginType)GetConn()->GetInt();
		}
		else 
			throw ExceptionResult(DBERR_UNKNOWERR);
	}
	DBSCATCH(AccountId)
	return nLogin;
}

// ----------------------------------------------------------------------------
// 保存登录信息
DBError TBLAccount::SaveLoginInfo(int AccountID, const char* LoginIP, int LoginError)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!StrSafeCheck(LoginIP, 20))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_UpdateAccountLogin  %d,'%s',%d", AccountID, LoginIP ? LoginIP:"0.0.0.0", LoginError);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(AccountID)
	return err;
}

void TBLAccount::GetAccountInfo(stAccount* Info)
{
	Info->AccountID			= GetConn()->GetInt();
	dStrcpy(Info->AccountName, ACCOUNT_NAME_LENGTH, GetConn()->GetString());
	Info->IsAdult			= GetConn()->GetInt() != 0;
	Info->Mode				= GetConn()->GetInt();
	Info->Status			= GetConn()->GetInt();
	Info->GMFlag			= GetConn()->GetInt();
	Info->LoginStatus		= GetConn()->GetInt();
	Info->LoginTime			= GetConn()->GetTime();
	Info->LogoutTime		= GetConn()->GetTime();
	dStrcpy(Info->LoginIP, 20, GetConn()->GetString());
	Info->LoginError		= GetConn()->GetInt();
	Info->TotalOnlineTime	= GetConn()->GetInt();
	Info->AccountFunction	= GetConn()->GetInt();
	Info->dTotalOnlineTime	= GetConn()->GetInt();
	Info->dTotalOfflineTime	= GetConn()->GetInt();
}

void TBLAccount::GetAccountInfo_GM(stAccount* Info)
{
	Info->AccountID			= GetConn()->GetInt();
	dStrcpy(Info->AccountName, ACCOUNT_NAME_LENGTH, GetConn()->GetString());
	Info->IsAdult			= GetConn()->GetInt() != 0;
	Info->Mode				= GetConn()->GetInt();
	Info->Status			= GetConn()->GetInt();
	Info->GMFlag			= GetConn()->GetInt();
	Info->LoginStatus		= GetConn()->GetInt();
	Info->LoginTime			= GetConn()->GetTime();
	Info->LogoutTime		= GetConn()->GetTime();
	dStrcpy(Info->LoginIP, 20, GetConn()->GetString());
	Info->LoginError		= GetConn()->GetInt();
	Info->TotalOnlineTime	= GetConn()->GetInt();
	Info->AccountFunction	= GetConn()->GetInt();
	Info->dTotalOnlineTime	= GetConn()->GetInt();
	Info->dTotalOfflineTime	= GetConn()->GetInt();
}

// ----------------------------------------------------------------------------
// 根据帐号名称查询帐号数据
DBError TBLAccount::Load(const char* AccountName, stAccount* pInfo)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		if(!StrSafeCheck(AccountName, ACCOUNT_NAME_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"SELECT       \
						 [AccountId]   \
						,[AccountName]\
						,[IsAdult]\
						,[Mode]\
						,[Status]\
						,[GMFlag]\
						,[LoginStatus]\
						,[LoginTime]\
						,[LogoutTime]\
						,[LoginIP]\
						,[LoginError]\
						,[TotalOnlineTime]\
						,[AccountFunction]\
						,[dTotalOnlineTime]\
						,[dTotalOfflineTime] FROM Tbl_Account WHERE AccountName='%s'", AccountName);
		if(GetConn()->More())
			GetAccountInfo(pInfo);
		else
			throw ExceptionResult(DBERR_ACCOUNT_NOFOUND);
		err = DBERR_NONE;
	}
	DBSCATCH(AccountName)
	return err;
}

// ----------------------------------------------------------------------------
// 根据帐号名称查询帐号数据
DBError	TBLAccount::Load(int AccountID, stAccount* pInfo)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"SELECT       \
						[AccountId]   \
						,[AccountName]\
						,[IsAdult]\
						,[Mode]\
						,[Status]\
						,[GMFlag]\
						,[LoginStatus]\
						,[LoginTime]\
						,[LogoutTime]\
						,[LoginIP]\
						,[LoginError]\
						,[TotalOnlineTime]\
						,[AccountFunction]\
						,[dTotalOnlineTime]\
						,[dTotalOfflineTime] FROM Tbl_Account WHERE AccountId=%d", AccountID);
		if(GetConn()->More())
			GetAccountInfo(pInfo);
		else
			throw ExceptionResult(DBERR_ACCOUNT_NOFOUND);
		err = DBERR_NONE;
	}
	DBSCATCH(AccountID);
	return err;
}
// ----------------------------------------------------------------------------
// 根据帐号名查询登录IP
DBError	TBLAccount::QueryLoginIP(const char* AccountName, int& AccountID, char* LoginIP)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!StrSafeCheck(AccountName, ACCOUNT_NAME_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"SELECT AccountID,LoginIP FROM Tbl_Account WHERE AccountName='%s'",AccountName);
		if(GetConn()->More())
		{
			AccountID			= GetConn()->GetInt();
			dStrcpy(LoginIP, 20, GetConn()->GetString());
		}
		else
			throw ExceptionResult(DBERR_ACCOUNT_NOFOUND);
		err = DBERR_NONE;
	}
	DBSCATCH(AccountName)
	return err;
}

// ----------------------------------------------------------------------------
// 更新帐号登录时间
DBError TBLAccount::UpdateLoginTime(int AccountID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_UpdateLoginTime  %d", AccountID);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(AccountID)
	return err;
}

DBError TBLAccount::UpdateAccountLogoutTime(int AccountID,const char* time)
{
    DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_UpdateAccountLogoutTime  %d,'%s'", AccountID,time);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(AccountID)
	return err;
}

// ----------------------------------------------------------------------------
// 更新帐号登出时间
DBError TBLAccount::UpdateLogoutTime(int AccountID, int CostTime, int dTotalOfflineTime, int dTotalOnlineTime)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_UpdateLogoutTime  %d,%d,%d,%d", 
			AccountID, CostTime, dTotalOfflineTime, dTotalOnlineTime);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(AccountID)
	return err;
}

// ----------------------------------------------------------------------------
// 创建帐号
DBError TBLAccount::MakeAccount(const char* AccountName, int& accountId, int flag, bool bApple /*= false*/)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_INT_CHECK(accountId);
		U_TINYINT_CHECK(flag);

		if (!StrSafeCheck(AccountName, ACCOUNT_NAME_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(), "EXECUTE Sp_MakeAccount '%s', %d, %d", AccountName, accountId, (int)flag);
		if (GetConn()->More())
		{
			int ret = GetConn()->GetInt();
			if(ret == -1)
				throw ExceptionResult(DBERR_ACCOUNT_IDEXISTS);	//帐号ID已经存在但帐号名不存在
			else if(ret == -2)
				throw ExceptionResult(DBERR_ACCOUNT_NAMEEXISTS);//帐号名称不存在但帐号ID不存在
			else if (ret == -3)
				throw ExceptionResult(DBERR_ACCOUNT_IDNAMENOMATCH);	//帐号ID与帐号名不匹配
			else
			{
				accountId = GetConn()->GetInt();
				err = DBERR_NONE;
			}
		}
		else
			throw ExceptionResult(DBERR_ACCOUNT_NOFOUND);
	}
	DBSCATCH(AccountName)
	return err;
}

// ----------------------------------------------------------------------------
// 检查帐号是否存在
DBError TBLAccount::CheckAccount( const char* AccountName, bool& isExisted )
{
	isExisted = false;
	int AccountID;
	DBError err = QueryID(AccountName, AccountID);
	if(err == DBERR_NONE)
	{
		if(AccountID <= 0)
			err = DBERR_ACCOUNT_NOFOUND;
		else
			isExisted = true;			
	}
	return err;
}

// ----------------------------------------------------------------------------
// 查询GM帐号
DBError TBLAccount::QueryGMAccount(int GMFlag, std::vector<std::pair<int, std::string> >& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_TINYINT_CHECK(GMFlag);

		M_SQL(GetConn(),"SELECT AccountID,AccountName FROM Tbl_Account WHERE GMFlag=%d", GMFlag);
		while(GetConn()->More())
		{
			int v1         = GetConn()->GetInt();
			std::string v2 = GetConn()->GetString();
			Info.push_back(std::make_pair(v1,v2));
		}
		err = DBERR_NONE;
	}
	DBCATCH(GMFlag)
	return err;
}

// ----------------------------------------------------------------------------
// 更新GM帐号标志
DBError TBLAccount::UpdateGMFlag(const char* AccountName, int GMFlag)
{
	DBError err = DBERR_UNKNOWERR;	
	try
	{
		U_TINYINT_CHECK(GMFlag);

		if(!StrSafeCheck(AccountName, ACCOUNT_NAME_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"UPDATE Tbl_Account SET GMFlag=%d WHERE AccountName='%s'", GMFlag, AccountName);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBSCATCH(AccountName)
	return err;
}

// ----------------------------------------------------------------------------
// GM工具查询帐号（条件查询）
DBError TBLAccount::QueryForGM(int Mode, const char* Key, const char* Condition, std::vector<stAccount>& AccountInfo)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_TINYINT_CHECK(Mode);

		if(!StrSafeCheck(Key, ACCOUNT_NAME_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		//Condition没有作SQL攻击检查，调用处安全可以忽略
		//if(!StrSafeCheck(Condition, strlen(Condition)))

		std::stringstream s;
		if(Mode == 0|| Mode == 2)
        {
			s << "EXEC Sp_QueryForGM 0,'";
			s << Key << "',";
			s << Mode << ",'";
			s << Condition << "'";
			//M_SQL(GetConn(),"EXECUTE Sp_QueryForGM 0,'%s',%d,'%s'", Key, Mode, Condition);
        }
		else 
        {
			int idKey = atoi(Key);
			s << "EXEC Sp_QueryForGM ";
			s << idKey << ",'',";
			s << Mode << ",'";
			s << Condition << "'";
			//M_SQL(GetConn(),"EXECUTE Sp_QueryForGM %s,'',%d,'%s'", Key, Mode, Condition);
        }

		GetConn()->SQLExt(s.str().c_str());
		while(GetConn()->Fetch())
		{
			stAccount Info;
			GetAccountInfo_GM(&Info);
			AccountInfo.push_back(Info);
		}
		err = DBERR_NONE;
	}
	DBSCATCH(Key)
	return err;
}

// ----------------------------------------------------------------------------
// 更新帐号状态
DBError TBLAccount::SetAccountStatus( int accountId, unsigned int status )
{
	DBError err = DBERR_UNKNOWERR;	
	try
	{
		U_INT_CHECK(accountId);		
		INT_CHECK(status);

		M_SQL(GetConn(),"UPDATE Tbl_Account SET [Status]=%d WHERE AccountID=%d", status, accountId );
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(accountId)
	return err;
}

// ----------------------------------------------------------------------------
// 更新帐号状态(status为0时置1) 
DBError TBLAccount::SetAccountStatus( int accountId)
{
	DBError err = DBERR_UNKNOWERR;	
	try
	{
		U_INT_CHECK(accountId);	

		M_SQL(GetConn(),"UPDATE Tbl_Account SET [Status]=1 WHERE AccountID=%d AND [Status]=0", accountId );
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(accountId)
	return err;
}

DBError TBLAccount::CheckFirstLoginStatus( int accountId, int playerId, const char* playName, int bonusCount )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if (bonusCount < 0)
			bonusCount = 0;
		
		U_INT_CHECK(accountId);
		U_INT_CHECK(playerId);

		if(!StrSafeCheck(playName, COMMON_STRING_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_FirstLoginBonus %d, %d,'%s', %d", accountId, playerId, playName, bonusCount );
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(accountId)
	return err;
}

DBError TBLAccount::CheckOldPlayerLogin( int accountId, int playerId, const char* playName )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
 
		U_INT_CHECK(accountId);
		U_INT_CHECK(playerId);

		if(!StrSafeCheck(playName, COMMON_STRING_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_OldPlayerLoginBonus %d, %d,'%s'", accountId, playerId, playName );
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(accountId)
		return err;
}

DBError TBLAccount::LoadPlayerList(int AccountID, Players& playerList)
{
    DBError err = DBERR_UNKNOWERR;

	try
	{		
		U_INT_CHECK(AccountID);
		M_SQL(GetConn(), "SELECT top(18) PlayerID FROM Tbl_Player WHERE AccountID=%d order by [Level] DESC", AccountID);

		while (GetConn()->More())
		{
			PlayerInfo info;
			info.playerID = GetConn()->GetInt();

            playerList.push_back(info);
		}

		err = DBERR_NONE;
	}
	DBCATCH(AccountID)
	return err;
}

DBError	TBLAccount::UpdateNetBarDrawTime( int accountId,int index )
{
	/*DBError err = DBERR_UNKNOWERR;	
	try
	{
		U_INT_CHECK(accountId);	

		M_SQL(GetConn(),"EXECUTE Sp_UpdateNetBarDrawTime %d,%d", accountId,index );
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(accountId)
		return err;*/
	return DBERR_NONE;
}

DBError	TBLAccount::QueryNetBarDrawTime( int accountId,int* pTimeList)
{
	DBError err = DBERR_UNKNOWERR;	
	try
	{
		U_INT_CHECK(accountId);	

		M_SQL(GetConn(),"SELECT NetBarDrawTime1,NetBarDrawTime2,NetBarDrawTime3,"\
			"NetBarDrawTime4,NetBarDrawTime5,NetBarDrawTime6,NetBarDrawTime7,"\
			"NetBarDrawTime8,NetBarDrawTime9,NetBarDrawTime10 FROM Tbl_Account WHERE AccountId=%d", accountId);
		if(GetConn()->More())
		{
			for(int i = 0; i < MAX_NETBAR_DRAW; i++)
				pTimeList[i] = GetConn()->GetTime();
		}
		err = DBERR_NONE;
	}
	DBCATCH(accountId)
	return err;
}
