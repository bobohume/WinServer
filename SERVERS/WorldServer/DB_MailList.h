#ifndef _DBMAILLIST_H_
#define _DBMAILLIST_H_

#include "CommLib/ThreadPool.h"
#include "Common/MailBase.h"

class DB_MailList : public ThreadBase
{
public:
	DB_MailList(void);
	
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
    
    U32 m_socketHandle;
    U32 m_accountId;

    int m_playerId;
	int m_recvCount;
	int m_noReadCount;
};

#endif /*_DBMAILLIST_H_*/