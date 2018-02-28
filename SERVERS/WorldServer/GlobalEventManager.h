#ifndef GLOBALEVENT_MANAGER_H
#define GLOBALEVENT_MANAGER_H
/*
#include "base/bitStream.h"
#include "common/PacketType.h"
#include "ManagerBase.h"
#include <hash_map>
#include "Common/GlobalEventBase.h"

class CGlobalEventManager 
	:public CManagerBase<CGlobalEventManager, MSG_GLOBALEVENT_BEGIN, MSG_GLOBALEVENT_END>
{
public:
	CGlobalEventManager();

	DECLARE_EVENT_FUNCTION(HandleGlobalEventUpdate);

	void NotifyZone(int lineID, int zoneID);
	void Update(const GlobalEvent& event);

private:
	typedef stdext::hash_map<int, GlobalEvent> Events;
	typedef Events::iterator EventsIter;

	Events m_events;
};
*/

#endif