#ifndef _DBCREATEPLAYER_
#define _DBCREATEPLAYER_

#include "CommLib/ThreadPool.h"
#include "AccountHandler.h"

class DB_CreatePlayer : public ThreadBase
{
public:
    DB_CreatePlayer(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    int  m_error;
    int  m_playerPos;
    int  m_accountStatus;
    bool m_isOpenedTime;

    PlayerSimpleDataRef m_spData;  
};

#endif /*_DBCREATEPLAYER_*/