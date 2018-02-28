#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include <time.h>
#include "TBLReward.h"
#include "Common/RewardBase.h"

TBLReward::TBLReward(DataBase* db):TBLBase(db)
{
}

TBLReward::~TBLReward()
{
}

// ----------------------------------------------------------------------------
// ²éÑ¯½±Àø²Ù×÷
DBError TBLReward::Query(int AccountID, stAccountReward* reward)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"SELECT TOP 1 RewardID,RewardType,RewardValue,RewardNum,IsAuto "\
			"FROM Tbl_Reward WHERE AccountID = %d AND IsDraw = 0", AccountID);
		if(GetConn()->More())
		{
			reward->AccountID = AccountID;
			reward->RewardID = GetConn()->GetInt();
			reward->RewardType = GetConn()->GetInt();
			reward->RewardValue = GetConn()->GetInt();
			reward->RewardNum = GetConn()->GetInt();
			reward->IsAuto = GetConn()->GetInt() != 0 ? true : false;
		}
		err = DBERR_NONE;
	}
	DBCATCH(AccountID)
	return err;
}

// ----------------------------------------------------------------------------
// ÖØÖÃ½±Àø²Ù×÷
DBError TBLReward::Reset(int AccountID, int RewardID)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"UPDATE Tbl_Reward SET IsDraw=0 WHERE AccountID=%d AND RewardID=%d",
			AccountID, RewardID);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH(RewardID)
	return err;
}

// ----------------------------------------------------------------------------
// ¸üÐÂ½±Àø²Ù×÷
DBError TBLReward::Update(stAccountReward* reward)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"EXECUTE Sp_UpdateAccountReward %d,%d,%d",
			reward->PlayerID, reward->AccountID, reward->RewardID);
		if(GetConn()->More() && GetConn()->GetInt() == 0)
			err = DBERR_NONE;
		else
			throw ExceptionResult(DBERR_UNKNOWERR);
	}
	DBCATCH(reward->RewardID)
	return err;
}