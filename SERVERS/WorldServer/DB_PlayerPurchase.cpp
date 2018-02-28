#include "stdafx.h"
#include "DBContext.h"
#include "DBLayer/common/DBUtility.h"
#include "data/TBLBase.h"
#include "DB_PlayerPurchase .h"

DB_PlayerPurchase::DB_PlayerPurchase():nPlayerId(0), nGold(0), nMoney(0)
{	
}

int DB_PlayerPurchase::Execute(int ctxId,void* param)
{
	do 
	{
		CDBConn* pDBConn = (CDBConn*)param;
		assert(0 != pDBConn);

		DBError err = DBERR_UNKNOWERR;
		
		if(nGold < 0)
			break;

		M_SQL(pDBConn, "EXEC Sp_PlayerPurchase  %d, %d", nPlayerId, nGold);
		pDBConn->Exec();

	} while (false);

	return PACKET_OK;
}