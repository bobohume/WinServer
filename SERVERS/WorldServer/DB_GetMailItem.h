#ifndef _DBGETMAILITEM_H_
#define _DBGETMAILITEM_H_

#include "CommLib/ThreadPool.h"
#include "Common/MailBase.h"

class DB_ResetGetMailFlag : public ThreadBase
{
public:
	DB_ResetGetMailFlag(void);
	virtual int Execute(int ctxId,void* param);
    int  m_nMailId;
};

#endif /*_DBGETMAILITEM_H_*/