#ifndef _DBSAVE_BADWORD_PLAYER_
#define _DBSAVE_BADWORD_PLAYER_

#include "CommLib/ThreadPool.h"
#include "../WorldServer/ChatMgr.h"

class DB_SaveBadWordPlayer : public ThreadBase
{
public:
    DB_SaveBadWordPlayer(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
	int		m_playerId;
	char	m_badWord[MAX_BADWORDS_LENGTH];
};

#endif 