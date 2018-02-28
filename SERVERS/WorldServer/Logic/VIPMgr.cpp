#include "VIPMgr.h"
#include "VIPData.h"
#include "../Database/DB_Execution.h"
#include "Common/MemGuard.h"
#include "../PlayerMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "BuffData.h"
#include "BuffMgr.h"
#include "PlayerInfoMgr.h"
#include "ItemMgr.h"
#include "TimeSetMgr.h"
#include "PrinceMgr.h"
#include "../Script/lua_tinker.h"

extern CVIPDataRes * g_VIPDataMgr;

CVIPMgr::CVIPMgr() : MAXVIPLEVEL(12)
{
	REGISTER_EVENT_METHOD("WW_VIP_RECHARGE", this, &CVIPMgr::HandleRecharge);

	REGISTER_EVENT_METHOD("CW_VIP_GIFT_REQUEST", this, &CVIPMgr::HandleClientVipGiftRequest);

	g_VIPDataMgr->read();
}


CVIPMgr::~CVIPMgr()
{
}

CVIPMgr* CVIPMgr::Instance()
{
	static CVIPMgr vipmgr;
	return &vipmgr;
}

bool CVIPMgr::HandleRecharge(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	U32 nDrawGold = (U32)Packet->readInt(Base::Bit32);

	//更新累计充值，更新经验，特权礼包，更新定时器
	
	PlayerSimpleDataRef pSimPlayer = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);

	U32 nVipLv = pSimPlayer->VipLv;
	U32 nOldVipLv = nVipLv;
	int VipLv = nVipLv + 1;
	VIPData* pData = g_VIPDataMgr->getData(VipLv);
	for (; pData && pData->m_DrawGold <= nDrawGold && VipLv < MAXVIPLEVEL; ++nVipLv, ++VipLv, pData = g_VIPDataMgr->getData(VipLv));
			
	if (SERVER->GetPlayerManager()->SetVipLv(nPlayerId, nVipLv))
	{
		/*********** 加buff *************/
		if (AddVipBuff(nPlayerId, nOldVipLv, nVipLv))
		{
			PLAYERINFOMGR->UpdateToClient(nPlayerId);
		}
	}

	return true;
}

bool CVIPMgr::HandleClientVipGiftRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 VIPGiftId = Packet->readInt(Base::Bit32);
	U32 nError = 0;
 	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (!pAccount)
		return false;

	if (U32 VipLv = GetVipLvByGiftId(pAccount->GetPlayerId(), VIPGiftId))
	{
		bool bGetting = true;
		U32 Flag = (1 << (VipLv - 1));
		TimeSetRef pTime = TIMESETMGR->GetData(pAccount->GetPlayerId(), VIP_TIME);
		if (pTime)
		{
			bGetting = pTime->Flag1 & (1 << (VipLv - 1)) ? false : true;
			Flag = pTime->Flag1 | (1 << (VipLv - 1));
		}
		if (bGetting) 
		{
			if (!ITEMMGR->AddItem(pAccount->GetPlayerId(), VIPGiftId, 1))
			{
				nError = 1;
			}
			else 
			{
				TIMESETMGR->AddTimeSet(pAccount->GetPlayerId(), VIP_TIME, 0, "", Flag);
			}
		}
		else 
		{
			nError = 1;
		}
	}
	else
	{
		nError = 1;
	}

	CMemGuard Buffer(64 MEM_GUARD_PARAM);
	Base::BitStream sendPacket(Buffer.get(), 64);
	stPacketHead* pPktHead = IPacket::BuildPacketHead(sendPacket, "WC_VIP_GIFT_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
	sendPacket.writeInt(nError, Base::Bit16);
	pPktHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	return true;
}

U32 CVIPMgr::GetVipVIMTopLimit(U32 nPlayerId) const
{
	PlayerSimpleDataRef PlayerData = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);
	VIPData* pData = g_VIPDataMgr->getData(PlayerData->VipLv);
	if(pData)
	{
		return pData->m_VIMTopLimit;
	}
	return 0;
}

U32 CVIPMgr::GetVipStrengthTopLimit(U32 nPlayerId) const
{
	PlayerSimpleDataRef PlayerData = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);
	VIPData* pData = g_VIPDataMgr->getData(PlayerData->VipLv);
	if (pData)
	{
		return pData->m_StrengthTopLimit;
	}
	return 0;
}

U32 CVIPMgr::GetVipActivityTopLimit(U32 nPlayerId) const
{
	PlayerSimpleDataRef PlayerData = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);
	VIPData* pData = g_VIPDataMgr->getData(PlayerData->VipLv);
	if (pData)
	{
		return pData->m_ActivityTopLimit;
	}
	return 0;
}

F32 CVIPMgr::GetVipVSAReducePercent(U32 nPlayerId) const
{
	PlayerSimpleDataRef PlayerData = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);
	VIPData* pData = g_VIPDataMgr->getData(PlayerData->VipLv);
	if (pData)
	{
		return pData->m_VSAReducePercent;
	}
	return 0;
}

U32 CVIPMgr::GetVipApplyTopLimit(U32 nPlayerId) const
{
	PlayerSimpleDataRef PlayerData = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);
	VIPData* pData = g_VIPDataMgr->getData(PlayerData->VipLv);
	if (pData)
	{
		return pData->m_ApplyTopLimit;
	}
	return 0;
}

U32 CVIPMgr::GetImposeTopLimit(U32 nPlayerId) const
{
	PlayerSimpleDataRef PlayerData = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);
	VIPData* pData = g_VIPDataMgr->getData(PlayerData->VipLv);
	if (pData)
	{
		return pData->m_ImposeTopLimit;
	}
	return 0;
}

U32 CVIPMgr::GetPriceNumTopLimit(U32 nPlayerId) const
{
	PlayerSimpleDataRef PlayerData = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);
	VIPData* pData = g_VIPDataMgr->getData(PlayerData->VipLv);
	if (pData)
	{
		return pData->m_PrinceNumberTopLimit;
	}
	return 0;
}

U32 CVIPMgr::GetResIncreasePercent(U32 nPlayerId) const
{
	PlayerSimpleDataRef PlayerData = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);
	VIPData* pData = g_VIPDataMgr->getData(PlayerData->VipLv);
	if (pData)
	{
		return pData->m_ImposePlus;
	}
	return 0;
}

bool CVIPMgr::CanJumpBattleAnim(U32 nPlayerId) const
{
	PlayerSimpleDataRef PlayerData = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);
	VIPData* pData = g_VIPDataMgr->getData(PlayerData->VipLv);
	if (pData)
	{
		return pData->m_CanJumpBattleAnim == 1;
	}
	return false;
}

void CVIPMgr::VIPInitAddImpose(U32 nPlayerId)
{
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerId, VIP_INIT_IMPOSE_TIME);
	if (!pTime) {
		TIMESETMGR->AddTimeSet(nPlayerId, VIP_INIT_IMPOSE_TIME, 0, "", 0);

		//增收令上限
		PLAYERINFOMGR->AddImposeTimes(nPlayerId, GetImposeTopLimit(nPlayerId));
	}
}

void __AddDrawGold(U32 nPlayerId, U32 DrawGold) 
{
	SERVER->GetPlayerManager()->AddDrawGold(nPlayerId, DrawGold);
}

void __GetVipGift(U32 nAccountId, U32 VipLevel)
{
	MAKE_WORKQUEUE_PACKET(sendPacket, 128);
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WW_VIP_GIFT_REQUEST", nAccountId);
	sendPacket.writeInt(VipLevel, Base::Bit32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SEND_WORKQUEUE_PACKET(sendPacket, WQ_NORMAL, OWQ_EnterGame);
}

void CVIPMgr::Export(struct lua_State* L)
{
	lua_tinker::def(L, "GetVipGift", &__GetVipGift);
	lua_tinker::def(L, "AddDrawGold", &__AddDrawGold);
}

bool CVIPMgr::AddVipBuff(U32 nPlayerId, U32 OldVipLevel, U32 CurrentVipLevel)
{
	if (OldVipLevel == CurrentVipLevel || CurrentVipLevel > MAXVIPLEVEL)
	{
		return false;
	}

	U8 OldImposeTopLimit, OldApplyTopLimit, OldImposePlus, OldPrinceNumberTopLimit;
	U8 NewImposeTopLimit, NewApplyTopLimit, NewImposePlus, NewPrinceNumberTopLimit;
	VIPData* OldData = g_VIPDataMgr->getData(OldVipLevel);
	VIPData* NewData = g_VIPDataMgr->getData(CurrentVipLevel);

	OldImposeTopLimit = OldData ? OldData->m_ImposeTopLimit : 0;
	//OldApplyTopLimit = OldData ? OldData->m_ApplyTopLimit : 0;
	OldImposePlus = OldData ? OldData->m_ImposePlus : 0;
	OldPrinceNumberTopLimit = OldData ? OldData->m_PrinceNumberTopLimit : 0;
	F32 OldLvReducePercent = OldData ? OldData->m_VSAReducePercent : 0;

	NewImposeTopLimit = NewData ? NewData->m_ImposeTopLimit : 0;
	//NewApplyTopLimit = NewData ? NewData->m_ApplyTopLimit : 0;
	NewImposePlus = NewData ? NewData->m_ImposePlus : 0;
	NewPrinceNumberTopLimit = NewData ? NewData->m_PrinceNumberTopLimit : 0;
	F32 CurrentLvReducePercent = NewData ? NewData->m_VSAReducePercent : 0;

	//皇帝席位上限
	//PRINCEMGR->AddPrinceSlot(nPlayerId, NewPrinceNumberTopLimit - OldPrinceNumberTopLimit);
	//增收令上限
	PLAYERINFOMGR->AddImposeTimes(nPlayerId, NewImposeTopLimit - OldImposeTopLimit);

	//精力时间减少
	TimeSetRef pTime = TIMESETMGR->GetData(nPlayerId, TURN_CARD_TIMER);
	if (pTime) {
		TIMESETMGR->AddTimeSet(nPlayerId, TURN_CARD_TIMER, (pTime->ExpireTime - time(NULL)) * ((100 + CurrentLvReducePercent - OldLvReducePercent) / 100), "", pTime->Flag1);
	}

	//体力时间减少
	pTime = TIMESETMGR->GetData(nPlayerId, CITY_TIMER2);
	if (pTime) {
		TIMESETMGR->AddTimeSet(nPlayerId, CITY_TIMER2, (pTime->ExpireTime - time(NULL)) * ((100 + CurrentLvReducePercent - OldLvReducePercent) / 100), "", pTime->Flag1);
	}

	//活力时间减少
	PRINCEMGR->ReduceTimePercent(nPlayerId, OldLvReducePercent, CurrentLvReducePercent);

	return true;
}

U32 CVIPMgr::GetVipLvByGiftId(U32 nPlayerId, U32 VipGiftId)
{
	PlayerSimpleDataRef pSimPlayer = SERVER->GetPlayerManager()->GetPlayerData(nPlayerId);

	for (int i = 1; i <= pSimPlayer->VipLv; ++i)
	{
		VIPData * Data = g_VIPDataMgr->getData(i);
		if (Data)
		{
			if (Data->m_VIPGiftId == VipGiftId)
				return i;
		}
	}
	return 0;
}
