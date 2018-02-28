#ifndef _DBCHECKOLDPLAYERLOGIN_
#define _DBCHECKOLDPLAYERLOGIN_

#include "CommLib/ThreadPool.h"

class DB_CheckOldPlayerLogin : public ThreadBase
{
public:
    DB_CheckOldPlayerLogin(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32 m_accountId;
    U32 m_playerId;
    std::string m_playerName;
};

#endif /*_DBCHECKOLDPLAYERLOGIN_*/