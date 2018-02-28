#ifndef _DBCONTEXT_H_
#define _DBCONTEXT_H_

#include "CommLib/ThreadPool.h"
#include "base/WorkQueue.h"
#include "WorldServer.h"

enum DBContext
{
    DB_CONTEXT_LOGIC = 0,       //Âß¼­Ïß³Ì

    DB_CONTEXT_NOMRAL,      //normal db manager
	DB_CONTEXT_CHILD,
};

static void PostLogicThread(ThreadBase* pDBHandle)
{
    if (0 == pDBHandle)
    {
        assert(0 != pDBHandle);
        return;
    }

    SERVER->GetWorkQueue()->PostEvent(0,pDBHandle,sizeof(pDBHandle),false,WQ_LOGIC);
}

#endif /*_DBCONTEXT_H_*/