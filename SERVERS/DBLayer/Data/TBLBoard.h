#ifndef TBL_BOARD_H
#define TBL_BOARD_H


#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include "common/BoardBase.h"

class CDBConn;

class TBLBoard : public TBLBase
{
public:
	TBLBoard(DataBase* db) : TBLBase( db ) {}
	virtual ~TBLBoard() {}

	DBError save( stBoardMsg& msg );
	DBError load( int id, stBoardMsg& msg );
	DBError load( int type, std::list< stBoardMsg >& msgList, int pageNo, int pageCount, int& totalPage );
	DBError load( int playerId, int type, stBoardMsg& msg );
    DBError del( int playerId, int type );
	DBError addHot1( int id );
	DBError addHot2( int id );
};

#endif /*TBL_BOARD_H*/