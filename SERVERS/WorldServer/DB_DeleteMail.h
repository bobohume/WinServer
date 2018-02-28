#ifndef _DBDELETEMAIL_H_
#define _DBDELETEMAIL_H_

#include "CommLib/ThreadPool.h"
#include "Common/MailBase.h"
#include <vector>

class DB_DeleteMail : public ThreadBase
{
public:
	DB_DeleteMail(void);
	
	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
    
    U32 m_socketHandle;
	U32 m_accountId;
    std::vector<int> m_idlist;
    int m_nRecver;
};

#endif /*_DBDELETEMAIL_H_*/