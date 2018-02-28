/*
#include "GlobalEventManager.h"
#include "WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "DB_UpdateGlobalEvent.h"
#include "ServerMgr.h"
#include "DBLayer/Data/TBLGlobalEvent.h"
#include "Common/OrgBase.h"

CGlobalEventManager::CGlobalEventManager()
{
	GlobalEvents events;
	TBLGlobalEvent tblEvent(SERVER_CLASS_NAME::GetActorDB());
	IF_NOT_ASSERT(tblEvent.Load(events) == DBERR_NONE)
	{
		for each(const GlobalEvent& event in events)
			m_events[event.eventID] = event;
	}

	registerEvent(GLOBALEVENT_UPDATE, &CGlobalEventManager::HandleGlobalEventUpdate);
}

void CGlobalEventManager::EVENT_FUNCTION(HandleGlobalEventUpdate)
{
	DB_UpdateGlobalEvent* pDBHandle = new DB_UpdateGlobalEvent;
	pDBHandle->event.ReadData(Packet);

	SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}

void CGlobalEventManager::Update(const GlobalEvent& event)
{
	m_events[event.eventID] = event;
}

void CGlobalEventManager::NotifyZone(int lineID, int zoneID)
{
	int svrSocket = SERVER->GetServerManager()->GetGateSocket(lineID, zoneID);
	if (svrSocket>0 && m_events.size()>0)
	{
		char buf[MAX_PACKET_SIZE];
		Base::BitStream sendPacket(buf, sizeof(buf));
		stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, GLOBALEVENT_UPDATE, 0, SERVICE_ZONESERVER, zoneID);
		sendPacket.writeInt(m_events.size(), Base::Bit32);
		for (EventsIter iter=m_events.begin(); iter!=m_events.end(); ++iter)
			iter->second.WriteData(&sendPacket);
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(svrSocket, sendPacket);
	}
}
*/