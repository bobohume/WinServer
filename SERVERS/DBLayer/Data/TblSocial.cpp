#include "Define.h"
#include "Base/Log.h"
#include "dblib/dbLib.h"
#include "DBUtility.h"
#include "Common/SocialBase.h"
#include "TBLSocial.h"

//=============================================================================
// 社会关系数据操作
// TBLSocial
//=============================================================================
// ----------------------------------------------------------------------------
// 载入某角色的所有社会关系
DBError TBLSocial::Load(int PlayerID, SocialItemMap& info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"SELECT target,[type],friendvalue FROM Tbl_Social WHERE player=%d", PlayerID);
		stSocialItem social;
		while(GetConn()->More())
		{
			social.playerId		= GetConn()->GetInt();
			social.type			= GetConn()->GetInt();
			social.friendValue	= GetConn()->GetInt();
			info[social.playerId] = social;
		}
		err = DBERR_NONE;
	}
	DBCATCH(PlayerID)
	return err;
}

// ----------------------------------------------------------------------------
// 保存某角色的所有社会关系
DBError TBLSocial::Save(int PlayerID, SocialItemMap& info)
{
	char szRecord[8100] = {0};
	char szRow[128] = {0};
	bool isfirst = true;
	SocialItemMap::iterator it = info.begin();
	for(; it != info.end(); ++it)
	{
		stSocialItem& stItem = it->second;
		if(isfirst)
		{
			sprintf_s(szRow, 128, "EXECUTE Sp_SaveSocial %d,'%d,%d,%d,%d",
				PlayerID,PlayerID, stItem.playerId, stItem.type,stItem.friendValue);
			isfirst = false;
		}
		else
		{
			sprintf_s(szRow, 128, ";#,%d,%d,%d",stItem.playerId,stItem.type,stItem.friendValue);
		}

		strcat_s(szRecord, 8100, szRow);
	}

	if(!isfirst)
		strcat_s(szRecord,8100,"'");

	DBError err = DBERR_UNKNOWERR;
	try
	{
		
		U_INT_CHECK(PlayerID);
		if(szRecord[0])
		{
			M_SQL(GetConn(),szRecord);
			GetConn()->Exec();
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

//保存某角色的某个目标的社会关系
DBError TBLSocial::Save(int playerId, const stSocialItem& social)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_SaveSingleSocial %d,%d,%d,%d",
			playerId, social.playerId, social.type, social.friendValue);

		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(playerId)
		return err;
}

DBError TBLSocial::Update(const LimitMap& friendValue)
{
	char szRecord[8100] = {0};
	char szRow[128] = {0};
	int i = 0;
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		for each(const std::pair<PlayerIdPair,int>& it in friendValue)
		{
			if(i == 0)
			{
				sprintf_s(szRow, 128, "EXECUTE Sp_UpdateSocialFriendValue '%d,%d,%d",
					it.first.first,it.first.second,it.second);
			}
			else
			{
				sprintf_s(szRow, 128, ";%d,%d,%d",
					it.first.first,it.first.second,it.second);
			}

			strcat_s(szRecord, 8100, szRow);
			i++;

			if(i == 260)
			{
				strcat_s(szRecord, 8100, "'");
				M_SQL(GetConn(),szRecord);
				GetConn()->Exec();
				i = 0;
				szRecord[0] = szRow[0] = 0;
			}
		}

		if(i != 0)
		{
			strcat_s(szRecord, 8100, "'");
			M_SQL(GetConn(),szRecord);
			GetConn()->Exec();
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 更新某角色的某个目标的社会关系
DBError TBLSocial::Update(int PlayerID, const stSocialItem& social)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_UpdateSingleSocial %d,%d,%d,%d",PlayerID,social.playerId,
			social.type,social.friendValue);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerID)
		return err;
}

// ----------------------------------------------------------------------------
// 更新某角色与某个目标角色两者社会关系
DBError TBLSocial::Update(int PlayerID, const stSocialItem& social,int destType)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_UpdateDoubleSocial %d,%d,%d,%d,%d",PlayerID,social.playerId,
			social.type,destType,social.friendValue);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerID)
	return err;
}

// ----------------------------------------------------------------------------
// 删除某角色的所有社会关系
DBError TBLSocial::Delete(int PlayerID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"DELETE FROM Tbl_Social WHERE player=%d", PlayerID);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerID)
	return err;
}
// ----------------------------------------------------------------------------
// 删除某角色的所有社会关系及关联所有社会关系(用于删号)
DBError TBLSocial::DeleteAll(int PlayerId)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"DELETE FROM Tbl_Social WHERE player=%d OR target=%d", PlayerId,PlayerId);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerId)
		return err;
}
// ----------------------------------------------------------------------------
// 删除某角色与目标角色的社会关系
DBError TBLSocial::DeleteTarget(int PlayerID, int TargetID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_DeleteTargetSocial %d,%d",PlayerID,TargetID);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerID)
	return err;
}

// ----------------------------------------------------------------------------
// 删除某角色的某类型所有社会关系
DBError TBLSocial::DeleteType(int PlayerID, int type)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"DELETE FROM Tbl_Social WHERE player=%d AND [type]=%d", PlayerID, type);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerID)
	return err;
}

//for gm
DBError TBLSocial::UpdateForGm(int PlayerID, const stSocialItem& social,int destType)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_GMUpdateSocial %d,%d,%d,%d,%d",PlayerID,social.playerId,
			social.type,destType,social.friendValue);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(PlayerID)
		return err;
}
