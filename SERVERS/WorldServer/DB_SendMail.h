#ifndef _DBSENDMAIL_H_
#define _DBSENDMAIL_H_

#include "CommLib/ThreadPool.h"
#include "Common/MailBase.h"

class DB_SendMail : public ThreadBase
{
public:
	DB_SendMail(void);
	virtual int Execute(int ctxId,void* param);    
    U32        m_socketHandle;
    int        m_nRecver;
    stMailItem m_mailItem;
    int        m_playerId;
    int        m_error;
};

#endif /*_DBSENDMAIL_H_*/