#ifndef _DBDELETEALLMAIL_H_
#define _DBDELETEALLMAIL_H_

#include "CommLib/ThreadPool.h"
#include "Common/MailBase.h"

class DB_DeleteAllMail : public ThreadBase
{
public:
	DB_DeleteAllMail(void);
	
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
    
    U32 m_socketHandle;
    int m_playerId;
};

#endif /*_DBDELETEALLMAIL_H_*/