#include "Define.h"
#include "Base/Log.h"
#include "common/PacketType.h"
#include "Common/PlayerStructEx.h"
//#include "zlib/zlib.h"
#include "dblib/dbLib.h"
#include "DBUtility.h"
#include "TBLPlayer.h"
#include "QuickLz/quicklz.h"
//#include "PlayerBlob/BlobHelp.h"

#define PLAYER_DATAFILE "cache\\"

DBError TBLPlayer::Load_Simple(int PlayerID,SimplePlayerData* pData)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		U_INT_CHECK(PlayerID);
		if(!pData)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"SELECT AccountID,PlayerName,Sex,[Level],Family,FirstClass,ZoneId,"\
		"LastLoginTime,LastLogoutTime,Gold,DrawGold,VipLv FROM Tbl_Player WHERE PlayerID=%d",PlayerID);
		if(GetConn()->More())
		{
			pData->AccountId  = GetConn()->GetInt();
			strcpy_s(pData->PlayerName,sizeof(pData->PlayerName),GetConn()->GetString());
			int Sex            = GetConn()->GetInt();
			int Level          = GetConn()->GetInt();
			int Family         = GetConn()->GetInt();
			int FirstClass	   = GetConn()->GetInt();
			int ZoneId		   = GetConn()->GetInt();
			int LastLoginTime  = GetConn()->GetTime();
			int LastLogoutTime = GetConn()->GetTime();
			int Gold		   = GetConn()->GetInt();
			int DarwGold	   = GetConn()->GetInt();
			U32 VipLv		   = GetConn()->GetInt();

			pData->PlayerId		  = PlayerID;
			pData->Sex            = Sex            >= 0 ? Sex : 0;
			pData->Level          = Level          >= 0 ? Level : 0;
			pData->Family         = Family         >= 0 ? Family : 0;
			pData->FirstClass	  = FirstClass	   >= 0 ? FirstClass : 0;
			pData->ZoneId		  = ZoneId		   >= 0 ? ZoneId : 0;
			pData->LastLoginTime  = LastLoginTime  >= 0 ? LastLoginTime : 0;
			pData->LastLogoutTime = LastLogoutTime >= 0 ? LastLogoutTime : 0;
			pData->Gold			  = Gold		   >= 0 ? Gold : 0;
			pData->DrawGold		  = DarwGold	   >= 0 ? DarwGold : 0;
			pData->VipLv		  = VipLv		   >= 0 ? VipLv : 0;
			
			err = DBERR_NONE;
		}
	}
	DBCATCH(PlayerID);
	return err;
}

DBError TBLPlayer::Load_Simple(const char* name,SimplePlayerData* pData)
{
	DBError err = DBERR_UNKNOWERR;
	name = name ? name : "";

	try
	{
		if(!StrSafeCheck(name, 32))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		U32 playerId = 0;
		do
		{
			if(!pData)
				throw ExceptionResult(DBERR_DB_SQLPARAMETER);

			std::stringstream s;
			s << "SELECT PlayerID FROM Tbl_Player WHERE PlayerName='";
			s << name << "'";
			GetConn()->SQLExt(s.str().c_str());
			//M_SQL(GetConn(),"SELECT PlayerID FROM Tbl_Player WHERE PlayerName='%s'",name);
			if(GetConn()->More())
			{
				playerId = GetConn()->GetInt();
			}
		}while(0);

		return Load_Simple(playerId,pData);
	}
	DBECATCH();
	return err;
}

DBError TBLPlayer::Save_Simple(U32 playerId,SimplePlayerData* pData)
{
	if (0 == pData)
		return DBERR_NONE;

	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_INT_CHECK(playerId);

		char szLoginTime[32] = {0};
		GetDBTimeString(pData->LastLoginTime, szLoginTime);

		char szLogoutTime[32] = {0};
		GetDBTimeString(pData->LastLogoutTime, szLogoutTime);

		M_SQL(GetConn(),"UPDATE Tbl_Player SET Sex=%d,[Level]=%d,Family=%d,FirstClass=%d,"\
			"ZoneId=%d,LastLoginTime='%s',LastLogoutTime='%s',Gold=%d, DrawGold=%d,VipLv=%d WHERE PlayerID=%d",
			pData->Sex,
			pData->Level,
			pData->Family,
			pData->FirstClass,
			pData->ZoneId,
			szLoginTime,
			szLogoutTime,
			pData->Gold,
			pData->DrawGold,
			pData->VipLv,
			playerId);

		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(playerId)
	return err;
}

DBError TBLPlayer::Save_Simple_DEL(U32 playerId,SimplePlayerData* pData)
{
	if (0 == pData)
		return DBERR_NONE;

	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_INT_CHECK(playerId);


		M_SQL(GetConn(),"UPDATE Tbl_Player_Deleted SET Sex=%d,[Level]=%d,Family=%d,FirstClass=%d,"\
			"FaceImage=%d,MasterLevel=%d,Activity=%d,MPMStudyLevel=%d WHERE PlayerID=%d",
			pData->Sex,
			pData->Level,
			pData->Family,
			pData->FirstClass,
			0,
			0,//pData->MasterLevel,
			0,//pData->Activity,
			0,//pData->MPMStudyLevel,
			playerId);

		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(playerId)
		return err;
}

// ----------------------------------------------------------------------------
// BLOB方式创建新角色
DBError TBLPlayer::Create_Simple(SimplePlayerData* pData)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_INT_CHECK(pData->AccountId);

		if(!StrSafeCheck(pData->PlayerName, 32))
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		M_SQL(GetConn(),"EXECUTE Sp_CreatePlayer %d,%d,'%s',%d,%d,%d", pData->AccountId, pData->PlayerId, pData->PlayerName,pData->Sex,pData->Family, pData->FirstClass);
		if(GetConn()->More())
		{
			int retCode = GetConn()->GetInt();
			switch(retCode)
			{
			case 0:
				{
					err = DBERR_NONE;
				}
				break;
			case -2:	//出现同名角色
				throw ExceptionResult(DBERR_PLAYER_NAMEEXISTS);
				break;
			case -3:	//角色数过多
				throw ExceptionResult(DBERR_PLAYER_TOOMANY);
				break;
			case -4:	//无法分配角色ID
				throw ExceptionResult(DBERR_PLAYER_NOASSIGNID);
				break;
			default:	//创建角色失败
				throw ExceptionResult(DBERR_PLAYER_CREATE);
				break;
			}
		}
	}
	DBECATCH()
	return err;
	return DBERR_NONE;
}

DBError TBLPlayer::ClearFirstLogin(int PlayerId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(), "UPDATE Tbl_Player SET FirstLogin=0 WHERE PlayerID=%d", PlayerId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
	return err;
}

DBError TBLPlayer::UpdateGold(int PlayerId, S32 Gold)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(), "EXECUTE Sp_UpdatePlayerGold %d,%d", PlayerId, Gold);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
	return err;
}

DBError TBLPlayer::UpdateDrawGold(int PlayerId, S32 Gold)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(), "EXECUTE Sp_UpdatePlayerDrawGold %d,%d", PlayerId, Gold);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
		return err;
}


DBError TBLPlayer::UpdateVipLv(int PlayerId, U32 VipLv)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(), "UPDATE Tbl_Player SET VipLv=%d WHERE PlayerID=%d", VipLv, PlayerId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
	return err;
}

DBError TBLPlayer::UpdateFirstClass(int PlayerId, S32 nVal)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(), "UPDATE Tbl_Player SET FirstClass=%d WHERE PlayerID=%d", nVal, PlayerId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
		return err;
}

DBError TBLPlayer::UpdateFamily(int PlayerId, S32 nVal)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(), "UPDATE Tbl_Player SET Family=%d WHERE PlayerID=%d", nVal, PlayerId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
		return err;
}