#include "../../stdafx.h"
#include "GateManager.h"
#include "../../AccountHandler.h"
#include "../../WorldServer.h"
#include "../../PlayerMgr.h"
#include "../../data/InvestiGate.h"
#include "../platform/platform.h"
#include "../Script/lua_tinker.h"
#include "../Item/DropManager.h"
#include "Common/MemGuard.h"
#include "WINTCP/dtServerSocket.h"
#include "../Script/lua_tinker.h"

CGateManager::CGateManager()
{
	registerEvent(CW_VERIFY_PLAYER_FAKE_REQUEST,		&CGateManager::HandlePlayerVerifyFakeRequest);
	registerEvent(CW_PLAYER_GOTONEXTGATE_REQUEST,		&CGateManager::HandlePlayerGoToNextGateRequest);
	registerEvent(CW_PLAYER_GOTONEWGATE_REQUEST,		&CGateManager::HandlePlayerGoToNewTypeGateRequest);
	registerEvent(CW_PLAYER_SCENEINITED_REQUEST,		&CGateManager::HandlePlayerSceneInitedRequest);
}

CGateManager::~CGateManager()
{

}

bool CGateManager::verifyPlayerFakt(AccountRef pAccount, U32 LocalLoginTime)
{
	if(!pAccount)
		return false;

	PlayerDataRef pPlayer = pAccount->GetPlayerData();
	if(!pPlayer)
		return  false;

	U32 nGateId = pPlayer->MainData.PartData.GateId;
	bool bNewScene = false;

	//玩家是否是新手
	if(!pPlayer->BaseData.PlayerFlag & PLAYER_FLAG_NEW_PLAYER)
	{
		//修改新手标志
		pAccount->setGateId(CInvestiGateDataRes::InvestiGate_BEGIN_ID);
		pAccount->setPlayerFlag(PLAYER_FLAG_NEW_PLAYER);
		bNewScene = true;

		lua_tinker::call<void>(L, "AcceptAllMission");
	}
	else
	{
		//比较本地登录时间，判断是否作弊
		if(LocalLoginTime != pPlayer->BaseData.LastLoginTime)
		{
			S32 iGateId = -1;
			CInvestiGateData* pGateData = g_InvestiGateDataMgr->getInvestiGateData(nGateId);
			if(pGateData)
			{
				iGateId = g_InvestiGateDataMgr->getNextGateId(nGateId, pGateData->m_Layer);
			}
				
			//初始地图
			if(iGateId == -1)
				pAccount->setGateId(CInvestiGateDataRes::InvestiGate_BRITH_ID);
			else 
				pAccount->setGateId(iGateId);

			bNewScene = true;
		}
		else
		{
			CInvestiGateData* pGateData = g_InvestiGateDataMgr->getInvestiGateData(nGateId);
			if(!pGateData)
			{
				pAccount->setGateId(CInvestiGateDataRes::InvestiGate_BRITH_ID);
				bNewScene = true;
			}
		}
	}

	if(!bNewScene)
	{
		//上次创建地图崩溃
		if(pPlayer->BaseData.PlayerFlag & PLAYER_FLAG_SCENE_INIT)
			bNewScene = true;
	}

	//保存登录时间到数据库
	pPlayer->BaseData.LastLoginTime = _time32(NULL);
	pAccount->PlayerBaseSaveToDB(&pPlayer->BaseData, PLAYER_BASE_LASTLOGINTIME);

	{
		CMemGuard pBuffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(pBuffer.get(), 64);
		stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, WC_VERIFY_PLAYER_FAKE_RESPONSE, pAccount->GetAccountId(), SERVICE_CLIENT, pPlayer->BaseData.LastLoginTime, pPlayer->MainData.PartData.GateId, bNewScene);
		pSendHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

	return true;
}

bool CGateManager::gotoNextGate(AccountRef pAccount)
 {
	if(!pAccount)
		return false;

	PlayerDataRef pPlayer = pAccount->GetPlayerData();
	if(!pPlayer)
		return  false;

	U32 nGateId = pPlayer->MainData.PartData.GateId;

	if(nGateId == CInvestiGateDataRes::InvestiGate_BRITH_ID)
		return false;

	{
		S32 iGateId = -1;
		CInvestiGateData* pGateData = g_InvestiGateDataMgr->getInvestiGateData(nGateId);
		if(pGateData)
		{
			iGateId = g_InvestiGateDataMgr->getNextGateId(nGateId, pGateData->m_Layer);
		}

		//初始地图
		if(iGateId == -1)
			pAccount->setGateId(CInvestiGateDataRes::InvestiGate_BRITH_ID);
		else 
			pAccount->setGateId(iGateId);

		nGateId = pPlayer->MainData.PartData.GateId;
	}

	//过场景结算
	{

	}

	{
		CMemGuard pBuffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(pBuffer.get(), 64);
		stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, WC_PLAYER_GOTONEXTGATE_RESPONSE, pAccount->GetAccountId(), SERVICE_CLIENT, nGateId);
		pSendHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

	return true;
}

bool CGateManager::gotoNewTypeGate(AccountRef pAccount, S32 nGateType)
{
	if(!pAccount)
		return false;

	PlayerDataRef pPlayer = pAccount->GetPlayerData();
	if(!pPlayer)
		return  false;

	U32 nGateId = pPlayer->MainData.PartData.GateId;
	//不在切换场景地图
	if(nGateId != CInvestiGateDataRes::InvestiGate_BRITH_ID)
		return false;

	if(nGateType < GATE_TYPE_BEGIN || nGateType > GATE_TYPE_END)
		return false;

	nGateId = 5000001 + nGateType * 1000;
	CInvestiGateData* pGateData = g_InvestiGateDataMgr->getInvestiGateData(nGateId);
	if(!pGateData)
		return false;

	//前置条件
	{

	}

	pAccount->setGateId(nGateId);

	{
		CMemGuard pBuffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(pBuffer.get(), 64);
		stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, WC_PLAYER_GOTONEXTGATE_RESPONSE, pAccount->GetAccountId(), SERVICE_CLIENT, nGateId);
		pSendHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

bool CGateManager::sceneInitFinsh(AccountRef pAccount, Base::BitStream *Packet)
{
	PlayerDataRef pPlayer = pAccount->GetPlayerData();
	if(!pPlayer)
		return  false;

	U8  nLayerId		= Packet->readInt(Base::Bit8);
	U32 nGateId			= pPlayer->MainData.PartData.GateId;
	U32 nCurNeedItemNum[CInvestiGateData::MAX_INVESTIGATE_GOODS_NUM] = {0,0,0};
	U32 nDropEventId    = 0;
	CInvestiGateData* pGateData = g_InvestiGateDataMgr->getInvestiGateData(nGateId);
	if(!pGateData)
		return false;

	if(nLayerId == 5)
	{
		U32 nItemSize = Packet->readInt(Base::Bit32);
		if(nItemSize > 100)
			return false;

		for(int i = 0; i < nItemSize; ++i)
		{
			if(Packet->readFlag())
			{
				U32 nItemId = Packet->readInt(Base::Bit32);
				U32 nRow = Packet->readInt(Base::Bit32);
				U32 nCol = Packet->readInt(Base::Bit32);
				std::string UID;
				en_DropType enDropType;
				if(nItemId == 0)
				{
					nDropEventId = 0;
					for(int i = 0; i < CInvestiGateData::MAX_INVESTIGATE_GOODS_NUM; ++i)
					{
						if(nCurNeedItemNum[i] <= pGateData->m_GoodsNum[i])
						{
							nDropEventId = pGateData->m_GoodsGroup[i];
							nCurNeedItemNum[i]++;
							break;
						}
					}

					if(nDropEventId == 0)
						continue;

					if(!DROPMGR->doDropItem(nDropEventId, pAccount->GetPlayerId(), nItemId, UID))
						continue;

					enDropType = DROP_TYPE_AUTODROP;
				}
				else
				{
					if(!DROPMGR->dropMapManCreate(nItemId, pAccount->GetPlayerId(), UID))
						continue;

					enDropType = DROP_TYPE_MANDROP;
				}

				if(nItemId > 0)
				{
					CMemGuard Buffer(128 MEM_GUARD_PARAM);
					Base::BitStream SendPacket(Buffer.get(), 128);
					stPacketHead* pSendHead = IPacket::BuildPacketHead(SendPacket, WC_DROP_ITME_RESPONSE, pAccount->GetAccountId(), SERVICE_CLIENT, enDropType);
					SendPacket.writeString(UID.c_str(), MAX_UUID_LENGTH);
					SendPacket.writeInt(nItemId, Base::Bit32);
					SendPacket.writeInt(nRow, Base::Bit32);
					SendPacket.writeInt(nCol, Base::Bit32);

					pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
					SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), SendPacket);
				}
			}
		}
	}

	//清楚场景创建标志
	pAccount->clearePlayerFlag(PLAYER_FLAG_SCENE_INIT);

	{
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream SendPacket(Buffer.get(), 64);
		stPacketHead* pSendHead = IPacket::BuildPacketHead(SendPacket, WC_PLAYER_SCENEINITED_RESPONSE, pAccount->GetAccountId(), SERVICE_CLIENT);
		pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), SendPacket);
	}
}

void CGateManager::HandlePlayerVerifyFakeRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	U32 LocalLoginTime = pHead->DestZoneId;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	verifyPlayerFakt(pAccount, LocalLoginTime);
}

void CGateManager::HandlePlayerGoToNextGateRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	gotoNextGate(pAccount);
}

void CGateManager::HandlePlayerGoToNewTypeGateRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	S32 iGateType = pHead->DestZoneId;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	gotoNewTypeGate(pAccount, iGateType);
}

void CGateManager::HandlePlayerSceneInitedRequest(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Player);
	U32 nPlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(!pAccount)
		return;

	sceneInitFinsh(pAccount, Packet);
}
