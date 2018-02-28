#ifndef _DBCONTEXT_H_
#define _DBCONTEXT_H_

#include "CommLib/ThreadPool.h"
#include "base/WorkQueue.h"
#include "Common/OrgBase.h"
#include "ChatServer.h"
#include "CommLib\PacketBase.h"
#include "DBLayer\Data\TBLBase.h"

enum DBContext
{
	DB_CONTEXT_LOGIC,  
	DB_CONTEXT_NOMRAL,
};

static void PostLogicThread(ThreadBase* pDBHandle)
{
	IF_ASSERT(pDBHandle == NULL)
		return;

	SERVER->getWorkQueue()->PostEvent(0, pDBHandle, 0, false, WQ_LOGIC);
}

#endif /*_DBCONTEXT_H_*/