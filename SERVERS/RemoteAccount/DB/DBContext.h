#ifndef _DBCONTEXT_H_
#define _DBCONTEXT_H_

#include "CommLib/ThreadPool.h"
#include "base/WorkQueue.h"
#include "Common/OrgBase.h"
#include "../RemoteAccount.h"
#include "CommLib\PacketBase.h"
#include "DBLayer\Data\TBLBase.h"

static void PostLogicThread(ThreadBase* pDBHandle)
{
	IF_ASSERT(pDBHandle == NULL)
		return;

	CRemoteAccount::getInstance()->getWorkQueue()->PostEvent(0, pDBHandle, 0, false, WQ_LOGIC);
}

#endif /*_DBCONTEXT_H_*/