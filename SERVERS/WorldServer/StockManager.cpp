/*
#include "StockManager.h"
#include "WorldServer.h"
#include "PlayerMgr.h"
#include "wintcp/dtServerSocket.h"
#include "DBLayer/Data//TBLGoldTrade.h"
#include "BillingManager.h"
#include "MailManager.h"
#include "DB_GoldTrade.h"
#include "GamePlayLog.h"

#ifndef _NTJ_UNITTEST
#include "Common/UnitTest.h"
#endif

GoldTradeManage::GoldTradeManage()
{
	registerEvent( CW_QUERYQUOTATION_REQUEST,	&GoldTradeManage::HandleCWQueryQuotation );
	registerEvent( ZW_COMMISSION_REQUEST,		&GoldTradeManage::HandleZWCommissionRequest );
	registerEvent( CW_CANCELORDER_REQUEST,		&GoldTradeManage::HandleCWCancelOrderRequest );
	registerEvent( CW_QUERYORDER_REQUEST,		&GoldTradeManage::HandleCWQueryOrderRequest );
	
	mNextCloseTime = 0;
	mClosePrice = mCurrentPrice = 0;
}

GoldTradeManage::~GoldTradeManage()
{
	ClearTable();
}

// ----------------------------------------------------------------------------
// World刚启动初始化元宝交易管理
bool GoldTradeManage::Initialize()
{
	TBLGoldTrade tbl_goldtrade(SERVER->GetActorDB());
	return tbl_goldtrade.LoadOrder(mSellTable, mBuyTable) == DBERR_NONE;
}

// ----------------------------------------------------------------------------
// 定时取消超过18小时的挂单
void GoldTradeManage::Update()
{
	DO_LOCK(Lock::Player);
	DB_GoldTrade_BreakOrder* pDBHandle = new DB_GoldTrade_BreakOrder;
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 收盘(关闭所有挂单)
void GoldTradeManage::CloseOrder(U32 currentTime)
{
	DO_LOCK(Lock::Stock);
	DB_GoldTrade_CloseOrder* pDBHandle = new DB_GoldTrade_CloseOrder;
	pDBHandle->currentTime = currentTime;
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 从挂单列表中剔除超时的挂单
void GoldTradeManage::SetBreakOrder(std::vector<int>& buyList, std::vector<int>& sellList)
{
	DO_LOCK(Lock::Player);
	stGoldTrade* pOrder;

	MAPDESC::iterator itBuy;
	for(size_t i = 0; i < buyList.size(); i++)
	{
		for(itBuy = mBuyTable.begin(); itBuy != mBuyTable.end(); ++itBuy)
		{
			pOrder = itBuy->second;
			if(pOrder && pOrder->OrderID == buyList[i])
			{
				delete pOrder;
				mBuyTable.erase(itBuy++);
				break;
			}
		}
	}

	MAPASC::iterator itSell;
	for(size_t i = 0; i < sellList.size(); i++)
	{
		for(itSell = mSellTable.begin(); itSell != mSellTable.end(); ++itSell)
		{
			pOrder = itSell->second;
			if(pOrder && pOrder->OrderID == sellList[i])
			{
				delete pOrder;
				mSellTable.erase(itSell++);
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------------
// 设置收盘后时间和上一次收盘价
void GoldTradeManage::SetClosePrice(int closePrice, U32 nextCloseTime, VECCLOSE& taxList, VECCLOSE& goldList)
{
	DO_LOCK(Lock::Stock);
	mCurrentPrice = 0;
	mClosePrice = closePrice;
	mNextCloseTime = nextCloseTime;

	//需要即时清理在线挂单列表里＂被收盘的挂单＂
	for(size_t i = 0; i < taxList.size(); i++)
	{
		MAPDESC::iterator it = mBuyTable.begin();
		for(; it != mBuyTable.end();++it)
		{
			stGoldTrade* pOrder = it->second;
			if(pOrder && pOrder->OrderID == taxList[i]->OrderID)
			{
				delete pOrder;
				mBuyTable.erase(it++);
				break;
			}
		}
	}

	for(size_t i = 0; i < goldList.size(); i++)
	{
		MAPASC::iterator it = mSellTable.begin();
		for(; it != mSellTable.end();++it)
		{
			stGoldTrade* pOrder = it->second;
			if(pOrder && pOrder->OrderID == goldList[i]->OrderID)
			{
				delete pOrder;
				mSellTable.erase(it++);
				break;
			}
		}
	}
	
	for each(stQuotation::stLatestPrice* it in mLatestPrice)
	{
		if(it) delete it;
	}
	mLatestPrice.clear();
}

// ----------------------------------------------------------------------------
// 回应Zone请求查询行情价
void GoldTradeManage::HandleCWQueryQuotation(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet)
{
	DO_LOCK(Lock::Stock);
	SendWCQueryQuotation(pHead->Id);
}

// ----------------------------------------------------------------------------
// 向客户端回应查询最新行情价
void GoldTradeManage::SendWCQueryQuotation(int PlayerID)
{
	stQuotation Quote;
	Quote.ClosePrice = mClosePrice;
	Quote.CurrentPrice = mCurrentPrice;
	Quote.Pendings = 0;

	int count = 0;
	for each(const std::pair<int, stGoldTrade*>& it in mBuyTable)
	{
		stGoldTrade* pOrder = it.second;
		if(!pOrder)
			continue;

		//统计当前玩家未决的挂单数
		if(pOrder->PlayerID == PlayerID)
			Quote.Pendings++;

		if(Quote.BuyPrice[0].Price == 0)
		{
			Quote.BuyPrice[0].Price = pOrder->Price;
		}

		if(pOrder->Price == Quote.BuyPrice[count].Price)
		{
			Quote.BuyPrice[count].Num += pOrder->LeftNum;
		}
		else
		{
			if(count < stQuotation::MAX_ITEMS - 1)
			{
				count ++;
				Quote.BuyPrice[count].Price = pOrder->Price;
				Quote.BuyPrice[count].Num = pOrder->LeftNum;
			}
		}
	}

	count = 0;
	for each(const std::pair<int, stGoldTrade*>& it in mSellTable)
	{
		stGoldTrade* pOrder = it.second;
		if(!pOrder)
			continue;

		//统计当前玩家未决的挂单数
		if(pOrder->PlayerID == PlayerID)
			Quote.Pendings++;

		if(Quote.SellPrice[0].Price == 0)
			Quote.SellPrice[0].Price = pOrder->Price;

		if(pOrder->Price == Quote.SellPrice[count].Price)
		{
			Quote.SellPrice[count].Num += pOrder->LeftNum;
		}
		else
		{
			if(count < stQuotation::MAX_ITEMS - 1)
			{
				count ++;
				Quote.SellPrice[count].Price = pOrder->Price;
				Quote.SellPrice[count].Num = pOrder->LeftNum;
			}			
		}
	}

	count = 0;
	for each(stQuotation::stLatestPrice* it in mLatestPrice)
	{
		if(!it)
			continue;
		Quote.LatestPrice[count] = *it;
		if(++count >= stQuotation::MAX_ITEMS)
			break;
	}

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerID);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("为查询最新行情价获取不到在线玩家数据!(PlayerID=%d)", PlayerID);
		return;
	}

	char buf[512];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_QUERYQUOTATION_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	sendPacket.writeBits(sizeof(stQuotation)<<3, &Quote);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自ZoneServer委托买入或卖出消息
void GoldTradeManage::HandleZWCommissionRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet)
{
	DO_LOCK(Lock::Stock);
	DB_GoldTrade_CreateOrder* pDBHandle = new DB_GoldTrade_CreateOrder;
	Packet->readBits(sizeof(stGoldTrade)<<3, &pDBHandle->trade);
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 委托挂单成功或失败回应Zone
void GoldTradeManage::SendZoneCommission(int error, int PlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerID);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("挂单操作向Zone回应结果，无法获得玩家在线数据!(PlayerID=%d)", PlayerID);
		return;
	}

	char buf[128];
	Base::BitStream sendPacket( buf, sizeof(buf) );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_COMMISSION_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	sendPacket.writeInt(error, Base::Bit32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket);
}

// ----------------------------------------------------------------------------
// 集中处理委托消息
void GoldTradeManage::CommissionOrder(bool IsOk, stGoldTrade* pTrade)
{
	DO_LOCK(Lock::Stock);

	if(IsOk == false)
	{
		//创建挂单失败，若是委托买入元宝挂单，必须退还金元
		if(pTrade->OrderType == OT_BUY)
		{
			S64 money = pTrade->Price * pTrade->TradeNum * 101/100;
			char szMsg[1024];
			sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您委托买入元宝挂单操作失败，系统退还您的金钱，请及时收取.</t>",
				pTrade->PlayerName);
			SERVER->GetMailManager()->sendMail(0, pTrade->PlayerID, (S32)money, 0, 0, "【元宝挂单】", szMsg, 1);
			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(pTrade->PlayerID,WORLD_CLIENT_MailNotify );

			//向Client通知挂单结果
			SendZoneCommission(GR_REDUCEGOLD, pTrade->PlayerID);
		}
		else
		{
			//向Client通知挂单结果
			SendZoneCommission(GR_FAILED, pTrade->PlayerID);
		}		
		return;
	}

	int NeedNum		= pTrade->TradeNum;			//后手挂单的挂单数量
	int ReduceNum	= 0;						//后手挂单与先手挂单成交的数量
	U32 curtime;
	_time32((__time32_t*)&curtime);

	DB_GoldTrade_CommissionOrder* pDBHandle = new DB_GoldTrade_CommissionOrder;
	pDBHandle->trade = *pTrade;					//后手挂单对象
	stGoldTradeEx* pSource = NULL;				//先手挂单对象

	if(pTrade->OrderType == OT_BUY)		//后手挂单类型为买入元宝
	{
		//从卖出元宝挂单列表中筛选出所有符合后手挂单买入价的先手挂单
		MAPASC::iterator it = mSellTable.begin();
		for(; it != mSellTable.end();)
		{
			if(it->first > pTrade->Price)
				break;

			stGoldTrade* pOrder = it->second;
			if(!pOrder)
			{
				mSellTable.erase(it++);
				continue;
			}

			pSource = new stGoldTradeEx;
			memcpy(pSource, pOrder, sizeof(stGoldTrade));

			if(pOrder->LeftNum > NeedNum)
			{
				ReduceNum = NeedNum;
				pOrder->LeftNum -= NeedNum;
				NeedNum = 0;
				++it;
			}
			else
			{
				ReduceNum = pOrder->LeftNum;
				pOrder->LeftNum = 0;
				NeedNum -= ReduceNum;
				delete pOrder;
				mSellTable.erase(it++);
			}

			pSource->BargainNum	= ReduceNum;
			pDBHandle->source.push_back(pSource);

			//添加最近交易记录
			AddLatestPrice(curtime, pSource->Price, ReduceNum);

			if(NeedNum == 0)
				break;
		}
	}
	else					 //后手挂单类型为卖出元宝
	{
		//从买入元宝挂单列表中筛选出所有符合后手挂单卖出价的先手挂单
		MAPDESC::iterator it = mBuyTable.begin();
		for(; it != mBuyTable.end();)
		{
			if(it->first < pTrade->Price)
				break;

			stGoldTrade* pOrder = it->second;
			if(!pOrder)
			{
				mBuyTable.erase(it++);
				continue;
			}

			pSource = new stGoldTradeEx;
			memcpy(pSource, pOrder, sizeof(stGoldTrade));

			if(pOrder->LeftNum > NeedNum)
			{
				ReduceNum = NeedNum;
				pOrder->LeftNum -= NeedNum;
				NeedNum = 0;
				++it;
			}
			else
			{
				ReduceNum = pOrder->LeftNum;
				pOrder->LeftNum = 0;
				NeedNum -= ReduceNum;
				delete pOrder;
				mBuyTable.erase(it++);
			}

			pSource->BargainNum	= ReduceNum;
			pDBHandle->source.push_back(pSource);

			//添加最近交易记录
			AddLatestPrice(curtime, pSource->Price, ReduceNum);

			if(NeedNum == 0)
				break;
		}
	}	
	
	//判断后手挂单与列表中的所有先手挂单是否有交易
	if(pDBHandle->source.empty())
	{
		//需要删除数据库线程对象
		delete pDBHandle;
		//后手挂单需要直接进列表
		stGoldTrade* pTemp = new stGoldTrade;
		memcpy(pTemp, pTrade, sizeof(stGoldTrade));
		if(pTemp->OrderType == OT_BUY)
		{			
			mBuyTable.insert(MAPDESC::value_type(pTemp->Price, pTemp));
		}
		else
		{
			mSellTable.insert(MAPASC::value_type(pTemp->Price, pTemp));
		}

		//向Client通知挂单结果
		SendZoneCommission(GR_PENDING, pTrade->PlayerID);
		if(pTrade->OrderType == OT_SELL)
		{
			//向Zone的玩家发送剩余元宝
			SendWZLeftGold(pTrade->PlayerID, pTrade->AccountID);
		}
		//向客户端玩家发送最新行情价
		SendWCQueryQuotation(pTrade->PlayerID);
	}
	else
	{
		//只保留最近5笔交易记录,多余的都剔除掉
		if(mLatestPrice.size() > stQuotation::MAX_ITEMS)
		{
			size_t count = mLatestPrice.size() - stQuotation::MAX_ITEMS;
			for(size_t i = 0; i < count; i++)
			{
				stQuotation::stLatestPrice* pPrice = mLatestPrice.front();
				if(pPrice)
					delete pPrice;
				mLatestPrice.pop_front();
			}
		}

		SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
	}
}

// ----------------------------------------------------------------------------
// 元宝挂单数据库操作交易结果
void GoldTradeManage::CommissionOrderResult(bool IsOk, stGoldTrade* pTrade, std::vector<stGoldTradeEx*> sourceList)
{
	DO_LOCK(Lock::Stock);
	if(IsOk == false)
	{
		//数据库操作交易挂单失败，则需要把先手挂单再放回队列
		for(size_t i = 0; i < sourceList.size(); i++)
		{
			stGoldTradeEx* pSource = sourceList[i];
			if(!pSource)
				continue;
			stGoldTrade* pOrder = NULL;
			if(pSource->OrderType == OT_BUY)	//先手挂单类型为买入元宝
			{
				MAPDESC::iterator itB = mBuyTable.lower_bound(pSource->Price);
				MAPDESC::iterator itE = mBuyTable.upper_bound(pSource->Price);
				for(; itB != itE; ++itB)
				{
					pOrder = itB->second;
					if(pOrder && pOrder->OrderID == pSource->OrderID)
					{
						break;
					}
					pOrder = NULL;
				}

				if(pOrder == NULL)
				{
					pOrder = new stGoldTrade;
					memcpy(pOrder, pSource, sizeof(stGoldTrade));
					pOrder->LeftNum = pSource->BargainNum;
					mBuyTable.insert(MAPDESC::value_type(pOrder->Price, pOrder));
				}
				else
				{
					pOrder->LeftNum += pSource->BargainNum;
				}
			}
			else						//先手挂单类型为卖出元宝
			{
				MAPASC::iterator itB = mSellTable.lower_bound(pSource->Price);
				MAPASC::iterator itE = mSellTable.upper_bound(pSource->Price);
				for(; itB != itE; ++itB)
				{
					pOrder = itB->second;
					if(pOrder && pOrder->OrderID == pSource->OrderID)
					{
						break;
					}
					pOrder = NULL;
				}

				if(pOrder == NULL)
				{
					pOrder = new stGoldTrade;
					memcpy(pOrder, pSource, sizeof(stGoldTrade));
					pOrder->LeftNum = pSource->BargainNum;
					mSellTable.insert(MAPASC::value_type(pOrder->Price, pOrder));
				}
				else
				{
					pOrder->LeftNum += pSource->BargainNum;
				}
			}
		}		
	}

	//无论数据库操作交易是成功还是失败，若后手挂单仍有剩余数量，则需要放入队列
	if(pTrade->TradeNum > 0 && pTrade->LeftNum > 0)
	{
		stGoldTrade* pTemp = new stGoldTrade;
		memcpy(pTemp, pTrade, sizeof(stGoldTrade));
		if(pTrade->OrderType == OT_BUY)
		{			
			mBuyTable.insert(MAPDESC::value_type(pTrade->Price, pTemp));
		}
		else
		{
			mSellTable.insert(MAPASC::value_type(pTrade->Price, pTemp));
		}

		if(pTrade->TradeNum == pTrade->LeftNum)
		{
			//向Client通知挂单结果
			SendZoneCommission(GR_PENDING, pTrade->PlayerID);
		}
		else
		{
			//向Client通知挂单结果
			SendZoneCommission(GR_PART, pTrade->PlayerID);
		}
	}
	else
	{
		if(IsOk)
		{
			//向Client通知挂单结果
			SendZoneCommission(GR_FINISH, pTrade->PlayerID);
		}
	}

	if(IsOk)
	{
		//向Zone的玩家发送剩余元宝
		SendWZLeftGold(pTrade->PlayerID, pTrade->AccountID);
		//向客户端通知邮件更新
		SERVER->GetPlayerManager()->SendSimplePacket(pTrade->PlayerID,WORLD_CLIENT_MailNotify );		
	}
	//向客户端玩家发送最新行情价
	SendWCQueryQuotation(pTrade->PlayerID);
}

// ----------------------------------------------------------------------------
// 处理来自ZoneServer玩家撤除挂单请求
void GoldTradeManage::HandleCWCancelOrderRequest(int SocketHandle, stPacketHead *pHead, Base::BitStream *Packet)
{
	DO_LOCK(Lock::Stock);

	int PlayerID = pHead->Id;
	int OrderID = Packet->readInt(32);
	int Price = Packet->readInt(32);	

	stGoldTrade* pOrder = NULL;
	MAPASC::iterator it = mSellTable.lower_bound(Price);
	for(; it != mSellTable.upper_bound(Price); ++it)
	{
		pOrder = it->second;
		if(pOrder && pOrder->PlayerID == PlayerID && pOrder->OrderID == OrderID)
		{
			mSellTable.erase(it);
			break;
		}
		pOrder = NULL;
	}

	if(pOrder == NULL)
	{
		MAPDESC::iterator it1 = mBuyTable.lower_bound(Price);
		for(; it1 != mBuyTable.upper_bound(Price); ++it1)
		{
			pOrder = it1->second;
			if(pOrder && pOrder->PlayerID == PlayerID && pOrder->OrderID == OrderID)
			{
				mBuyTable.erase(it1);
				break;
			}
			pOrder = NULL;
		}
	}

	if(pOrder)
	{
		DB_GoldTrade_CancelOrder* pDBHandle = new DB_GoldTrade_CancelOrder;
		pDBHandle->trade = *pOrder;
		SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
		delete pOrder;
	}
	else
	{
		SendWCCancelOrder(pHead->Id, OrderID, false);
	}
}

// ----------------------------------------------------------------------------
// 处理数据库操作撤消挂单结果
void GoldTradeManage::CancelOrderResult(bool IsOk, stGoldTrade* pOrder)
{
	DO_LOCK(Lock::Stock);
	//回应客户端撤消成功与否
	SendWCCancelOrder(pOrder->PlayerID, pOrder->OrderID, IsOk);
	if(IsOk)
	{
		//向客户端通知邮件更新
		SERVER->GetPlayerManager()->SendSimplePacket(pOrder->PlayerID,WORLD_CLIENT_MailNotify );
		//向客户端玩家发送最新行情价
		SendWCQueryQuotation(pOrder->PlayerID);
	}
	else
	{
		//若失败，则需要放回这挂单到列表中
		stGoldTrade* pTemp = new stGoldTrade;
		memcpy(pTemp, pOrder, sizeof(stGoldTrade));
		if(pTemp->OrderType == OT_BUY)
		{			
			mBuyTable.insert(MAPDESC::value_type(pTemp->Price, pTemp));
		}
		else
		{
			mSellTable.insert(MAPASC::value_type(pTemp->Price, pTemp));
		}
	}
}

// ----------------------------------------------------------------------------
// 向客户端回应撤消委托挂单
void GoldTradeManage::SendWCCancelOrder(int PlayerID, int OrderID, bool IsOK)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerID);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("为回应撤消委托挂单获取不到在线玩家数据!(PlayerID=%d)", PlayerID);
		return;
	}
	char buf[128];
	Base::BitStream sendPacket( buf, sizeof(buf) );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_CANCELORDER_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	sendPacket.writeInt(OrderID, Base::Bit32);
	sendPacket.writeFlag(IsOK);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自查询指定玩家挂单数据的请求
void GoldTradeManage::HandleCWQueryOrderRequest(int SocketHandle,stPacketHead* pHead, Base::BitStream* Packet)
{
	DO_LOCK(Lock::Stock);
	DB_GoldTrade_QueryOrder* pDBHandle = new DB_GoldTrade_QueryOrder;
	pDBHandle->PlayerID = pHead->Id;
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

void GoldTradeManage::SendWCQueryOrder(int PlayerID, bool IsOk, std::vector<stGoldTrade*>& order,
									   std::vector<stGoldTradeDetail*>& detail)
{
	DO_LOCK(Lock::Stock);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerID);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("为回应查询委托挂单获取不到在线玩家数据!(PlayerID=%d)", PlayerID);
		return;
	}

	char buf[MAX_PACKET_SIZE];
	Base::BitStream sendPacket(buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_QUERYORDER_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	if(IsOk)
	{
		sendPacket.writeInt(order.size(), 8);
		for(size_t i = 0; i < order.size(); i++)
		{
			sendPacket.writeBits(sizeof(stGoldTrade)<<3, order[i]);
		}
		sendPacket.writeInt(detail.size(), 16);
		for(size_t i = 0; i < detail.size(); i++)
		{
			sendPacket.writeBits(sizeof(stGoldTradeDetail)<<3, detail[i]);
		}
	}
	else
	{
		sendPacket.writeInt(0, 8);
	}
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket);
}

// ----------------------------------------------------------------------------
// 添加最新交易记录
void GoldTradeManage::AddLatestPrice(U32 tradedate, S32 price, U16 num)
{
	mCurrentPrice = price;
	stQuotation::stLatestPrice* pPrice = new stQuotation::stLatestPrice;
	pPrice->TradeDate = tradedate;
	pPrice->Num = num;
	pPrice->Price = price;
	mLatestPrice.push_back(pPrice);
}

// ----------------------------------------------------------------------------
// 清除委托交易数据表
void GoldTradeManage::ClearTable()
{
	for each(const std::pair<int, stGoldTrade*>& it in mSellTable)
	{
		if(it.second) delete it.second;
	}

	for each(const std::pair<int, stGoldTrade*>& it in mBuyTable)
	{
		if(it.second) delete it.second;
	}

	for each(stQuotation::stLatestPrice* it in mLatestPrice)
	{
		if(it) delete it;
	}
	mSellTable.clear();
	mBuyTable.clear();
	mLatestPrice.clear();
}

// ----------------------------------------------------------------------------
// 向Zone发送剩余元宝数据
void GoldTradeManage::SendWZLeftGold(int PlayerID, int AccountID)
{
	DO_LOCK(Lock::Stock);

	DB_Billing_QueryGold* pDBHandle = new DB_Billing_QueryGold;
	pDBHandle->AccountID	= AccountID;
	pDBHandle->PlayerID		= PlayerID;
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}
*/