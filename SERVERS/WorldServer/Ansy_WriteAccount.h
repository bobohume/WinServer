#ifndef _ANSYWRITEACCOUNT_
#define _ANSYWRITEACCOUNT_

#include "CommLib/ThreadPool.h"

struct AccoutWriteInfo;

class Ansy_WriteAccount : public ThreadBase
{
public:
    Ansy_WriteAccount(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
    U32 m_totalCount;
    AccoutWriteInfo* m_pAccounts;
};

#endif /*_ANSYWRITEACCOUNT_*/