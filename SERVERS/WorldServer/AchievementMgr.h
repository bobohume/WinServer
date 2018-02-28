#ifndef __ACHIEVEMENT_H__
#define __ACHIEVEMENT_H__

#include <hash_map>
#include "ManagerBase.h"
#include "LockCommon.h"
#include "Event.h"
#include "base\bitStream.h"

class CAchManager : ILockable,  public CManagerBase< CAchManager, MSG_ACH_BEGIN, MSG_ACH_END >
{
public:
	CAchManager();
	virtual ~CAchManager();

	//void HandleZoneAchievementRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	//void HandleZoneAchRecordRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleZoneSaveAchDataRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleZoneSaveAchRecordRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );

	void SendAchievementInfo(int nPlayerId);
	void SendAchRecInfo(int nPlayerId);
};

#endif

