#ifndef _DBTRYGETMAILITEM_H_
#define _DBTRYGETMAILITEM_H_

#include "CommLib/ThreadPool.h"
#include "Common/MailBase.h"

class DB_TryGetMailItem : public ThreadBase
{
public:
	DB_TryGetMailItem(void);
	
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
    
    U32 m_socketHandle;
	U32 m_accountId;
    int m_nMailId;
    int m_nPlayerId;
	stMailItem m_mailItem;
};

#endif /*_DBTRYGETMAILITEM_H_*/