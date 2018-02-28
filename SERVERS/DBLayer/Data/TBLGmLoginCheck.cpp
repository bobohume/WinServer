#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLGmLoginCheck.h"
#include "DBUtility.h"
#include "Common/PacketType.h"

DBError	TBLGmLoginCheck::QueryIP(const char* LoginIP,char* mac)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!StrSafeCheck(LoginIP, IP_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"SELECT MacAddr FROM Tbl_GMLoginCheck WHERE LoginIP='%s'", LoginIP);
		if(GetConn()->More())
		{
			strcpy_s(mac,256,GetConn()->GetString());
			err = DBERR_NONE;
		}
		else
		{
			err = DBERR_ACCOUNT_NOFOUND;
			throw ExceptionResult(DBERR_ACCOUNT_NOFOUND);
		}
	}
	DBSCATCH(LoginIP)
		return err;

}

DBError TBLGmLoginCheck::QueryMAC(const char* LoginMac,char* ip)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!StrSafeCheck(LoginMac, MAC_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"SELECT LoginIP FROM Tbl_GMLoginCheck WHERE MacAddr LIKE '%s'", LoginMac);
		if(GetConn()->More())
		{
			strcpy_s(ip,20,GetConn()->GetString());
			err = DBERR_NONE;
		}
		else
		{
			err = DBERR_ACCOUNT_NOFOUND;
			throw ExceptionResult(DBERR_ACCOUNT_NOFOUND);
		}
	}
	DBSCATCH(LoginMac)
		return err;

}

DBError TBLGmLoginCheck::QueryMACandIP(const char* LoginMac,const char* ip)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!StrSafeCheck(LoginMac, MAC_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if(!StrSafeCheck(ip, IP_LENGTH))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"SELECT LoginIP FROM Tbl_GMLoginCheck WHERE MacAddr LIKE '%s' AND LoginIP='%s'", LoginMac,ip);
		if(GetConn()->More())
		{
			err = DBERR_NONE;
		}
		else
		{
			err = DBERR_ACCOUNT_NOFOUND;
			throw ExceptionResult(DBERR_ACCOUNT_NOFOUND);
		}
	}
	DBSCATCH(LoginMac)
		return err;

}