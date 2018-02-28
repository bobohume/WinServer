#include "stdafx.h"
#include "DB_UpdatePlayerFlag.h"
#include "DBLayer/common/DBUtility.h"
#include "DBLayer/data/TBLAccount.h"
#include "DBLayer/Data//TBLBase.h"
#include "DBContext.h"
#include "Common/Define.h"

DB_UpdatePlayerFlag::DB_UpdatePlayerFlag(void): mPlayerId(0), mPlayerFlag(0)
{
}

int DB_UpdatePlayerFlag::Execute(int ctxId,void* param)
{
	do 
	{
		CDBConn* pDBConn = (CDBConn*)param;
		assert(0 != pDBConn);

		DBError err = DBERR_UNKNOWERR;
		try
		{	
			U32 nPlayerFlag = 0;
			M_SQL(pDBConn, "SELECT PlayerFlag FROM Tbl_Player  WHERE PlayerID = %d", mPlayerId);
			if(pDBConn->More())
			{
				nPlayerFlag = pDBConn->GetInt();
			}

			if(nPlayerFlag & mPlayerFlag)
			{
				nPlayerFlag =  (nPlayerFlag & (~mPlayerFlag));
				M_SQL(pDBConn, "UPDATE Tbl_Player SET PlayerFlag = %d WHERE PlayerID = %d", nPlayerFlag, mPlayerId);
				pDBConn->Exec();
			}

			err = DBERR_NONE;
		}
		DBECATCH()
	} while (false);

	return PACKET_OK;
}