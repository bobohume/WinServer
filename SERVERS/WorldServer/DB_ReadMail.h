#ifndef _DBREADMAIL_H_
#define _DBREADMAIL_H_

#include "CommLib/ThreadPool.h"
#include "Common/MailBase.h"

class DB_ReadMail : public ThreadBase
{
public:
	DB_ReadMail(void);
	
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
    
    U32 m_socketHandle;
	U32 m_accountId;
    int m_mailId;
    int m_playerId;
};

#endif /*_DBREADMAIL_H_*/