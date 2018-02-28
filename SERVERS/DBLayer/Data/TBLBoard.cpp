#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "DBUtility.h"
#include "TBLBoard.h"

DBError TBLBoard::save( stBoardMsg& msg )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		unsigned long titleSize = _countof(msg.title)*2;
		TempAlloc<char> pTitle(titleSize);
		SafeDBString(msg.title, _countof(msg.title), pTitle, titleSize);

		unsigned long msgSize = _countof(msg.msg)*2;
		TempAlloc<char> pMsg(msgSize);
		SafeDBString(msg.msg, _countof(msg.msg), pMsg, msgSize);


		M_SQL(GetConn(), "EXECUTE Sp_SaveBoard %d,%d,%d,%d,%d,%d,%d,%d,%d,'%s','%s'",
			msg.playerId, msg.type, msg.gender, msg.family, msg.levelHigh, msg.levelLow,
			msg.hot, msg.hot1, msg.hot2, (char*)pTitle, (char*)pMsg );
		if(GetConn()->More())
		{
			msg.id = GetConn()->GetInt();
		}		
		err = DBERR_NONE;
	}
	DBCATCH( msg.playerId );
	return err;
}

DBError TBLBoard::load( int id, stBoardMsg& msg )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(), "SELECT * FROM Tbl_Board WHERE id=%d", id );
		if( GetConn()->More() )
		{
			msg.playerId = GetConn()->GetInt();
			msg.gender = GetConn()->GetInt();
			msg.levelHigh = GetConn()->GetInt();
			msg.levelLow = GetConn()->GetInt();
			msg.family = GetConn()->GetInt();
			msg.type = GetConn()->GetInt();			
			strcpy_s( msg.title, sizeof( msg.title ), GetConn()->GetString() );
			strcpy_s( msg.msg, sizeof( msg.msg ), GetConn()->GetString() );
			msg.hot = GetConn()->GetInt();
			msg.hot1 = GetConn()->GetInt();
			msg.hot2 = GetConn()->GetInt();
			msg.postTime = GetConn()->GetTime();		// 跳过time字段
			msg.time = GetConn()->GetInt();			// 取有效天数
			msg.id = GetConn()->GetInt();
		}
		err = DBERR_NONE;
	}
	DBCATCH( id );
	return err;
}

DBError TBLBoard::load( int playerId, int type, stBoardMsg& msg )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(), "SELECT * FROM Tbl_Board WHERE playerid=%d AND [type]=%d", playerId, type );
		if( GetConn()->More() )
		{
			msg.playerId = GetConn()->GetInt();
			msg.gender = GetConn()->GetInt();
			msg.levelHigh = GetConn()->GetInt();
			msg.levelLow = GetConn()->GetInt();
			msg.family = GetConn()->GetInt();
			msg.type = GetConn()->GetInt();
			strcpy_s( msg.title, sizeof( msg.title ), GetConn()->GetString() );
			strcpy_s( msg.msg, sizeof( msg.msg ), GetConn()->GetString() );
			msg.hot = GetConn()->GetInt();
			msg.hot1 = GetConn()->GetInt();
			msg.hot2 = GetConn()->GetInt();
            msg.postTime = GetConn()->GetTime();		// 跳过time字段
			msg.time = GetConn()->GetInt();			// 取有效天数
			msg.id = GetConn()->GetInt();

			err = DBERR_NONE;
		}
	}
	DBCATCH( playerId );
	return err;
}

DBError TBLBoard::load( int type, std::list< stBoardMsg >& msgList, int pageNo, int pageCount, int& totalPage )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_LoadBoard %d,%d,%d", type, pageNo, pageCount );
		stBoardMsg msg;
		while(GetConn()->Fetch())
		{
			int index = GetConn()->GetRecordSetIndex();
			if(index == 2)		//行数，总页数，当前页数,总记录数
			{
				int totalrow	= GetConn()->GetInt();
				totalPage	    = GetConn()->GetInt();
				int curpage		= GetConn()->GetInt();
				int totalCount  = GetConn()->GetInt();
			}
			else if(index == 3) //每页数据
			{
				msg.playerId = GetConn()->GetInt();
				msg.gender = GetConn()->GetInt();
				msg.levelHigh = GetConn()->GetInt();
				msg.levelLow = GetConn()->GetInt();
				msg.family = GetConn()->GetInt();
				msg.type = GetConn()->GetInt();
				strcpy_s( msg.title, sizeof( msg.title ), GetConn()->GetString() );
				strcpy_s( msg.msg, sizeof( msg.msg ), GetConn()->GetString() );
				msg.hot = GetConn()->GetInt();
				msg.hot1 = GetConn()->GetInt();
				msg.hot2 = GetConn()->GetInt();
				msg.postTime = GetConn()->GetTime();		// 跳过time字段
				msg.time = GetConn()->GetInt();			// 取有效天数
				msg.id = GetConn()->GetInt();
				msgList.push_back( msg );				
			}
		}
		err = DBERR_NONE;
	}
	DBCATCH(type);
	return err;
}

DBError TBLBoard::del( int playerId, int type )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"DELETE FROM Tbl_Board WHERE playerId=%d AND [type]=%d", playerId, type );
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH( playerId );
	return err;
}

DBError TBLBoard::addHot1( int id )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(), "UPDATE Tbl_Board SET hot=hot+3,hot1=hot1+1" \
			" WHERE id=%d AND hot<0x7FFFFFFC AND hot1<0x7FFFFFFE", id);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH( id );
	return err;
}

DBError TBLBoard::addHot2( int id )
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(), "UPDATE Tbl_Board SET hot=hot+1,hot2=hot2+1"\
			" WHERE id=%d AND (hot<0x7FFFFFFE AND hot2<0x7FFFFFFE)", id);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBCATCH( id );
	return err;
}