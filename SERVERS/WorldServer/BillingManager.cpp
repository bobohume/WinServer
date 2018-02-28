/*#include "WorldServer.h"
#include "wintcp/AsyncSocket.h"
#include "wintcp/dtServerSocket.h"
#include "ServerMgr.h"
#include "BillingManager.h"
#include "MailManager.h"
#include "DB_Billing.h"
#include "DB_GoldTrade.h"
#include "PlayerMgr.h"
#include "AccountHandler.h"
#include "common/ItemData.h"
#include "DB_ZoneLogicEvent.h"
#include "GM/GMRespondDef.h"
#include "GM/GMRecver.h"
#include "GamePlayLog.h"

class CMyLog;
extern CMyLog g_gmLog;

BillingManager::BillingManager()
{
	registerEvent( ZW_QUERYGOODS_REQUEST,		&BillingManager::HandleZWQueryGoods);
	registerEvent( ZW_DRAWGOLD_REQUEST,			&BillingManager::HandleZWDrawGold);
	registerEvent( AW_DRAWGOLD_RESPOND,			&BillingManager::HandleAWDrawGold);
	registerEvent( ZW_QUERYGOLD_REQUEST,		&BillingManager::HandleZWQueryGold);
	registerEvent( ZW_BUYGOODS_REQUEST,			&BillingManager::HandleZWBuyGoods);
	registerEvent( CW_QUERYACCOUNTGOLD_REQUEST,	&BillingManager::HandleCWQueryAccountGold);
	registerEvent( AW_QUERYACCOUNTGOLD_RESPOND,	&BillingManager::HandleAWQueryAccountGold);
	registerEvent( ZW_QUERYTOTALDRAWGOLD_REQUEST,	&BillingManager::HandleZWQueryTotalDrawGold);
	registerEvent( CW_MALL_QUERYBOARD_REQUEST,	&BillingManager::HandleCWQueryBoard);
	registerEvent( ZW_REDUCEGOLD_REQUEST,		&BillingManager::HandleZWReduceGold);
	registerEvent( AW_GMQUERYACCOUNTGOLD_RESPOND,	&BillingManager::HandleAWGMQueryAccountGold);
	registerEvent( AW_GMUPDATEACCOUNTGOLD_RESPOND,	&BillingManager::HandleAWGMUpdateAccountGold);
	mVersion = 0;
}

BillingManager::~BillingManager()
{
	ClearGoods();
	ClearBoardList();
}

// ----------------------------------------------------------------------------
// 检查更新
bool BillingManager::Update()
{
	DB_Billing_CheckUpdate* pDBHandle = new DB_Billing_CheckUpdate;
	pDBHandle->mVersion = mVersion;
	SERVER->GetBillingDBManager()->SendPacket(pDBHandle);

	static U32 lastTick = 0;
	U32 curTick = ::GetTickCount();
	if(lastTick != 0 && lastTick + 300000 > curTick)//每3分钟定时查询一下商铺广播信息
		return true;

	DB_Billing_QueryMsg* pDBHandle1 = new DB_Billing_QueryMsg;
	SERVER->GetBillingDBManager()->SendPacket(pDBHandle1);

	lastTick = curTick;
	return true;
}

// ----------------------------------------------------------------------------
// 改变商城数据版本
void BillingManager::ChangeVersion(int ver, GOODSVEC& goodsList)
{
	DO_LOCK(Lock::Billing);
	ClearGoods();
	LoadGoods(goodsList);
	mVersion = ver;
}

// ----------------------------------------------------------------------------
// 载入商城商品数据
void BillingManager::LoadGoods(GOODSVEC& goodsList)
{
	stGoodsPrice* pGoods = NULL;
	for(size_t i = 0; i < goodsList.size(); i++)
	{
		if(goodsList[i])
		{
			pGoods = new stGoodsPrice;
			memcpy(pGoods, goodsList[i], sizeof(stGoodsPrice));
			mGoodsList.push_back(pGoods);
		}
	}
}

// ----------------------------------------------------------------------------
// 清除商品数据
bool BillingManager::ClearGoods()
{
	for(size_t i = 0; i < mGoodsList.size(); ++i)
	{
		if(mGoodsList[i])
			delete mGoodsList[i];
	}
	mGoodsList.clear();
	return true;
}

// ----------------------------------------------------------------------------
// 清除商城广播消息字符串列表
void BillingManager::ClearBoardList()
{
	for(size_t i = 0; i < mBoardList.size(); i++)
	{
		if(mBoardList[i])
			delete mBoardList[i];
	}
	mBoardList.clear();
}

// ----------------------------------------------------------------------------
// 更新商城广播消息字符串内容
void BillingManager::UpdateBoardList(BOARDVEC& vec)
{
	DO_LOCK(Lock::Billing);
	ClearBoardList();
	stMallBoard* pMsg = NULL;
	for(size_t i = 0; i < vec.size(); i++)
	{
		pMsg = new stMallBoard;
		pMsg->MsgId = vec[i]->MsgId;
		strcpy_s(pMsg->Content, 250, vec[i]->Content);
		pMsg->UpdValue = vec[i]->UpdValue;
		mBoardList.push_back(pMsg);
	}
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求发送商城数据的消息
void BillingManager::HandleZWQueryGoods(int SocketHandle,stPacketHead* pHead, Base::BitStream* Packet)
{
	DO_LOCK(Lock::Billing);
	int Version = pHead->Id;
	if(GetVersion() != Version)
	{
		SendWZQueryGoods(SocketHandle, pHead->SrcZoneId);
	}
}

// ----------------------------------------------------------------------------
// 向Zone回应查询商城商品数据
void BillingManager::SendWZQueryGoods(int SocketHandle, int ZoneID)
{
	const U32 MAX_BLOCK = 180;
	size_t counts = mGoodsList.size() / MAX_BLOCK;
	if(mGoodsList.size() % MAX_BLOCK != 0)
		counts ++;

	char* buf = new char[MAX_PACKET_SIZE];
	Base::BitStream sendPacket(buf, MAX_PACKET_SIZE);

	for(size_t i = 0; i < counts; i++)
	{
		sendPacket.setPosition(0);
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_QUERYGOODS_RESPOND, GetVersion(), SERVICE_ZONESERVER, ZoneID);
		size_t count = MAX_BLOCK;
		if( i == counts - 1)
			count = mGoodsList.size() - i * MAX_BLOCK;

		sendPacket.writeInt(count, Base::Bit16);
		for(size_t j = i * MAX_BLOCK; j < (i * MAX_BLOCK + count); j++)
		{
			sendPacket.writeBits(sizeof(stGoodsPrice)<<3, mGoodsList[j]);
		}
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
	}
	delete[] buf;
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求领取元宝的消息
void BillingManager::HandleZWDrawGold(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Billing);
	int PlayerID	= pHead->Id;
	int AccountID	= Packet->readInt(32);
	int TakeGold	= Packet->readInt(32);

    g_Log.WriteLog("RA ZWDrawGold");

	char buf[128];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WA_DRAWGOLD_REQUEST,AccountID);
	sendPacket.writeInt(PlayerID, Base::Bit32);
	sendPacket.writeInt(TakeGold, Base::Bit32);
	SERVER->GetRemoteAccountSocket()->Send(sendPacket);
}

// ----------------------------------------------------------------------------
// 处理Account回应领取元宝的消息
void BillingManager::HandleAWDrawGold(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{	
	DO_LOCK(Lock::Billing);
	DB_Billing_UpdateGold* pDBHandle = new DB_Billing_UpdateGold;
	pDBHandle->type			= GO_DRAWSAVE;
	pDBHandle->AccountID	= pHead->Id;
	pDBHandle->PlayerID		= Packet->readInt(Base::Bit32);
	pDBHandle->Gold			= Packet->readInt(Base::Bit32);
	pDBHandle->AccountGold	= Packet->readInt(Base::Bit32);
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

//处理来自Zone请求购买商品的消息
void BillingManager::HandleZWBuyGoods( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	DO_LOCK(Lock::Billing);
	stTradeGoods tradegoods;
	int PlayerID = pHead->Id;
	Packet->readBits(sizeof(stTradeGoods) << 3, &tradegoods);
	int FriendID = Packet->readInt(Base::Bit32);
	tradegoods.AreaID = SERVER->currentAreaId;

	//若消费的是元宝
	if(tradegoods.PriceType == 0)
	{
		DB_Billing_UpdateGold* pDBHandle = new DB_Billing_UpdateGold;
		pDBHandle->type			= GO_BUYGOODS;
		pDBHandle->AccountID	= tradegoods.AccountID;
		pDBHandle->PlayerID		= PlayerID;
		pDBHandle->Gold			= 0-tradegoods.Price * tradegoods.PriceNum;
		pDBHandle->FriendID		= FriendID;
		pDBHandle->tradeGoods	= tradegoods;
		SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
	}
	else
	{
		DB_Billing_RecordTrade* pDBHandle = new DB_Billing_RecordTrade;
		pDBHandle->tradeGoods	= tradegoods;
		pDBHandle->LeftGold		= 0;
		pDBHandle->FriendID		= FriendID;
		pDBHandle->Op			= 1;
		SERVER->GetBillingDBManager()->SendPacket(pDBHandle);
	}
}

// ----------------------------------------------------------------------------
// 在World上处理失败，商品以邮件回馈给玩家
void BillingManager::SendMailForPrice(int PlayerID, int FriendID, PRICEID priceId, U32 priceNum)
{
	DO_LOCK(Lock::Billing);
	stGoodsPrice* goods = NULL;
	for(size_t i = 0; i < mGoodsList.size(); i++)
	{
		if(mGoodsList[i] && mGoodsList[i]->PriceID == priceId)
		{
			goods = mGoodsList[i];
			break;
		}
	}

	if(NULL == goods)
	{
		g_Log.WriteFocus("商城购物，商品[PriceID=%I64d]没有找到!", priceId);
		return;
	}

	if(FriendID == 0)
	{
		for(int i = 0; i < goods->ItemCount; i++)
		{
			if(goods->ItemID[i] != 0)
			{
				SERVER->GetMailManager()->sendMailEx(0, PlayerID, 0, goods->ItemID[i], goods->ItemNum[i] * priceNum, "【商城购物】",
					"<t>因您包裹满或网络阻塞等其它问题，商城所购物品邮寄给您!</t>", 1);
			}
		}
	}
	else
	{
		//商品以邮件向好友寄出
		for(int i = 0; i < goods->ItemCount; i++)
		{
			if(goods->ItemID[i] != 0)
			{				
				SERVER->GetMailManager()->sendMailEx(0, FriendID, 0, goods->ItemID[i], goods->ItemNum[i] * priceNum, "【商城购物】",
					"<t>您有好友在商城为您购买了商品,请收取.</t>", 1);
			}
		}
	}
}

// ----------------------------------------------------------------------------
// 处理数据库操作商城购物回调商品更新的消息
void BillingManager::BuyGoodsUpdatePriceCallBack(int LeftGold, int FriendID, int Op, stTradeGoods& tradegoods)
{
	DO_LOCK(Lock::Billing);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(tradegoods.PlayerID);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("玩家商城购物成功，在回应Zone之前无法获取在线信息.(PlayerID=%d)", tradegoods.PlayerID);
		if(Op == 1)
		{
			//商城购物以邮件回馈
			SendMailForPrice(tradegoods.PlayerID, FriendID, tradegoods.PriceID,tradegoods.PriceNum);
		}
		return;
	}

	U32 zoneId = pAccount->GetLastZoneId();
	int zoneSocketId = SERVER->GetServerManager()->GetGateSocket(pAccount->GetLastLineId(),zoneId);
	if(zoneSocketId == 0)
	{
		g_Log.WriteWarn("玩家商城购物成功，在回应Zone之前无法获取目标Zone的连接.(PlayerID=%d)", tradegoods.PlayerID);
		if(Op == 1)
		{
			//商城购物以邮件回馈
			SendMailForPrice(tradegoods.PlayerID, FriendID, tradegoods.PriceID,tradegoods.PriceNum);
		}
		return;
	}

	char buf[256];
	Base::BitStream sendPacket( buf, sizeof(buf) );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_BUYGOODS_RESPOND, tradegoods.PlayerID, SERVICE_ZONESERVER, zoneId);
	sendPacket.writeBits(sizeof(stTradeGoods) << 3, &tradegoods);
	sendPacket.writeInt(LeftGold, Base::Bit32);
	sendPacket.writeInt(Op, Base::Bit8);
	sendPacket.writeInt(FriendID, Base::Bit32);
	if(FriendID != 0)
	{
		char szFirendName[32] = {0};
		PlayerSimpleDataRef friendplayer = SERVER->GetPlayerManager()->GetPlayerData(FriendID);
		if(friendplayer)
			dStrcpy(szFirendName, 32, friendplayer->PlayerName);
		sendPacket.writeString(szFirendName, 32);
	}
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( zoneSocketId, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求查询元宝的消息
void BillingManager::HandleZWQueryGold(int SocketHandle,stPacketHead* pHead, Base::BitStream* Packet)
{
	DO_LOCK(Lock::Billing);

	DB_Billing_QueryGold* pDBHandle = new DB_Billing_QueryGold;
	pDBHandle->AccountID	= Packet->readInt(Base::Bit32);
	pDBHandle->PlayerID		= pHead->Id;
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求查询累计提取元宝的消息
void BillingManager::HandleZWQueryTotalDrawGold(int SocketHandle,stPacketHead* pHead, Base::BitStream* Packet)
{
	DO_LOCK(Lock::Billing);

	DB_Billing_QueryTotalDrawGold* pDBHandle = new DB_Billing_QueryTotalDrawGold;
	pDBHandle->PlayerID		= pHead->Id;
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求扣除元宝的消息
void BillingManager::HandleZWReduceGold(int SocketHandle,stPacketHead* pHead, Base::BitStream* Packet)
{
	DO_LOCK(Lock::Billing);
	DB_Billing_UpdateGold* pDBHandle = new DB_Billing_UpdateGold;
	pDBHandle->AccountID	= Packet->readInt(Base::Bit32);
	pDBHandle->PlayerID		= pHead->Id;
	pDBHandle->type			= Packet->readInt(Base::Bit32);
	pDBHandle->Gold			= 0-Packet->readInt(Base::Bit32);
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 处理数据库操作商城购物回调商品更新的消息
void BillingManager::ReduceGoldCallBack(int PlayerId, int type, int Op)
{
	DO_LOCK(Lock::Billing);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerId);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("玩家扣减元宝成功，但在回应Zone之前无法获取在线信息.(PlayerID=%d)", PlayerId);
		
		DB_ZoneLogicEvent* pDBHandle = new DB_ZoneLogicEvent;
		pDBHandle->m_event.data_type = stEventItem::DATATYPE_REDUCEGOLD;
		pDBHandle->m_event.data_value = type;	//这里放着扣减元宝的操作类型
		pDBHandle->m_event.reserve[0] = Op;		//保留值里放着是否成功
		pDBHandle->m_event.sender = 0;
		pDBHandle->m_event.recver = PlayerId;
		pDBHandle->m_playerId = PlayerId;
		pDBHandle->m_zoneId = 0;
		pDBHandle->m_zoneSocketId = 0;
		SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
		return;
	}

	U32 zoneId = pAccount->GetLastZoneId();
	int zoneSocketId = SERVER->GetServerManager()->GetGateSocket(pAccount->GetLastLineId(),zoneId);
	if(zoneSocketId == 0)
	{
		g_Log.WriteWarn("玩家扣减元宝成功，在回应Zone之前无法获取目标Zone的连接.(PlayerID=%d)", PlayerId);

		DB_ZoneLogicEvent* pDBHandle = new DB_ZoneLogicEvent;
		pDBHandle->m_event.data_type = stEventItem::DATATYPE_REDUCEGOLD;
		pDBHandle->m_event.data_value = type;	//这里放着扣减元宝的操作类型
		pDBHandle->m_event.reserve[0] = Op;		//保留值里放着是否成功
		pDBHandle->m_event.sender = 0;
		pDBHandle->m_event.recver = PlayerId;
		pDBHandle->m_playerId = PlayerId;
		pDBHandle->m_zoneId = zoneId;
		pDBHandle->m_zoneSocketId = 0;
		SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
		return;
	}

	char buf[128];
	Base::BitStream sendPacket( buf, sizeof(buf) );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_REDUCEGOLD_RESPOND, PlayerId, SERVICE_ZONESERVER, zoneId);
	sendPacket.writeInt(type, Base::Bit32);
	sendPacket.writeInt(Op, Base::Bit8);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( zoneSocketId, sendPacket);
}

// ----------------------------------------------------------------------------
// 向Zone回应查询玩家元宝
void BillingManager::SendWZQueryGold(int PlayerID, int LeftGold, int TotalDrawGold, bool IsDraw/*=false*//*)
{
	DO_LOCK(Lock::Billing);
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerID);
	int ZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerID);
	int svrSocket = SERVER->GetServerManager()->GetGateSocket( LineId, ZoneId );

	if(0 == svrSocket)
	{
		g_Log.WriteWarn("查询玩家所在线所在地图失败!(玩家查询元宝数)");
		return;
	}

	char buf[128];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_QUERYGOLD_RESPOND, PlayerID, SERVICE_ZONESERVER, ZoneId );
	sendPacket.writeInt(LeftGold, Base::Bit32);
	//Note:新增发送累计提取元宝数
	sendPacket.writeInt(TotalDrawGold, Base::Bit32);
	sendPacket.writeFlag(IsDraw);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send(svrSocket, sendPacket );
}

// ----------------------------------------------------------------------------
// 向Zone回应查询累计提取元宝
void BillingManager::SendWZQueryTotalDrawGold(int PlayerID, int TotalDrawGolds)
{
	DO_LOCK(Lock::Billing);
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerID);
	int ZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerID);
	int svrSocket = SERVER->GetServerManager()->GetGateSocket( LineId, ZoneId );

	if(0 == svrSocket)
	{
		g_Log.WriteWarn("查询玩家所在线所在地图失败!(累计提取元宝)");
		return;
	}

	char buf[128];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_QUERYTOTALDRAWGOLD_RESPOND, PlayerID, SERVICE_ZONESERVER, ZoneId );
	sendPacket.writeInt(TotalDrawGolds, Base::Bit32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send(svrSocket, sendPacket );
}

// ----------------------------------------------------------------------------
// 处理来自Client请求查询帐号元宝的消息
void BillingManager::HandleCWQueryAccountGold(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Billing);
	int PlayerID	= pHead->Id;
	int AccountID	= pHead->DestZoneId;
    
    g_Log.WriteLog("RA QueryAccountGold");

	char buf[128];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WA_QUERYACCOUNTGOLD_REQUEST,AccountID);
	sendPacket.writeInt(PlayerID, Base::Bit32);
	SERVER->GetRemoteAccountSocket()->Send(sendPacket);
}

// ----------------------------------------------------------------------------
// 处理Account回应查询帐号元宝的消息
void BillingManager::HandleAWQueryAccountGold(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{	
	DO_LOCK(Lock::Billing);
	int AccountID	= pHead->Id;
	int PlayerID	= Packet->readInt(Base::Bit32);
	int AccountGold	= Packet->readInt(Base::Bit32);
	SendWCQueryAccountGold(PlayerID, AccountGold);
}	
// ----------------------------------------------------------------------------
// 处理来自GM工具的请求Account查询帐号元宝的消息
void BillingManager::HandleWAGMQueryAccountGold(int sid,int AccountID,int GMAccountID)
{
	DO_LOCK(Lock::Billing);

	g_Log.WriteLog("WR GMQueryAccountGold");

	char buf[128];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WA_GMQUERYACCOUNTGOLD_REQUEST,AccountID,SERVICE_REMOTESERVER);
	sendPacket.writeInt(sid, Base::Bit32);
	sendPacket.writeInt(GMAccountID, Base::Bit32);
	SERVER->GetRemoteAccountSocket()->Send(sendPacket);
}

// ----------------------------------------------------------------------------
// 处理Account回应GM工具的查询帐号元宝的消息
void BillingManager::HandleAWGMQueryAccountGold(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{	
	DO_LOCK(Lock::Billing);
	int AccountID	= pHead->Id;
	int sid	= Packet->readInt(Base::Bit32);
	int GMAccountID	= Packet->readInt(Base::Bit32);
	int AccountGold	= Packet->readInt(Base::Bit32);

	CGMAccountGoldOpResp resp;
	resp.Gold = AccountGold;

	//日志记录：帐号元宝查询
	g_gmLog.WriteLog("Gm帐号:%d 操作类型:GM_AccountGoldOp 操作对象ID:%d  操作内容：账号元宝查询 ",AccountID,GMAccountID);

	CGMRecver::Instance()->PushResp(resp);
	CGMRecver::Instance()->SendResps("GM_AccountGoldOp",0,sid);//sid
}	

// ----------------------------------------------------------------------------
// 处理来自GM工具的请求Account修改帐号元宝的消息
// void BillingManager::HandleWAGMUpdateAccountGold(int sid,int AccountID,int GMAccountID,int gold)
// {
// 	DO_LOCK(Lock::Billing);
// 
// 	g_Log.WriteLog("WR GMQueryAccountGold");
// 
// 	char buf[128];
// 	Base::BitStream sendPacket(buf,sizeof(buf));
// 	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WA_GMUPDATEACCOUNTGOLD_REQUEST,AccountID,SERVICE_REMOTESERVER);
// 	sendPacket.writeInt(sid, Base::Bit32);
// 	sendPacket.writeInt(gold, Base::Bit32);
// 	sendPacket.writeInt(GMAccountID, Base::Bit32);
// 	SERVER->GetRemoteAccountSocket()->Send(sendPacket);
// }

// ----------------------------------------------------------------------------
// 处理Account回应GM工具的修改帐号元宝的消息
void BillingManager::HandleAWGMUpdateAccountGold(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{	
	DO_LOCK(Lock::Billing);
	int AccountID	= pHead->Id;
	int sid	= Packet->readInt(Base::Bit32);
	int GMAccountID	= Packet->readInt(Base::Bit32);
	int AccountGold	= Packet->readInt(Base::Bit32);

	CGMAccountGoldOpResp resp;
	resp.Gold = AccountGold;

	//日志记录：帐号元宝查询
	GameplayLog::GL_GMUpdateAccountGold(GMAccountID, GMAccountID);
	g_gmLog.WriteLog("Gm帐号:%d 操作类型:GM_AccountGoldOp 操作对象ID:%d  操作内容：账号元宝修正 ",AccountID,GMAccountID);

	CGMRecver::Instance()->PushResp(resp);
	CGMRecver::Instance()->SendResps("GM_AccountGoldOp",0,sid);//sid
}	

// ----------------------------------------------------------------------------
// 向Client发送查询帐号元宝的消息
void BillingManager::SendWCQueryAccountGold(int PlayerID, int AccountGold)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerID);
	if(NULL == pAccount)
		return;

	char buf[128];
	Base::BitStream sendPacket(buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_QUERYACCOUNTGOLD_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	sendPacket.writeInt(AccountGold, Base::Bit32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket );
}

// ----------------------------------------------------------------------------
// 处理来自Client请求查询商城广播消息
void BillingManager::HandleCWQueryBoard(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::Billing);
	int PlayerId	= pHead->Id;
	size_t counts	= Packet->readInt(32);
	std::vector<std::pair<int,int> > SrcVec;
	BOARDVEC TagVec;
	int MsgId, UpdValue;
	if(counts >= 64)
	{
		g_Log.WriteWarn("客户端上传一个非法的商城广播消息总数数值!(Player=%d)", PlayerId);
		return;
	}

	for(size_t i = 0; i < counts; i++)
	{
		MsgId = Packet->readInt(32);
		UpdValue = Packet->readInt(32);
		SrcVec.push_back(std::make_pair(MsgId, UpdValue));
	}

	bool bFind;
	for(size_t i = 0; i < mBoardList.size(); i++)
	{
		bFind = false;
		for(size_t j = 0; j < SrcVec.size(); j++)
		{
			if(SrcVec[j].first == mBoardList[i]->MsgId)
			{
				if(SrcVec[j].second == mBoardList[i]->UpdValue)
					bFind = true;
				break;
			}
		}

		if(bFind == false)
		{
			stMallBoard* pMsg = new stMallBoard;
			memcpy(pMsg, mBoardList[i], sizeof(stMallBoard));
			TagVec.push_back(pMsg);
		}
	}

	if(TagVec.empty())
		return;

	counts = TagVec.size();
	if(counts >= 24)
		counts = 24;//最多嵌制到24条，目前最多只有2条而已

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerId);
	if(pAccount)
	{
		char buf[8192];
		Base::BitStream sendPacket(buf,sizeof(buf));
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_MALL_QUERYBOARD_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
		sendPacket.writeInt(counts, 32);
		for(size_t i = 0; i < counts; i++)
		{
			sendPacket.writeBits(sizeof(stMallBoard)<<3, TagVec[i]);
		}
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket);
	}

	for(size_t i = 0; i < TagVec.size(); i++)
	{
		if(TagVec[i])
			delete TagVec[i];
	}
}*/
