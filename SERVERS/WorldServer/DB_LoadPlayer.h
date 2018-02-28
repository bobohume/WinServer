#ifndef _DB_LOADPLAYER_H_
#define _DB_LOADPLAYER_H_

#include "CommLib/ThreadPool.h"
#include "Common/PlayerStruct.h"

class DB_LoadPlayer : public ThreadBase
{
public:
	enum
	{
		DB_LOAD_NONE					=	0,
		DB_LOAD_TBL_PALYER_ERROR,
		DB_LOAD_TBL_PALYERBASICINFO_ERROR,
	};
	DB_LoadPlayer();
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32 m_playerId;
    stPlayerStruct m_pPlayer;
	U32 m_error;
};

#endif /*_DBSETACCOUNTSTATUS_*/