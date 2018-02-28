/*
#include "WorldServer.h"
#include "wintcp/AsyncSocket.h"
#include "wintcp/dtServerSocket.h"
#include "ServerMgr.h"
#include "GoldExchangeManager.h"
#include "MailManager.h"
#include "DB_GoldExchange.h"
#include "PlayerMgr.h"
#include "AccountHandler.h"
#include "GamePlayLog.h"
#include "DB_ZoneLogicEvent.h"

GoldExChangeManager::GoldExChangeManager()
{
	registerEvent( CW_QUERYGOLDCONFIG_REQUEST,		&GoldExChangeManager::HandleCWQueryGoldConfig);
	registerEvent( AW_QUERYMONTHPAYS_RESPOND,		&GoldExChangeManager::HandleAWQueryMonthPays);
	registerEvent( CW_QUERYGOLDEXCHANGE_REQUEST,	&GoldExChangeManager::HandleCWQueryGoldExchange);
	registerEvent( CW_DRAWGOLDEXCHANGE_REQUEST,		&GoldExChangeManager::HandleCWDrawGoldExchange);
	registerEvent( CW_QUERYSRCONFIG_REQUEST,		&GoldExChangeManager::HandleCWQuerySRConfig);
	registerEvent( ZW_QUERYSRBONUS_REQUST,			&GoldExChangeManager::HandleZWQuerySRBonus);
	registerEvent( CW_DRAWSRBONUS_REQUEST,			&GoldExChangeManager::HandleCWDrawSRBonus);
	Version = ::GetTickCount();
}

// ----------------------------------------------------------------------------
// 检查更新
void GoldExChangeManager::Update()
{
	DB_GoldConfig_Query* pDBHandle = new DB_GoldConfig_Query;
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 更新元宝兑换配置数据
void GoldExChangeManager::UpdateConfig(stGoldConfig& _config)
{
	DO_LOCK(Lock::GoldExchage);
	memcpy(&config, &_config, sizeof(stGoldConfig));
	//更新元宝兑换配置版本值
	Version = ::GetTickCount();
}

// ----------------------------------------------------------------------------
// 处理来自Client请求查询元宝兑换配置数据的消息
void GoldExChangeManager::HandleCWQueryGoldConfig(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::GoldExchage);
	int PlayerID = pHead->Id;
	U32 ClientVersion = Packet->readInt(Base::Bit32);
	int AccountID = Packet->readInt(Base::Bit32);

	//根据版本来确定是否要更新兑换配置
	if(Version !=  ClientVersion)
	{
		AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerID);
		if(NULL == pAccount)
		{
			g_Log.WriteWarn("未能查询到玩家[%d]在线数据(HandleCWQueryGoldConfig)", PlayerID);
			return;
		}

		char buf[512];
		Base::BitStream sendPacket(buf,sizeof(buf));
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_QUERYGOLDCONFIG_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
		sendPacket.writeInt(Version, Base::Bit32);
		sendPacket.writeBits(sizeof(stGoldConfig)<<3, &config);
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket );
	}

	//向Remote查询充值次数
	SendWAQueryMonthPays(AccountID, PlayerID, "", false);
}

// ----------------------------------------------------------------------------
// 向Remote发送查询充值次数
void GoldExChangeManager::SendWAQueryMonthPays(int AccountId, int PlayerId, const char* PlayerName, bool isDraw)
{
	DO_LOCK(Lock::GoldExchage);

    g_Log.WriteLog("RA QueryMonthPays");

	char buf[128];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WA_QUERYMONTHPAYS_REQUEST,AccountId);
	sendPacket.writeFlag(isDraw);
	sendPacket.writeInt(PlayerId, Base::Bit32);
	sendPacket.writeString(PlayerName, 32);
	SERVER->GetRemoteAccountSocket()->Send(sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Remote回应查询角色当月充值总次数的消息
void GoldExChangeManager::HandleAWQueryMonthPays(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	bool isDraw = Packet->readFlag();
	int AccountId = pHead->Id;
	int PlayerID = Packet->readInt(Base::Bit32);
	int AccountGold = Packet->readInt(Base::Bit32);
	int MonthPays = Packet->readInt(Base::Bit32);
	char szPlayerName[32];
	Packet->readString(szPlayerName, 32);

	//判断是否即时兑换"充值礼包"
	if(isDraw)
	{
		//检查本月充值次数是否满足条件
		if(MonthPays >= config.Condition[GEF_CR])
		{
			DB_GoldExchange_Draw* pDBHandle = new DB_GoldExchange_Draw;
			pDBHandle->PlayerID = PlayerID;
			pDBHandle->AccountID = AccountId;
			pDBHandle->type = GEF_CR;
			Packet->readString(szPlayerName, 32);
			SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
		}
		else
		{
			//不可即时兑换"充值礼包"
			SendWCDrawGoldExchange(PlayerID, GEF_CR, false, 0);
		}		
	}
	else
	{
		DB_GoldExchange_Query* pDBHandle = new DB_GoldExchange_Query;
		pDBHandle->exchange.AccountID = AccountId;
		pDBHandle->exchange.PlayerID = PlayerID;
		pDBHandle->exchange.AccountGold = AccountGold;
		pDBHandle->exchange.MonthPays = MonthPays;
		SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
	}
}

// ----------------------------------------------------------------------------
// 向Client回应查询累计提取元宝数
void GoldExChangeManager::SendWCQueryMonthGolds(U32 PlayerID, S32 MonthGolds)
{
	DO_LOCK(Lock::GoldExchage);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerID);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("未能查询到玩家[%d]在线数据(SendWCQueryMonthGolds)", PlayerID);
		return;
	}

	char buf[128];
	Base::BitStream sendPacket( buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_QUERYMONTHGOLDS_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	sendPacket.writeInt(MonthGolds, Base::Bit32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket );
}

// ----------------------------------------------------------------------------
// 处理来自Client请求查询当月兑换元宝数据的消息
void GoldExChangeManager::HandleCWQueryGoldExchange(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::GoldExchage);
	int PlayerID = pHead->Id;
	int AccountID = Packet->readInt(Base::Bit32);
	//向Remote查询充值次数
	SendWAQueryMonthPays(AccountID, PlayerID, "", false);
}

// ----------------------------------------------------------------------------
// 向Client回应查询玩家当月兑换元宝数据
void GoldExChangeManager::SendWCQueryGoldExchange(stGoldExchange* exchange)
{
	DO_LOCK(Lock::GoldExchage);

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(exchange->PlayerID);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("未能查询到玩家[%d]在线数据(SendWCQueryGoldExchange)", exchange->PlayerID);
		return;
	}

	char buf[512];
	Base::BitStream sendPacket( buf, sizeof(buf) );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_QUERYGOLDEXCHANGE_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	sendPacket.writeBits(sizeof(stGoldExchange)<<3, exchange);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket );
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求领取元宝兑换的消息
void GoldExChangeManager::HandleCWDrawGoldExchange(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::GoldExchage);
	int PlayerID = pHead->Id;
	int AccountID = Packet->readInt(Base::Bit32);
	int type = Packet->readInt(Base::Bit8);
	char szPlayerName[MAX_NAME_LENGTH];
	Packet->readString(szPlayerName, 32); 

	//判断是否"充值返利礼包"兑换,需要特别处理
	if(type == GEF_CR)
	{
		//向Remote查询充值次数,返回后以确认是否能即时领取
		SendWAQueryMonthPays(AccountID, PlayerID, szPlayerName, true);
	}
	else if (type > GEF_CR && type < GEF_MAX)
	{
		DB_GoldExchange_Draw* pDBHandle = new DB_GoldExchange_Draw;
		pDBHandle->PlayerID = PlayerID;
		pDBHandle->AccountID = AccountID;
		pDBHandle->type = type;
		dStrcpy(pDBHandle->PlayerName, 32, szPlayerName);
		SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
	}
	else
	{
		g_Log.WriteWarn("收到一个玩家无效的数据包(HandleCWDrawGoldExchange)");
	}
}

// ----------------------------------------------------------------------------
// 向Zone回应元宝兑换领取结果
void GoldExChangeManager::SendWCDrawGoldExchange(int PlayerID, int type, int isOK, int Exchanges)
{
	DO_LOCK(Lock::GoldExchage);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerID);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("未能查询到玩家[%d]在线数据(SendWCDrawGoldExchange)", PlayerID);
		return;
	}

	char buf[128];
	Base::BitStream sendPacket( buf, sizeof(buf) );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_DRAWGOLDEXCHANGE_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	sendPacket.writeInt(isOK, 8);
	sendPacket.writeInt(type, 8);
	sendPacket.writeInt(Exchanges, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket );

	if(isOK)
	{
		//日志记录:玩家元宝兑换
		GameplayLog::GL_GoldExchange(PlayerID, type, config.Change[type], config.ItemID[type]);

		//向客户端通知邮件更新
		SERVER->GetPlayerManager()->SendSimplePacket(PlayerID,WORLD_CLIENT_MailNotify );
	}
}

// ----------------------------------------------------------------------------
// 处理来自Client请求查询分期返利配置数据的消息
void GoldExChangeManager::HandleCWQuerySRConfig(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::GoldExchage);

	DB_StagingRebate_QueryConfig* pDBHandle = new DB_StagingRebate_QueryConfig;
	pDBHandle->PlayerID = pHead->Id;
	pDBHandle->AccountID = Packet->readInt(32);
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 处理来自Client请求查询分期返利奖励数据的消息
void GoldExChangeManager::HandleZWQuerySRBonus(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::GoldExchage);

	DB_StagingRebate_QueryBonus* pDBHandle = new DB_StagingRebate_QueryBonus;
	pDBHandle->PlayerID = pHead->Id;
	pDBHandle->BonusData.SRId = Packet->readInt(32);
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 处理来自Client请求领取分期返利奖励的消息
void GoldExChangeManager::HandleCWDrawSRBonus(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK(Lock::GoldExchage);

	DB_StagingRebate_DrawBonus* pDBHandle = new DB_StagingRebate_DrawBonus;
	pDBHandle->PlayerID = pHead->Id;
	pDBHandle->AccountID = Packet->readInt(32);
	pDBHandle->SRId = Packet->readInt(32);
	pDBHandle->PerMonth = Packet->readInt(8);
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);
}

// ----------------------------------------------------------------------------
// 向Client回应查询分期返利配置数据
void GoldExChangeManager::SendWCQuerySRConfig(U32 PlayerId,
						std::vector<stSR_Period*>& ActivityList,
						std::vector<stSR_Bonus*>& ConfigList,
						std::vector<stSR_History*>& HistoryList)
{
	DO_LOCK(Lock::GoldExchage);
	
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerId);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("未能查询到玩家[%d]在线数据(SendWCQuerySRConfig)", PlayerId);
		return;
	}

	char* buf = new char[MAX_PACKET_SIZE];
	Base::BitStream sendPacket( buf, MAX_PACKET_SIZE);
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_QUERYSRCONFIG_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	//分期返利活动期限配置
	if(sendPacket.writeFlag(!ActivityList.empty()))
	{
		sendPacket.writeInt(ActivityList.size(), 8);
		for(size_t i = 0; i < ActivityList.size(); i++)
		{
			sendPacket.writeBits(sizeof(stSR_Period)<<3, ActivityList[i]);
		}
	}
	//分期返利活动奖励配置
	if(sendPacket.writeFlag(!ConfigList.empty()))
	{
		sendPacket.writeInt(ConfigList.size(), 8);
		for(size_t i = 0; i < ConfigList.size(); i++)
		{
			sendPacket.writeBits(sizeof(stSR_Bonus)<<3, ConfigList[i]);
		}
	}
	//分期返利活动玩家奖励数据
	if(sendPacket.writeFlag(!HistoryList.empty()))
	{
		sendPacket.writeInt(HistoryList.size(), 16);
		for(size_t i = 0; i < HistoryList.size(); i++)
		{
			sendPacket.writeBits(sizeof(stSR_History)<<3, HistoryList[i]);
		}
	}
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket );

	SAFE_DELETE_ARRAY(buf);
}

// ----------------------------------------------------------------------------
// 向Client发送分期返利奖励配置查询结果
void GoldExChangeManager::SendWCQuerySRBonus(U32 PlayerId, stSR_History& BonusData)
{
	DO_LOCK(Lock::GoldExchage);

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerId);
	if(NULL == pAccount)
	{
		g_Log.WriteWarn("未能查询到玩家[%d]在线数据(SendWCQuerySRBonus)", PlayerId);
		return;
	}

	char buf[512];
	Base::BitStream sendPacket( buf, sizeof(buf) );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_QUERYSRBONUS_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
	sendPacket.writeBits(sizeof(stSR_History)<<3, &BonusData);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket );
}

// ----------------------------------------------------------------------------
// 向Zone的玩家发送领取分期返利结果
void GoldExChangeManager::SendWZDrawSRBonus(int error, U32 PlayerId, U32 SRId, S32 PerMonth, 
							stSR_Bonus_Data& BonusData)
{
	DO_LOCK(Lock::GoldExchage);

	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int ZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int svrSocket = SERVER->GetServerManager()->GetGateSocket( LineId, ZoneId );
	if(0 == svrSocket)
	{
		if(error != 0)
			return;	//没有成功领取直接退出不发送

		//安全起见，如果找不到socket，就直接以系统逻辑事件返还给玩家
		DB_ZoneLogicEvent* pDBHandle = new DB_ZoneLogicEvent;
		pDBHandle->m_event.data_type = stEventItem::DATATYPE_BINDGOLD;
		pDBHandle->m_event.data_value = BonusData.BindGold;
		pDBHandle->m_event.sender = PlayerId;
		pDBHandle->m_event.recver = PlayerId;
		pDBHandle->m_playerId = PlayerId;
		pDBHandle->m_zoneId = ZoneId;
		pDBHandle->m_zoneSocketId = 0;
		SERVER->GetCommonDBManager()->SendPacket(pDBHandle);

		pDBHandle = new DB_ZoneLogicEvent;
		pDBHandle->m_event.data_type = stEventItem::DATATYPE_MONEY;
		pDBHandle->m_event.data_value = BonusData.Money;
		pDBHandle->m_event.sender = PlayerId;
		pDBHandle->m_event.recver = PlayerId;
		pDBHandle->m_playerId = PlayerId;
		pDBHandle->m_zoneId = ZoneId;
		pDBHandle->m_zoneSocketId = 0;
		SERVER->GetCommonDBManager()->SendPacket(pDBHandle);

		pDBHandle = new DB_ZoneLogicEvent;
		pDBHandle->m_event.data_type = stEventItem::DATATYPE_BINDMONEY;
		pDBHandle->m_event.data_value = BonusData.BindMoney;
		pDBHandle->m_event.sender = PlayerId;
		pDBHandle->m_event.recver = PlayerId;
		pDBHandle->m_playerId = PlayerId;
		pDBHandle->m_zoneId = ZoneId;
		pDBHandle->m_zoneSocketId = 0;
		SERVER->GetCommonDBManager()->SendPacket(pDBHandle);

		pDBHandle = new DB_ZoneLogicEvent;
		pDBHandle->m_event.data_type = stEventItem::DATATYPE_EXP;
		pDBHandle->m_event.data_value = BonusData.Exp;
		pDBHandle->m_event.sender = PlayerId;
		pDBHandle->m_event.recver = PlayerId;
		pDBHandle->m_playerId = PlayerId;
		pDBHandle->m_zoneId = ZoneId;
		pDBHandle->m_zoneSocketId = 0;
		SERVER->GetCommonDBManager()->SendPacket(pDBHandle);

		//日志记录：领取分期返利活动奖励
		GameplayLog::GL_DrawSRBonus(PlayerId, SRId, PerMonth, BonusData.Gold, BonusData.BindGold,
			BonusData.Money, BonusData.BindMoney, BonusData.Exp);
		
		g_Log.WriteWarn("查询玩家[%d]所在线所在地图失败!(SendWZDrawSRBonus)", PlayerId);
		return;
	}

	char buf[256];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_DRAWSRBONUS_RESPOND, PlayerId, SERVICE_ZONESERVER, ZoneId );
	sendPacket.writeInt(error, 32);
	sendPacket.writeInt(SRId, 32);
	sendPacket.writeBits(sizeof(stSR_Bonus_Data)<<3, &BonusData);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send(svrSocket, sendPacket );

	//根据当前活动查询角色能获得的分期返利的奖励数据
	DB_StagingRebate_QueryBonus* pDBHandle = new DB_StagingRebate_QueryBonus;
	pDBHandle->PlayerID = PlayerId;
	pDBHandle->BonusData.SRId = SRId;
	SERVER->GetGoldTradeDBManager()->SendPacket(pDBHandle);

	//日志记录：领取分期返利活动奖励
	GameplayLog::GL_DrawSRBonus(PlayerId, SRId, PerMonth, BonusData.Gold, BonusData.BindGold,
		BonusData.Money, BonusData.BindMoney, BonusData.Exp);
}
*/