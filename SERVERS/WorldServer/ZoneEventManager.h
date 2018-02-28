/*#include <hash_map>
#include "ManagerBase.h"
#include "LockCommon.h"
#include "Common/GlobalEvent.h"


class CZoneEventManager : public CManagerBase< CZoneEventManager, MSG_ZONEEVENT_BEGIN, MSG_ZONEEVENT_END >
{
public:
	CZoneEventManager(void);
	~CZoneEventManager(void);

	void HandleSetSeverEvent(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	void HandleClientGetSeverEvent(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);

	stEventData m_eventData[MAX_EVENT];
};*/
