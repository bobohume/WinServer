#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "Common/PacketType.h"
#include "Common/LogicEventBase.h"
#include "DBUtility.h"
#include "TBLEventList.h"

// ----------------------------------------------------------------------------
// 查询所有系统事件数据(返回列表的指针用完需清除)
DBError TBLEventList::LoadAll(U32 nRecver, std::vector<stEventItem*>& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		U_INT_CHECK(nRecver);

		M_SQL(GetConn(),"EXECUTE Sp_LoadAllEvent %d", nRecver);
		while(GetConn()->More())
		{
			stEventItem* item = new stEventItem;
			item->id				= GetConn()->GetBigInt();
			item->sender			= GetConn()->GetInt();
			item->recver			= GetConn()->GetInt();
			item->event_state		= 0;
			item->event_type		= GetConn()->GetInt();
			item->timer				= GetConn()->GetTime();
			item->data_type			= GetConn()->GetInt();
			item->data_value		= GetConn()->GetInt();
			for(int i = 0; i < stEventItem::RESERVE_MAX; i++)
				item->reserve[i]	= GetConn()->GetInt();
			Info.push_back(item);
		}
		err = DBERR_NONE;
	}
	DBCATCH(nRecver)
	return err;
}

// ----------------------------------------------------------------------------
// 根据事件ID删除系统事件数据
DBError TBLEventList::Delete(stEventItem* item, int reason)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{	
		if(item == NULL || item->id >= 0X7FFFFFFFFFFFFFFF)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		if(item->id == 0)
		{
			//因为删除并备份每条系统事件都必须要有唯一ID
			err = Save(item);
			if(err != DBERR_NONE)
				return err;
		}
		
		M_SQL(GetConn(),"EXECUTE Sp_DeleteEvent %I64d,%d",item->id, reason);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 保存系统事件数据
DBError TBLEventList::Save(stEventItem* item)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(item == NULL || item->id >= 0X7FFFFFFFFFFFFFFF)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		U_INT_CHECK(item->sender);
		U_INT_CHECK(item->recver);
		U_TINYINT_CHECK(item->event_type);
		U_TINYINT_CHECK(item->data_type);
		INT_CHECK(item->data_value);
		INT_CHECK(item->reserve[0]);
		INT_CHECK(item->reserve[1]);
		INT_CHECK(item->reserve[2]);

		char szTimer[32] = {0};
		GetDBTimeString(item->timer, szTimer);
		M_SQL(GetConn(),"EXECUTE Sp_UpdateEvent  %I64d,%d,%d,%d,'%s',%d,%d,%d,%d,%d",
			item->id, item->sender, item->recver,
			item->event_type, szTimer,
			item->data_type, item->data_value,
			item->reserve[0],item->reserve[1], item->reserve[2]);
		if(GetConn()->More())
		{
			item->id = GetConn()->GetBigInt();
			if(item->id != 0)
				return DBERR_NONE;
		}
		throw ExceptionResult(DBERR_LOGICEVENT_SAVE);		
	}
	DBCATCH(item->recver)
	return err;
}

// ----------------------------------------------------------------------------
// 更新系统逻辑事件已处理/未处理标志
DBError TBLEventList::UpdateState(U64 id, int state)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		if(id > 0x7FFFFFFFFFFFFFFF)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);
		
		U_TINYINT_CHECK(state);

		M_SQL(GetConn(),"UPDATE Tbl_EventList SET event_state=%d WHERE id=%I64d", state, id);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 更新系统逻辑事件已处理/未处理标志
DBError TBLEventList::BatchUpdateState(std::vector<U64>& idList, int state)
{
	char szRecord[8000] = {0};
	char szRow[128] = {0};
	bool isfirst = true;
	for(size_t i = 0; i < idList.size(); i++)
	{
		if(isfirst)
		{
			sprintf_s(szRow, 128, "EXECUTE Sp_BatchUpdateEventState %d, '%I64d", state, idList[i]);
			isfirst = false;
		}
		else
		{
			sprintf_s(szRow, 128, ",%I64d", idList[i]);
		}		
		strcat_s(szRecord, szRow);
	}

	if(!isfirst)
		strcat_s(szRecord, "'");

	DBError err = DBERR_UNKNOWERR;
	try
	{		
		U_TINYINT_CHECK(state);
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