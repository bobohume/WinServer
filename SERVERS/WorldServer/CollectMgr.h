#ifndef COLLECT_MANAGER_H
#define COLLECT_MANAGER_H
/*
#include <hash_map>
#include "Common/CollectEventBase.h"
#include "ManagerBase.h"

class CCollectManager 
	:public CManagerBase<CCollectManager, MSG_COLLECT_BEGIN, MSG_COLLECT_END>
{
private:
	typedef stdext::hash_map<int, stCollectEvent> CollectEvents;
	typedef CollectEvents::iterator CollectsIter;
	typedef CollectEvents::const_iterator CollectsConIter;

public:
	CCollectManager();

	bool     isOpen                (int iEventId);
	bool     isZoneId              (int zoneId);
	bool     isClosed              (const stCollectEvent& collect);
	bool     isComplete            (const stCollectEvent& collect);
	void     SendCollectToZone     (int sockId, int zoneId);

    void     SetCollect(stCollectEvent& event);
	
	DECLARE_EVENT_FUNCTION(HandleCollect);

private:
	CollectEvents m_collectEvents;
};
*/
#endif

