/*
#include "WorldServer.h"
#include "wintcp/AsyncSocket.h"
#include "wintcp/dtServerSocket.h"
#include "ServerMgr.h"
#include "DBLayer\Data\TblAuction.h"
#include "AuctionManager.h"
#include "MailManager.h"
#include "PlayerMgr.h"
#include "AccountHandler.h"
#include "DBAuction.h"
#include "common/ItemData.h"
#include "ChatHelper.h"
#include "GamePlayLog.h"

AuctionManager::AuctionManager()
{
	registerEvent(ZW_AUCTION_QUERYINFO_REQUEST,		&AuctionManager::HandleQueryAuction);
	registerEvent(ZW_AUCTION_ADD_REQUEST,			&AuctionManager::HandleAddAuction);
	registerEvent(ZW_AUCTION_BUY_REQUEST,			&AuctionManager::HandleBuyAuction);
	registerEvent(ZW_AUCTION_CANCEL_REQUEST,		&AuctionManager::HandleCancelAuction);
}

// ----------------------------------------------------------------------------
// 每分钟定时检查拍卖行数据（检查有效期）
void AuctionManager::Update()
{
	DO_LOCK( Lock::Player );
	DB_CheckUpdateAuction* pDBHandle = new DB_CheckUpdateAuction;
	SERVER->GetAuctionDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 查询拍卖行商品基本信息数据
void AuctionManager::HandleQueryAuction(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Player );
	DB_QueryAuction* pDBHandle = new DB_QueryAuction;
	pDBHandle->lineId = pHead->LineServerId;
	pDBHandle->zoneId = pHead->SrcZoneId;
	pDBHandle->lastQueryTime = pHead->Id;
	SERVER->GetAuctionDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 查询拍卖行商品结果
void AuctionManager::SendQueryAuction(S32 lineId, S32 zoneId, U32 lastQueryTime, AUCTIONLIST& auctionList)
{
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( lineId, zoneId);
	if(!SocketHandle)
	{
		g_Log.WriteWarn("查询拍卖行商品数据找不到网络连接(lineId=%d, zone=%d", lineId, zoneId);
		return;
	}

	if(auctionList.empty())
	{
		//若拍卖行为空,也必须回应消息给Zone
		char buf[120];
		Base::BitStream sendPacket(buf, 120);
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_AUCTION_QUERYINFO_RESPOND, lastQueryTime, SERVICE_ZONESERVER, zoneId);
		sendPacket.writeInt(0, 16);
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
		return;
	}

	size_t count = auctionList.size();
	size_t block = count / MAX_AUCTION_QUERY;
	if(count % MAX_AUCTION_QUERY != 0)
		block++;

	int blocknum;
	stAuctionInfo* pInfo;
	char* buffer = new char[MAX_PLAYER_STRUCT_SIZE];
	Base::BitStream sendPacket(buffer, MAX_PLAYER_STRUCT_SIZE);
	
	for(size_t i = 0; i < block; i++)
	{
		sendPacket.setPosition(0);
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_AUCTION_QUERYINFO_RESPOND, lastQueryTime, SERVICE_ZONESERVER, zoneId);

		if(i == block-1)
			blocknum = count % MAX_AUCTION_QUERY;
		else
			blocknum = MAX_AUCTION_QUERY;

		sendPacket.writeInt(blocknum, 16);
		for(size_t j = i*MAX_AUCTION_QUERY; j < i*MAX_AUCTION_QUERY+blocknum; j++)
		{
			pInfo = auctionList[j];
			sendPacket.writeBits(sizeof(stAuctionInfo)<<3, pInfo);
			if(sendPacket.writeFlag(pInfo->pItem != NULL))
			{
				sendPacket.writeBits(sizeof(stItemInfo)<<3, pInfo->pItem);
			}
		}

		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
	}
	SAFE_DELETE_ARRAY(buffer);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求上架拍卖商品的消息
void AuctionManager::HandleAddAuction(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Player );
	DB_AddAuction* pDBHandle = new DB_AddAuction;
	Packet->readBits(sizeof(stAuctionInfo)<<3, &pDBHandle->info);
	if(Packet->readFlag())
	{
		pDBHandle->info.pItem = new stItemInfo;
		Packet->readBits(sizeof(stItemInfo)<<3, pDBHandle->info.pItem);
	}
	SERVER->GetAuctionDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 数据操作完AddAuction回调处理结果．
void AuctionManager::SendAddAuction(S32 error, stAuctionInfo* pInfo)
{
	DO_LOCK( Lock::Player );
	if(error != 0)
	{
		if(pInfo->pItem)
		{
			//退物品给拍卖的物主
			SERVER->GetMailManager()->sendMail(0, pInfo->Owner, 0,*(pInfo->pItem), "【拍卖行交易】",
				"<t c='0xff9900ff'>    很抱歉，您向拍卖行上架商品失败，系统退还物品，请及时收取</t>",1);
		}
		else
		{
			SERVER->GetMailManager()->sendMail(0, pInfo->Owner, 0, pInfo->ItemId, pInfo->ItemNum, "【拍卖行交易】",
				"<t c='0xff9900ff'>    很抱歉，您向拍卖行上架商品失败，系统退还物品，请及时收取</t>",1);
		}
		//向客户端通知邮件更新
		SERVER->GetPlayerManager()->SendSimplePacket(pInfo->Owner,WORLD_CLIENT_MailNotify );

		//记录游戏逻辑日志:拍卖行物品上架失败
		GameplayLog::GL_AuctionAddFailed(pInfo->Owner, pInfo->ItemId, pInfo->pItem, pInfo->ItemNum, pInfo->FixedPrice);
	}
	else
	{
		//记录游戏逻辑日志:拍卖行物品上架成功
		GameplayLog::GL_AuctionAddOK(pInfo->Owner, pInfo->ItemId, pInfo->pItem, pInfo->ItemNum, pInfo->FixedPrice);
	}

	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(pInfo->Owner);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(pInfo->Owner);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[120];
	Base::BitStream sendPacket( buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_AUCTION_ADD_RESPOND, pInfo->Owner, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求购买拍卖商品的消息
void AuctionManager::HandleBuyAuction(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Player );
	DB_BuyAuction* pDBHandle = new DB_BuyAuction;
	pDBHandle->buyerId	= pHead->Id;
	pDBHandle->id		= Packet->readInt(32);
	pDBHandle->buyPrice	= Packet->readInt(32);
	pDBHandle->buyTime	= Packet->readInt(32);
	SERVER->GetAuctionDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 数据操作完BuyAuction回调处理结果．
void AuctionManager::SendBuyAuction(U32 id, U32 buyerId, S32 buyPrice, S32 error)
{
	DO_LOCK( Lock::Player );
	if(error != 0)
	{
		SERVER->GetMailManager()->sendMail(0, buyerId, buyPrice, 0,0, "【拍卖行交易】",
				"<t c='0xff9900ff'>    很抱歉，您购买拍卖行商品失败，请及时收取金元.",1);
		//向客户端通知邮件更新
		SERVER->GetPlayerManager()->SendSimplePacket(buyerId, WORLD_CLIENT_MailNotify );

		//记录游戏逻辑日志:拍卖行购买商品成功
		GameplayLog::GL_AuctionBuyFailed(buyerId, id, buyPrice);
	}
	else
	{
		//记录游戏逻辑日志:拍卖行购买商品成功
		GameplayLog::GL_AuctionBuyOK(buyerId, id, buyPrice);
	}

	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(buyerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(buyerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket( buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_AUCTION_BUY_RESPOND, buyerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);

	if(error == 0)
		SERVER->GetPlayerManager()->SendSimplePacket(buyerId, WORLD_CLIENT_MailNotify );
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求取消拍卖商品的消息
void AuctionManager::HandleCancelAuction(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Player );
	DB_CancelAuction* pDBHandle = new DB_CancelAuction;
	pDBHandle->playerId = pHead->Id;
	pDBHandle->id = Packet->readInt(32);
	pDBHandle->cancelTime = Packet->readInt(32);
	SERVER->GetAuctionDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 数据操作完CancelAuction回调处理结果．
void AuctionManager::SendCancelAuction(U32 Id, U32 PlayerId, S32 error)
{
	DO_LOCK( Lock::Player );
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket( buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_AUCTION_CANCEL_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);

	//记录游戏逻辑日志: 拍卖行取消寄售成功
	GameplayLog::GL_AuctionCancelOK(PlayerId, Id);
}
*/