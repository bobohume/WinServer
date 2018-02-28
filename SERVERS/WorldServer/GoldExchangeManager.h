#ifndef __GOLDEXCHANGEMANAGER_H__
#define __GOLDEXCHANGEMANAGER_H__
/*
#include "Common/TimerMgr.h"
#include "Common/PacketType.h"
#include "Base/bitStream.h"
#include "Base/Locker.h"
#include "Common/MemGuard.h"
#include "wintcp/IPacket.h"
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"
#include "ServerMgr.h"
#include "LockCommon.h"
#include "ManagerBase.h"
#include "Common/GoldExchangeBase.h"

class GoldExChangeManager :public CManagerBase< GoldExChangeManager, MSG_GOLDEXCHANGE_BEGIN, MSG_GOLDEXCHANGE_END >
{
public:
	GoldExChangeManager();

	void Update();
	void UpdateConfig(stGoldConfig& _config);

	void HandleAWQueryMonthPays( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleCWQueryGoldExchange( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleCWQueryGoldConfig( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleCWDrawGoldExchange( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void SendWAQueryMonthPays(int AccountId, int PlayerId, const char* PlayerName, bool isDraw);
	void SendWCQueryMonthGolds(U32 PlayerID, S32 MonthGolds);
	void SendWCQueryGoldExchange(stGoldExchange* exchange);
	void SendWZQueryGoldConfig(int PlayerID);
	void SendWCDrawGoldExchange(int PlayerID, int type, int isOK, int Exchanges);
	
	void HandleCWQuerySRConfig(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleZWQuerySRBonus(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void HandleCWDrawSRBonus(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	void SendWCQuerySRConfig(U32, std::vector<stSR_Period*>&, std::vector<stSR_Bonus*>&,
		std::vector<stSR_History*>&);
	void SendWCQuerySRBonus(U32, stSR_History&);
	void SendWZDrawSRBonus(int error, U32, U32, S32 PerMonth, stSR_Bonus_Data&);
private:
	U32 Version;				//当前元宝兑换配置版本
	stGoldConfig	config;		//当前元宝兑换配置
};
*/
#endif//__GOLDEXCHANGEMANAGER_H__