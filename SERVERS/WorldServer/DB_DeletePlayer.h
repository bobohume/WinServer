#ifndef _DBDELETEPLAYER_
#define _DBDELETEPLAYER_

#include "CommLib/ThreadPool.h"
#include "AccountHandler.h"

class DB_DeletePlayer : public ThreadBase
{
public:
    DB_DeletePlayer(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32  m_accountId;
    U32  m_playerId;
    int  m_error;
    bool m_isDeleteRecord;
    bool m_isFromZone;
};

#endif /*_DBDELETEPLAYER_*/