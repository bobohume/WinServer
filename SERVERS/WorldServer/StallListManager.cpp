/*
#include "StallListManager.h"
#include "WorldServer.h"
#include "PlayerMgr.h"
#include "ServerMgr.h"
#include "MailManager.h"
#include "DBFixedStall.h"
#include "ChatHelper.h"
#include "GamePlayLog.h"
#include "wintcp/dtServerSocket.h"

CStallManager::CStallManager(void)
{
	registerEvent(ZW_STALL_QUERYSTALLLISTINFO_REQUEST,	&CStallManager::HandleQueryStallListInfoRequest);
	registerEvent(ZW_STALL_QUERYSTALLBASEINFO_REQUEST,	&CStallManager::HandleQueryStallBaseInfoRequest);
	registerEvent(ZW_STALL_QUERYSTALLDATA_REQUEST,		&CStallManager::HandleQueryStallDataRequest);
	registerEvent(ZW_STALL_STALLBUY_REQUEST,			&CStallManager::HandleStallBuyRequest);
	registerEvent(ZW_STALL_SETSTATUS_REQUEST,			&CStallManager::HandleSetStallStatusRequest);
	registerEvent(ZW_STALL_SETNAME_REQUEST,				&CStallManager::HandleSetStallNameRequest);
	registerEvent(ZW_STALL_SETATTORNPRICE_REQUEST,		&CStallManager::HandleSetAttornPriceRequest);
	registerEvent(ZW_STALL_NOTICE_REQUEST,				&CStallManager::HandleNoticeRequest);
	registerEvent(ZW_STALL_SETFLOWMONEY_REQUEST,		&CStallManager::HandleSetFlowMoneyRequest);
	registerEvent(ZW_STALL_SETFIXEDMONEY_REQUEST,		&CStallManager::HandleSetFixedMoneyRequest);
	registerEvent(ZW_STALL_SHOUTCONTENT_REQUEST,		&CStallManager::HandleShoutContentRequest);
	registerEvent(ZW_STALL_STALLTYPE_REQUEST,			&CStallManager::HandleStallTypeRequest);
	registerEvent(ZW_STALL_ISATTORN_REQUEST,			&CStallManager::HandleIsAttornRequest);
	registerEvent(ZW_STALL_ISSHOUTALL_REQUEST,			&CStallManager::HandleIsShoutAllRequest);
	registerEvent(ZW_STALL_SHOUTINTERVAL_REQUEST,		&CStallManager::HandleShoutIntervalRequest);
	registerEvent(ZW_STALL_BUYSTALLSLOTS_REQUEST,		&CStallManager::HandleBuyStallSlotsRequest);
	registerEvent(ZW_STALL_SELLITEM_REQUEST,			&CStallManager::HandleSellItemRequest);
	registerEvent(ZW_STALL_SELLPET_REQUEST,				&CStallManager::HandleSellPetRequest);
	registerEvent(ZW_STALL_BUYITEM_REQUEST,				&CStallManager::HandleBuyItemRequest);
	registerEvent(ZW_STALL_SETSHOUTITEM_REQUEST,		&CStallManager::HandleSetShoutItemRequest);
	registerEvent(ZW_STALL_PLAYERITEM_REQUEST,			&CStallManager::HandlePlayerItemRequest);
	registerEvent(ZW_STALL_SETSTALLMONEY_REQUEST,		&CStallManager::HandleSetStallMoneyRequest);
	
	registerEvent(CW_STALL_SENDCHAT_REQUEST,			&CStallManager::HandleSendChatRequest);
	registerEvent(CW_STALL_QUERYCHAT_REQUEST,			&CStallManager::HandleQueryChatRequest);
	registerEvent(CW_STALL_SENDCLEARCHAT_REQUEST,		&CStallManager::HandleSendClearChatRequest);
}

CStallManager::~CStallManager()
{
	m_stallMap.clear();
}

// ----------------------------------------------------------------------------
// 若共享内存无商铺数据，则从数据库更新到共享内存
bool CStallManager::LoadAllStalls(void)
{
	bool bLoaded = false;
    stFixedStall* pInfo         = new stFixedStall[MAX_FIXEDSTALLS];
	stFixedStallItem* pSellItem = new stFixedStallItem[MAX_FIXEDSTALLS * MAX_FIXEDSTALL_SELLITEM];
	stFixedStallItem* pBuyItem  = new stFixedStallItem[MAX_FIXEDSTALLS * MAX_FIXEDSTALL_BUYITEM];
	stFixedStallPet* pSellPet   = new stFixedStallPet[MAX_FIXEDSTALLS * MAX_FIXEDSTALL_SELLPET];
	
    TBLStallList tb_stalllist(SERVER->GetActorDB());
	
	//初始化商铺基本信息数据
	for(U32 i = 0; i < MAX_FIXEDSTALLS; i++)
    {
		pInfo[i].Init(i);
    }

	//加载商铺基本信息数据
	DBError err = tb_stalllist.LoadAllInfo(pInfo);
	if(err != DBERR_NONE)
	{
		g_Log.WriteFocus("LoadAllInfo error.");
		goto clearall;
	}
	
	//初始化商铺出售物品栏数据
	for(int i = 0; i < MAX_FIXEDSTALLS * MAX_FIXEDSTALL_SELLITEM; i++)
    {
		pSellItem[i].InitData(i/MAX_FIXEDSTALL_SELLITEM, i%MAX_FIXEDSTALL_SELLITEM);
    }

	//加载商铺出售物品栏数据
	err = tb_stalllist.LoadAllSellItem(pSellItem);
	if(err != DBERR_NONE)
	{
		g_Log.WriteFocus("LoadAllSellItem error.");
		goto clearall;
	}

	//初始化商铺收购物品栏数据
	for(int i = 0; i < MAX_FIXEDSTALLS * MAX_FIXEDSTALL_BUYITEM; i++)
    {
		pBuyItem[i].InitData(i/MAX_FIXEDSTALL_BUYITEM, i%MAX_FIXEDSTALL_BUYITEM);
    }

	//加载商铺收购物品栏数据
	err = tb_stalllist.LoadAllBuyItem(pBuyItem);
	if(err != DBERR_NONE)
	{
		g_Log.WriteFocus("LoadAllBuyItem error.");
		goto clearall;
	}

	//初始化商铺出售灵兽栏数据
	for(int i = 0; i < MAX_FIXEDSTALLS * MAX_FIXEDSTALL_SELLPET; i++)
    {
		pSellPet[i].InitData(i/MAX_FIXEDSTALL_SELLPET, i%MAX_FIXEDSTALL_SELLPET);
    }

	//加载商铺出售灵兽栏数据
	err = tb_stalllist.LoadAllSellPet(pSellPet);
	if(err != DBERR_NONE)
	{
		g_Log.WriteFocus("LoadAllSellPet error.");
		goto clearall;
	}

	//更新到共享内存,保持数据同步
	U32 k = 0;
	StallHandle pStall;
	for(U32 i = 0; i < MAX_FIXEDSTALLS; i++)
	{
        //如果数据已经在内存，则忽略从DB加载的数据
		pStall = SERVER->GetStallManager()->GetStall(i);
        if(0 == pStall)
        {
            pStall = new CStall(i);
            if (0 == pStall)
			{
				g_Log.WriteFocus("No enough memory be alloced for stallshop(%d)", i);
                goto clearall;
			}

			//Note:这里仅仅为商铺分配共享内存数据块，然后建立绑定关系
            if (!pStall->InitSM(i))
            {
                assert(0);
				g_Log.WriteFocus("Attch SM MEMORY failed for stallshop(%d)", i);
                delete pStall;
                goto clearall;
            }

            pStall->SetInfo(&pInfo[i]);

            for(int j = 0; j < MAX_FIXEDSTALL_SELLITEM; j++)
            {
				k = pInfo[i].StallID * MAX_FIXEDSTALL_SELLITEM + j;
				pStall->SetOneSellItem(&pSellItem[k]);
			}

			for(int j = 0; j < MAX_FIXEDSTALL_BUYITEM; j++)
			{
				k = pInfo[i].StallID * MAX_FIXEDSTALL_BUYITEM + j;
				pStall->SetOneBuyItem(&pBuyItem[k]);
			}

			for(int j = 0; j < MAX_FIXEDSTALL_SELLPET; j++)
			{
				k = pInfo[i].StallID * MAX_FIXEDSTALL_SELLPET + j;
				pStall->SetOneSellPet(&pSellPet[k]);
			}

			//Note:因为上面商铺数据由set数据库数据而获得变更，必须复制本地数据到共享.
			//内存，但不必savetodb,,所以即使此刻WorldDB退出，也不会引发保存．
			pStall->m_pSMData->Save(false, false);

			m_stallMap[i] = pStall;			
		}
	}
	bLoaded = true;
clearall:
	SAFE_DELETE_ARRAY(pInfo);
	SAFE_DELETE_ARRAY(pSellItem);
	SAFE_DELETE_ARRAY(pBuyItem);
	SAFE_DELETE_ARRAY(pSellPet);
	return bLoaded;
}

bool CStallManager::InitSMData(void)
{
	while(!m_stallSMPool.Init(SMKEY_STALLLIST))
	{
		g_Log.WriteLog("正在等待商铺数据共享内存初始化完成...");
		Sleep(1000);
	}	

	SM_STALL_DATA* pSMData = 0;

	for (U32 i = 0; i < SM_MAXSTALLLIST; ++i)
	{
		pSMData = m_stallSMPool.Get(i);
		if(0 == pSMData || !pSMData->IsUsed())
			continue;

		//这里先解锁,防止关闭时锁没有释放
		pSMData->UnLock();

		if (pSMData->IsCopying())
		{
			//pSMData->Reset();
			g_Log.WriteFocus("商铺数据从共享内存加载失败（原因:IsCopying, id=%d)", i);
			return false;
		}

		StallHandle pStall = new CStall(i);
		if (!pStall->InitSM(pSMData))
		{
			//pSMData->Reset();
			delete pStall;
			g_Log.WriteFocus("商铺数据从共享内存加载失败（原因:LoadFrom, id=%d)", i);
			return false;
		}

		if(pStall->GetStallID() != i)
		{
			g_Log.WriteFocus("商铺数据从共享内存加载数据混乱(原因:LoadFrom, srcid=%d, nowid=%d",
						i, pStall->GetStallID());
			delete pStall;
			return false;
		}

		m_stallMap[i] = pStall;
	}
	
	stWorldSetting& setting = SERVER->GetServerManager()->GetWorldSetting();
	m_TotalFlourish = setting.TotalFlourish;
	m_MaxStalls = setting.MaxStalls;
	return true;
}

StallHandle CStallManager::GetStall(U32 stallId)
{
	STALLMAP::iterator it = m_stallMap.find( stallId );
	if( it != m_stallMap.end() )
		return it->second;
	return StallHandle();
}

// ----------------------------------------------------------------------------
// 设置最大繁荣度（商业指数）
void CStallManager::SetTotalFlourish(U32 iAllFlourish)
{
	if(iAllFlourish < 2000)
	{
		SetMaxStalls(50);
	}
	else if(iAllFlourish >= 2000 && iAllFlourish < 4000)
	{
		SetMaxStalls(100);
	}
	else if(iAllFlourish >= 4000 && iAllFlourish < 6000)
	{
		SetMaxStalls(150);
	}
	else if(iAllFlourish >= 6000 && iAllFlourish < 8000)
	{
		SetMaxStalls(200);
	}
	else
	{
		SetMaxStalls(250);
	}

	m_TotalFlourish = iAllFlourish;

	SERVER->GetServerManager()->SetStallShopSetting(GetMaxStalls(), m_TotalFlourish);

	UpdateAllStallTax();
}

// ----------------------------------------------------------------------------
// 更新所有商铺的商铺税收
void CStallManager::UpdateAllStallTax()
{
	S32 TotalFlourish = GetTotalFlourish();
	for(U32 i = 0; i < m_MaxStalls; i++)
	{
		StallHandle pStall = GetStall(i);
		if(pStall)
		{
			pStall->SetStallTax(GetDayTax(pStall->GetFlourish()));
			pStall->SetInfoVer();
		}
	}
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求查询商铺总数及商业指数数据
void CStallManager::HandleQueryStallListInfoRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );
	char buf[128];
	Base::BitStream sendPacket( buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_QUERYSTALLLISTINFO_RESPOND, GetTotalFlourish(), SERVICE_ZONESERVER, pHead->SrcZoneId);
	sendPacket.writeInt(GetMaxStalls(), 16);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求查询更新所有商铺基本信息的消息
void CStallManager::HandleQueryStallBaseInfoRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );
	U32 ver[MAX_FIXEDSTALLS];
	Packet->readBits(sizeof(U32)*8*MAX_FIXEDSTALLS, ver);

	std::vector<StallHandle> vecInfo;
	for(size_t i = 0; i < GetMaxStalls(); i++)
	{
		StallHandle pStall = GetStall(i);
		if(!pStall)
			continue;

		if(ver[i] != pStall->GetInfoVer())
		{
			vecInfo.push_back(pStall);
		}
	}

	SendStallInfoData(SocketHandle, pHead->SrcZoneId, vecInfo);
	vecInfo.clear();
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求查询更新所有商铺数据的消息
void CStallManager::HandleQueryStallDataRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );

	size_t srcsize = Packet->readInt(32);
	char* src = new char[srcsize];
	Packet->readBits(srcsize<<3, src);

	size_t tagsize = sizeof(stStallVersion) * MAX_FIXEDSTALLS;
	stStallVersion ver[MAX_FIXEDSTALLS];

	if(SXZ_UNCOMPRESS(ver, &tagsize, src, srcsize) != 0)
	{
		g_Log.WriteFocus("HandleQueryStallDataRequest 解压缩商铺版本数据失败!");
		delete[] src;
		return;
	}

	delete[] src;
	std::vector<stFixedStallItem*> vecSellItem;
	std::vector<stFixedStallItem*> vecBuyItem;
	std::vector<stFixedStallPet*> vecSellPet;
	for(size_t i = 0; i < GetMaxStalls(); i++)
	{
		StallHandle pStall = GetStall(i);
		if(!pStall)
			continue;

		//若出售物品栏数据版本不同,则更新
		for(int j = 0; j < MAX_FIXEDSTALL_SELLITEM; j++)
		{
			if(ver[i].SellItemVer[j] != pStall->GetSellItemVer(j))
				vecSellItem.push_back(pStall->GetOneSellItem(j));
		}

		for(int j = 0; j < MAX_FIXEDSTALL_BUYITEM; j++)
		{
			if(ver[i].BuyItemVer[j] != pStall->GetBuyItemVer(j))
				vecBuyItem.push_back(pStall->GetOneBuyItem(j));
		}

		for(int j = 0; j < MAX_FIXEDSTALL_SELLPET; j++)
		{
			if(ver[i].SellPetVer[j] != pStall->GetSellPetVer(j))
				vecSellPet.push_back(pStall->GetOneSellPet(j));
		}
	}

	SendSellItemData(SocketHandle, pHead->SrcZoneId, vecSellItem);
	SendBuyItemData(SocketHandle, pHead->SrcZoneId, vecBuyItem);
	SendSellPetData(SocketHandle, pHead->SrcZoneId, vecSellPet);
	vecSellItem.clear();
	vecBuyItem.clear();
	vecSellPet.clear();
}

// ----------------------------------------------------------------------------
// 向Zone发送商铺基本信息数据
void CStallManager::SendStallInfoData(S32 SocketHandle, U32 srcZoneId, std::vector<StallHandle>& vecInfo)
{
	if(vecInfo.empty())
	{
		char buf[128];
		Base::BitStream sendPacket(buf, sizeof(buf));
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_QUERYINFO_RESPOND, 0, SERVICE_ZONESERVER, srcZoneId);
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
		return;
	}

	U16 stallcount = vecInfo.size();
	char* buffer = new char[MAX_PLAYER_STRUCT_SIZE];
	Base::BitStream sendPacket(buffer, MAX_PLAYER_STRUCT_SIZE);
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_QUERYINFO_RESPOND, stallcount, SERVICE_ZONESERVER, srcZoneId);
	
	//发送商铺基本信息，以压缩数据形式
	size_t srcsize = sizeof(stFixedStall) * stallcount;
	stFixedStall* src = new stFixedStall[stallcount];
	for(size_t i = 0; i < vecInfo.size(); i++)
	{
		memcpy(&src[i], vecInfo[i]->GetInfo(), sizeof(stFixedStall));
	}

	size_t tagsize = srcsize + 400;
	char* tag = new char[tagsize];
	if(SXZ_COMPRESS(tag, &tagsize, (char*)src, srcsize) == 0 && tagsize <= srcsize)
	{
		sendPacket.writeInt(tagsize, 32);
		sendPacket.writeBits(tagsize<<3, tag);
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
	}
	else
	{
		g_Log.WriteFocus("压缩发送商铺基本信息数据失败!");
	}

	delete[] src;
	delete[] tag;
	delete[] buffer;	
}

// ----------------------------------------------------------------------------
// 向Zone发送商铺出售物品栏数据
void CStallManager::SendSellItemData(S32 SocketHandle, U32 srcZoneId, std::vector<stFixedStallItem*>& vecItem)
{
	if(vecItem.empty())
		return;

	int count = vecItem.size();
	size_t srcsize = sizeof(stFixedStallItem) * count;
	stFixedStallItem* src = new stFixedStallItem[count];
	for(size_t i = 0; i < vecItem.size(); i++)
	{
		memcpy(&src[i], vecItem[i], sizeof(stFixedStallItem));
	}

	size_t tagsize = srcsize + 400;
	char* tag = new char[tagsize];
	if(SXZ_COMPRESS(tag, &tagsize, (char*)src, srcsize) != 0 || tagsize > srcsize)
	{
		g_Log.WriteFocus("压缩发送商铺出售物品栏数据失败!");
		delete[] src;
		delete[] tag;
		return;
	}

	//压缩数据分块发送
	const U32 MAX_BLOCKS = MAX_PLAYER_STRUCT_SIZE-128;

	int blockcount = tagsize / MAX_BLOCKS;
	if(tagsize % MAX_BLOCKS != 0)
		blockcount ++;
	
	char* buffer = new char[MAX_PLAYER_STRUCT_SIZE];
	char* offset = tag;
	int writeSize;
	for(int i = 0; i < blockcount; i++)
	{
		memset(buffer, 0, MAX_PLAYER_STRUCT_SIZE);
		Base::BitStream sendPacket( buffer, MAX_PLAYER_STRUCT_SIZE);
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_QUERYSELLITEM_RESPOND, count, SERVICE_ZONESERVER, srcZoneId);
		sendPacket.writeInt(blockcount, 8);
		sendPacket.writeInt(i, 8);
		if(i == blockcount-1)
			writeSize = tagsize-i*MAX_BLOCKS;
		else
			writeSize = MAX_BLOCKS;
		sendPacket.writeInt(writeSize, 32);
		sendPacket.writeBits(writeSize<<3, offset);
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
		offset += MAX_BLOCKS;
	}

	delete[] src;
	delete[] tag;
	delete[] buffer;
}

// ----------------------------------------------------------------------------
// 向Zone发送商铺收购物品栏数据
void CStallManager::SendBuyItemData(S32 SocketHandle, U32 srcZoneId, std::vector<stFixedStallItem*>& vecItem)
{
	if(vecItem.empty())
		return;

	int count = vecItem.size();
	size_t srcsize = sizeof(stFixedStallItem) * count;
	stFixedStallItem* src = new stFixedStallItem[count];
	for(size_t i = 0; i < vecItem.size(); i++)
	{
		memcpy(&src[i], vecItem[i], sizeof(stFixedStallItem));
	}

	size_t tagsize = srcsize + 400;
	char* tag = new char[tagsize];
	if(SXZ_COMPRESS(tag, &tagsize, (char*)src, srcsize) != 0 || tagsize > srcsize)
	{
		g_Log.WriteFocus("压缩发送商铺收购物品栏数据失败!");
		delete[] src;
		delete[] tag;
		return;
	}

	//压缩数据分块发送
	const U32 MAX_BLOCKS = MAX_PLAYER_STRUCT_SIZE-128;

	int blockcount = tagsize / MAX_BLOCKS;
	if(tagsize % MAX_BLOCKS != 0)
		blockcount ++;

	char* buffer = new char[MAX_PLAYER_STRUCT_SIZE];
	char* offset = tag;
	int writeSize;
	for(int i = 0; i < blockcount; i++)
	{
		memset(buffer, 0, MAX_PLAYER_STRUCT_SIZE);
		Base::BitStream sendPacket( buffer, MAX_PLAYER_STRUCT_SIZE);
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_QUERYBUYITEM_RESPOND, count, SERVICE_ZONESERVER, srcZoneId);
		sendPacket.writeInt(blockcount, 8);
		sendPacket.writeInt(i, 8);
		if(i == blockcount-1)
			writeSize = tagsize-i*MAX_BLOCKS;
		else
			writeSize = MAX_BLOCKS;
		sendPacket.writeInt(writeSize, 32);
		sendPacket.writeBits(writeSize<<3, offset);
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
		offset += MAX_BLOCKS;
	}

	delete[] src;
	delete[] tag;
	delete[] buffer;
}

// ----------------------------------------------------------------------------
// 向Zone发送商铺出售灵兽栏数据
void CStallManager::SendSellPetData(S32 SocketHandle, U32 srcZoneId, std::vector<stFixedStallPet*>& vecPet)
{
	if(vecPet.empty())
		return;

	int count = vecPet.size();
	size_t srcsize = sizeof(stFixedStallPet) * count;
	stFixedStallPet* src = new stFixedStallPet[count];
	for(size_t i = 0; i < vecPet.size(); i++)
	{
		memcpy(&src[i], vecPet[i], sizeof(stFixedStallPet));
	}

	size_t tagsize = srcsize + 400;
	char* tag = new char[tagsize];
	if(SXZ_COMPRESS(tag, &tagsize, (char*)src, srcsize) != 0 || tagsize > srcsize)
	{
		g_Log.WriteFocus("压缩发送商铺出售灵兽栏数据失败!");
		delete[] src;
		delete[] tag;
		return;
	}

	//压缩数据分块发送
	const U32 MAX_BLOCKS = MAX_PLAYER_STRUCT_SIZE-128;

	int blockcount = tagsize / MAX_BLOCKS;
	if(tagsize % MAX_BLOCKS != 0)
		blockcount ++;

	char* buffer = new char[MAX_PLAYER_STRUCT_SIZE];
	char* offset = tag;
	int writeSize;
	for(int i = 0; i < blockcount; i++)
	{
		memset(buffer, 0, MAX_PLAYER_STRUCT_SIZE);
		Base::BitStream sendPacket( buffer, MAX_PLAYER_STRUCT_SIZE);
		stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_QUERYSELLPET_RESPOND, count, SERVICE_ZONESERVER, srcZoneId);
		sendPacket.writeInt(blockcount, 8);
		sendPacket.writeInt(i, 8);
		if(i == blockcount-1)
			writeSize = tagsize-i*MAX_BLOCKS;
		else
			writeSize = MAX_BLOCKS;
		sendPacket.writeInt(writeSize, 32);
		sendPacket.writeBits(writeSize<<3, offset);
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
		offset += MAX_BLOCKS;
	}

	delete[] src;
	delete[] tag;
	delete[] buffer;
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求购买商铺的消息
void CStallManager::HandleStallBuyRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );

	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//待购买的商铺ID
	S32 money		= Packet->readInt(32);	//购买需消耗的金元
	U32 itemId		= Packet->readInt(32);	//购买需消耗的行商秘籍
	char PlayerName[32] = {0};				//玩家名称
	Packet->readString(PlayerName, 32);

	U32 curtime;
	_time32((__time32_t*)&curtime);

	int error = 0;
	StallHandle pStall = GetStall(stallId);

	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;
		}

		//判断是否非系统商铺
		if(pStall->GetStallerID() != 0)
		{
			if(pStall->GetStallerID() == PlayerId)
			{
				error = 3417;
				break;//不能购买自己的商铺
			}

			if(pStall->GetStatus() != SS_OPEN)
			{
				error = 3426;
				break;//商铺未开放,不能购买
			}

			if(!pStall->IsAttorn())
			{
				error = 3403;
				break;//商铺未转让,不能购买
			}

			if(curtime - pStall->GetAttornStartTime() < 3600)
			{
				error = 3403;
				break;//商铺未转让,不能购买
			}

			if(money != (pStall->GetAttornPrice() + BUYSTALLIMPREST))
			{
				error = 3400;
				break;//购买商铺金元不足
			}
		}
		else
		{
			if(money != (GetBuySystemStall() + BUYSTALLIMPREST))
			{
				error = 3400;
				break;//购买商铺金元不足
			}
		}
	} while (false);

	if(error != 0)
	{
		char szMsg[1024] = {0};
		sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
			"    您对【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】的购买商铺"\
			"操作失败,系统退还金钱和物品给您,请及时收取.</t>", PlayerName, stallId+1);
		SERVER->GetMailManager()->sendMail(0, PlayerId, money, itemId, 1, "【商铺购买】", szMsg, 1);
		//向客户端通知邮件更新
		SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );

		//向Zone回应购买商铺消息
		SendStallBuyRespond(PlayerId, stallId, 0, NULL, error);
	}
	else
	{
		money -= BUYSTALLIMPREST;
		//先处理原铺主
		if(pStall->GetStallerID() != 0)
		{
			struct tm today;
			_localtime32_s(&today, (__time32_t*)&curtime);
			char szMsg[1024] = {0};

			sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】于%d年%2d月%2d日"\
				"被玩家[</t><t c='0x00ff00ff'>%s</t><t c='0xff9900ff'>]成功收购，系统退还您"\
				"的流动资金,请及时收取.</t>", pStall->GetStallerName(), stallId+1,
				today.tm_year+1900, today.tm_mon+1, today.tm_mday, PlayerName);
			SERVER->GetMailManager()->sendMail(0, pStall->GetStallerID(), pStall->GetFlowMoney(), 0, 0, "【商铺转让】", szMsg, 1);

			sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】于%d年%2d月%2d日"\
				"被玩家[</t><t c='0x00ff00ff'>%s</t><t c='0xff9900ff'>]成功收购，系统退还您"\
				"的固定资金,请及时收取.</t>", pStall->GetStallerName(), stallId+1,
				today.tm_year+1900, today.tm_mon+1, today.tm_mday, PlayerName);
			SERVER->GetMailManager()->sendMail(0, pStall->GetStallerID(), pStall->GetFixedMoney(), 0, 0, "【商铺转让】", szMsg, 1);

			sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】于%d年%2d月%2d日"\
				"被玩家[</t><t c='0x00ff00ff'>%s</t><t c='0xff9900ff'>]成功收购，系统返还您的"\
				"出售商铺价钱,请及时收取.</t>", pStall->GetStallerName(), stallId+1,
				today.tm_year+1900, today.tm_mon+1, today.tm_mday, PlayerName);
			SERVER->GetMailManager()->sendMail(0, pStall->GetStallerID(), money, 0, 0, "【商铺转让】", szMsg, 1);

			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(pStall->GetStallerID(),WORLD_CLIENT_MailNotify );

			//被收购后，转让价格会上调10%
			pStall->SetAttornPrice(money*11/10);
		}
		else
			pStall->SetAttornPrice(money);

		//再处理购买者
		char stallName[32];
		sprintf_s(stallName, sizeof(stallName), "%s的商铺", PlayerName);
		pStall->SetStallName(stallName);
		pStall->SetPreStallerID(pStall->GetStallerID());
		pStall->SetStallerID(PlayerId);
		pStall->SetStallerName(PlayerName);
		pStall->SetStatus(SS_MODIFY);
		pStall->SetPayTaxTime(getZeroTime(curtime));
		pStall->SetIsAttorn(false);
		pStall->SetAttornStartTime(0);
		pStall->SetFixedMoney(BUYSTALLIMPREST);
		pStall->SetFlowMoney(0);
		pStall->SetNotice("");
		pStall->SetBuyTime(curtime);

		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();

		//日志记录：购买商铺
		GameplayLog::GL_ShopBuyShop(PlayerId, pStall->GetStallID(), money+BUYSTALLIMPREST, BUYSTALLITEM);

		//向Zone回应购买商铺消息
		SendStallBuyRespond(PlayerId, stallId, pStall->GetInfoVer(), pStall->GetInfo(), error);
	}	
}

// ----------------------------------------------------------------------------
// 向Zone回应购买商铺消息
void CStallManager::SendStallBuyRespond(U32 PlayerId, U16 stallId, U32 infoVer, stFixedStall* pInfo, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[4096];
	Base::BitStream sendPacket( buf, sizeof(buf) );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_STALLBUY_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(infoVer, 32);
	sendPacket.writeInt(error, 32);
	if(error == 0)
	{
		sendPacket.writeBits(sizeof(stFixedStall)<<3, pInfo);
	}
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求设置商铺状态的消息
void CStallManager::HandleSetStallStatusRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	U8  state		= Packet->readInt(8);	//玩家修改商铺状态

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断状态是否正确
		if(state != SS_MODIFY && state != SS_OPEN)
		{
			error = 3432;
			break;//设置状态不正确
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetStatus(state);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}

	SendSetStallStatusRespond(PlayerId, stallId, state, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺状态消息
void CStallManager::SendSetStallStatusRespond(U32 PlayerId, U16 stallId, U8 state, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket( buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SETSTATUS_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(state, 8);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求修改商铺名称的消息
void CStallManager::HandleSetStallNameRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	char szName[32] = {0};
	Packet->readString(szName, 32);			//商铺名称

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3401;
			break;
		}
		
		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;
		}

		//判断商铺是否处于修改模式
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetStallName(szName);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}

	SendSetStallNameRespond(PlayerId, stallId, szName, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺名称消息
void CStallManager::SendSetStallNameRespond(U32 PlayerId, U16 stallId, const char* name, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[200];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SETNAME_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeString(name, 32);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求设置商铺转让价格的消息
void CStallManager::HandleSetAttornPriceRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	S32 money		= Packet->readInt(32);	//玩家修改商铺的转让价格

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall  || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断状态是否正确
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//商铺正在收摊状态,不能修改
		}

		if(money < ATTORNPRICE)
		{
			error = 3404;
			break;//请输入正确的转让价格
		}

		 if(money > MAX_MONEY_NUM)
		 {
			 error = 617;
			 break;//设置的金钱达到上限
		 }
	}while (false);

	if(error == 0)
	{
		pStall->SetAttornPrice(money);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}
	SendSetAttornPriceRespond(PlayerId, stallId, money, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺状态消息
void CStallManager::SendSetAttornPriceRespond(U32 PlayerId, U16 stallId, S32 money, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket(buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SETATTORNPRICE_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(money, 32);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求修改商铺告示的消息
void CStallManager::HandleNoticeRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	char szWords[128] = {0};
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	Packet->readString(szWords, 128);		//玩家修改商铺告示

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改商铺
		}

		//判断商铺是否处于修改模式
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 3432;
			break;//商铺操作失败
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetNotice(szWords);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}

	SendNoticeRespond(PlayerId, stallId, szWords, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺留言消息
void CStallManager::SendNoticeRespond(U32 PlayerId, U16 stallId, const char* words, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[250];
	Base::BitStream sendPacket( buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_NOTICE_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeString(words, 128);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求设置流动资金的消息
void CStallManager::HandleSetFlowMoneyRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	S32 money		= Packet->readInt(32);	//玩家修改商铺的流动资金
	bool flag		= Packet->readFlag();	//流动资金是否取出
	char szPlayerName[32];
	Packet->readString(szPlayerName, 32);	//玩家名称

	int error = 0;
	S32 iFlowMoney = 0, srcFlowMoney = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断状态是否正确
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//商铺应先收摊再修改
		}

		if(money <= 0)
		{
			error = 580;//请输入正确的数值
			break;			
		}

		iFlowMoney = pStall->GetFlowMoney();
		srcFlowMoney = iFlowMoney;
		if(flag == 0)		//存入流动资金
		{
			S64 iMoney = (S64)iFlowMoney + money;			
			if (iMoney > MAX_MONEY_NUM)
				error = 3434;//存入流动资金已达上限
			else
				iFlowMoney = (S32)iMoney;
		}
		else				//取出流动资金
		{
			if(iFlowMoney < money)
				error = 3436;//请输入正确的数值
			else
				iFlowMoney -= money;
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetFlowMoney(iFlowMoney);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();

		if(flag == 1)
		{
			char szMsg[1024] = {0};
			sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】取出流动资金成功,系统返还您的金元，请及时收取!</t>",
				szPlayerName, stallId + 1);
			//取出的流动资金以邮件归还
			SERVER->GetMailManager()->sendMail(0, pStall->GetStallerID(), money, 0, 0, "【取出商铺资金】", szMsg, 1);
			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );
		}

		//日志记录:商铺流动资金存取
		GameplayLog::GL_ShopFlowMoney(PlayerId, pStall->GetStallID(), srcFlowMoney, iFlowMoney);
	}
	else
	{
		if(flag == 0)
		{
			char szMsg[1024] = {0};
			sprintf_s(szMsg, 1024, "<t c='0xff990ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】存入流动资金失败,系统退还您的金元，请及时收取!</t>",
				szPlayerName, stallId + 1);
			//失败必须以邮件退还预扣的金钱
			SERVER->GetMailManager()->sendMail(0, PlayerId, money, 0, 0, "【存入商铺资金】", szMsg, 1);
			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );
		}
	}

	SendSetFlowMoneyRespond(PlayerId, stallId, money, flag, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺流动资金消息
void CStallManager::SendSetFlowMoneyRespond(U32 PlayerId, U16 stallId, S32 money, bool flag, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket( buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SETFLOWMONEY_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(money, 32);
	sendPacket.writeFlag(flag);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求设置固定资金的消息
void CStallManager::HandleSetFixedMoneyRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	S32 money		= Packet->readInt(32);	//玩家修改商铺的流动资金
	bool flag		= Packet->readFlag();
	char szPlayerName[32];
	Packet->readString(szPlayerName, 32);	//玩家名称

	int error = 0;
	S32 iFixedMoney = 0, srcFixedMoney = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断状态是否正确
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//商铺应先收摊再修改
		}

		if(money <= 0)
		{
			error = 580;//请输入正确的数值
			break;			
		}

		iFixedMoney = pStall->GetFixedMoney();
		srcFixedMoney = iFixedMoney;
		if(flag == 0)		//存入流动资金
		{
			S64 iMoney = (S64)iFixedMoney + money;			
			if (iMoney > MAX_MONEY_NUM)
				error = 3434;//存入流动资金已达上限
			else
				iFixedMoney = (S32)iMoney;
		}
		else				//取出流动资金
		{
			if(iFixedMoney < money)
				error = 3436;//请输入正确的数值
			else
				iFixedMoney -= money;
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetFixedMoney(iFixedMoney);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();

		//if(flag == 1)
		//{
		//	char szMsg[1024] = {0};
		//	sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
		//		"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】取出固定资金成功,系统返还您的金元，请及时收取!</t>",
		//		szPlayerName, stallId + 1);
		//	//取出的固定资金以邮件归还
		//	SERVER->GetMailManager()->sendMail(0, pStall->GetStallerID(), money, 0, 0, "【取出商铺资金】", szMsg, 1);
		//	//向客户端通知邮件更新
		//	SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );
		//}

		//日志记录:商铺固定资金存取
		GameplayLog::GL_ShopFixedMoney(PlayerId, pStall->GetStallID(), srcFixedMoney, iFixedMoney);
	}
	else
	{
		if(flag == 0)
		{
			char szMsg[1024] = {0};
			sprintf_s(szMsg, 1024, "<t c='0xff990ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】存入固定资金失败,系统退还您的金元，请及时收取!</t>",
				szPlayerName, stallId + 1);
			//失败必须以邮件退还预扣的金钱
			SERVER->GetMailManager()->sendMail(0, PlayerId, money, 0, 0, "【存入商铺资金】", szMsg, 1);
			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );
		}
	}

	SendSetFixedMoneyRespond(PlayerId, stallId, money, flag, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺固定资金消息
void CStallManager::SendSetFixedMoneyRespond(U32 PlayerId, U16 stallId, S32 money, bool flag, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	CMemGuard buf( 128 MEM_GUARD_PARAM );
	Base::BitStream sendPacket( buf.get(), 128 );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SETFIXEDMONEY_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(money, 32);
	sendPacket.writeFlag(flag);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求修改商铺喊话内容的消息
void CStallManager::HandleShoutContentRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	char szShoutContent[128] = {0};
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	Packet->readString(szShoutContent, 128);//玩家修改商铺的喊话内容

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断商铺是否处于修改模式
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//请先收摊,再操作
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetShoutContent(szShoutContent);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}

	SendShoutContentRespond(PlayerId, stallId, szShoutContent, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺喊话内容消息
void CStallManager::SendShoutContentRespond(U32 PlayerId, U16 stallId, const char* words, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[250];
	Base::BitStream sendPacket(buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SHOUTCONTENT_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeString(words, 128);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求设置商铺类型的消息
void CStallManager::HandleStallTypeRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	U8  type		= Packet->readInt(8);	//玩家修改商铺类型

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断商铺是否处于修改模式
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//请先收摊,再操作
		}

		//判断类型是否正确
		if(type > STALLTYPE_MATERAIL)
		{
			error = 3432;
			break;//不能操作
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetStallType(type);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}

	SendSetStallTypeRespond(PlayerId, stallId, type, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺类型消息
void CStallManager::SendSetStallTypeRespond(U32 PlayerId, U16 stallId, U8 type, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_STALLTYPE_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(type, 8);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求设置商铺是否转让的消息
void CStallManager::HandleIsAttornRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	bool IsAttorn	= Packet->readFlag();	//是否转让标志

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断商铺是否处于修改模式
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//请先收摊,再操作
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetIsAttorn(IsAttorn);
		if(IsAttorn)
		{
			U32 curtime;
			_time32((__time32_t*)&curtime);
			pStall->SetAttornStartTime(curtime);
		}
		else
		{
			pStall->SetAttornStartTime(0);
		}	

		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}

	SendIsAttornRespond(PlayerId, stallId, IsAttorn, pStall->GetAttornStartTime(), error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺是否转让消息
void CStallManager::SendIsAttornRespond(U32 PlayerId, U16 stallId, bool flag,U32 attorntime, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket(buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_ISATTORN_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeFlag(flag);
	sendPacket.writeInt(attorntime, 32);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求设置商铺是否启用喊话的消息
void CStallManager::HandleIsShoutAllRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	bool IsShoutAll	= Packet->readFlag();	//是否启用喊话标志

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断商铺是否处于修改模式
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//请先收摊,再操作
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetIsShoutAll(IsShoutAll);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}

	SendIsShoutAllRespond(PlayerId, stallId, IsShoutAll, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺是否转让消息
void CStallManager::SendIsShoutAllRespond(U32 PlayerId, U16 stallId, bool flag, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_ISSHOUTALL_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeFlag(flag);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求设置商铺喊话间隔的消息
void CStallManager::HandleShoutIntervalRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	U16  interval	= Packet->readInt(16);	//玩家修改商铺喊话间隔

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断商铺是否处于修改模式
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//请先收摊,再操作
		}

		//判断喊话间隔是否正确
		if(interval != SHOUTINTERVAL0 && interval != SHOUTINTERVAL1 && interval != SHOUTINTERVAL2)
		{
			error = 3432;
			break;//商铺操作失败
		}
	}while (false);

	if(error == 0)
	{
		pStall->SetShoutInterval(interval);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}

	SendShoutIntervalRespond(PlayerId, stallId, interval, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺喊话间隔消息
void CStallManager::SendShoutIntervalRespond(U32 PlayerId, U16 stallId, U16 interval, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket(buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SHOUTINTERVAL_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(interval, 16);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求设置商铺购买栏位数的消息
void CStallManager::HandleBuyStallSlotsRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	U8  type		= Packet->readInt(8);	//是否出售物品栏OR出售灵兽栏
	U8	page		= Packet->readInt(8);	//栏位页数
	char szPlayerName[32];
	Packet->readString(szPlayerName, 32);	//玩家名称

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		if((type != 1 && type != 2) || page > 10)
		{
			error = 3414;
			break;
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断商铺是否处于修改模式
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//请先收摊,再操作
		}

		S32 curPage = 0;
		if(type == 1)
			curPage = pStall->GetBuySellItems() / INITSELLITEMS;
		else if(type == 2)
			curPage = pStall->GetBuySellPets() /INITSELLPETS;

		//判断栏位页数是否合法
		if(curPage == 0 || page != (curPage + 1))
		{
			error = 3432;
			break;//商铺操作失败
		}
	}while (false);

	if(error == 0)
	{
		if(!pStall)
		{
			g_Log.WriteError("购买商铺栏位，未查询到商铺对象!(stallid=%d",stallId);
			return;
		}
		if(type == 1)
		{
			pStall->SetBuySellItems(page * INITSELLITEMS);
		}
		else
		{
			pStall->SetBuySellPets(page * INITSELLPETS);
		}
		U8 flourish = pStall->GetFlourish();
		pStall->SetFlourish(flourish + 5);
		S32 tax = 12 - (pStall->GetFlourish() * 0.1f);
		pStall->SetTradeTax(tax);		
		pStall->SetStallTax(GetDayTax(pStall->GetFlourish()));
		//U32 totalFlourish = GetTotalFlourish();
		//SetTotalFlourish(totalFlourish + 5);
		pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();

		//日志记录：购买栏位
		GameplayLog::GL_ShopBuySlots(PlayerId, stallId, type, page);

		SendBuyStallSlotsRespond(PlayerId, stallId, type, page, pStall->GetFlourish(), pStall->GetStallTax(),
				pStall->GetTradeTax(), error);
	}
	else
	{
		S32 money = page * 10000;
		//失败需要退回购买栏位金钱
		char szMsg[1024] = {0};
		sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
			"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】购买%s第%d页的操作失败，系统退还您的金钱,请及时收取!</t>",
			szPlayerName, stallId+1, type==1?"出售物品栏":"出售灵兽栏", page);
		SERVER->GetMailManager()->sendMail(0, PlayerId, money, 0, 0, "【购买商铺栏位】", szMsg, 1);
		//向客户端通知邮件更新
		SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );

		SendBuyStallSlotsRespond(PlayerId, stallId, type, page, 0, 0, 0, error);
	}	
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺购买栏位数消息
void CStallManager::SendBuyStallSlotsRespond(U32 PlayerId, U16 stallId, U8 type, U8 page, U8 flourish,
							S32 stallTax, S32 tradeTax, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_BUYSTALLSLOTS_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(type, 8);
	sendPacket.writeInt(page, 8);
	sendPacket.writeInt(flourish, 8);
	sendPacket.writeInt(stallTax, 32);
	sendPacket.writeInt(tradeTax, 32);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求出售物品栏物品上下架的消息
void CStallManager::HandleSellItemRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	S16	srcIndex	= Packet->readInt(16);	//商铺出售栏索引位置
	S16	tagIndex	= Packet->readInt(16);	//包裹栏索引位置
	bool isUpItem	= Packet->readFlag();	//是否上架物品
	S32	price		= Packet->readInt(32);	//物品价格
	char szPlayerName[32];
	Packet->readString(szPlayerName, 32);	//玩家名称

	stFixedStallItem item;
	Packet->readBits(sizeof(stItemInfo)<<3, &item);

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺已不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主，不能修改操作
		}

		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2308;
			break;//当前已出摊，请收摊再试
		}

		if(isUpItem)	//物品上架
		{
			if(tagIndex != -1)
			{
				//判断是否操作索位位置越界
				if(tagIndex < 0 || tagIndex >= pStall->GetBuySellItems())
				{
					error = 3023;
					break;//无法拖至目标栏位
				}

				//目标物品栏上不能有的物品
				stFixedStallItem* pItem = pStall->GetOneSellItem(tagIndex);
				if(pItem && pItem->ItemID != 0)
				{
					error = 3022;
					break;//目标已经有物品
				}				
			}
			else
			{
				tagIndex = pStall->FindEmptyPosFromSellItem();
				if(tagIndex == -1)
				{
					error = 3023;
					break;//无法添加到目标栏位
				}
			}
			item.StallID = pStall->GetStallID();
			item.Pos = tagIndex;
			item.Price = price;
			pStall->SetOneSellItem(&item);
			pStall->SetSellItemVer(tagIndex);
			pStall->UpdateSMData(StallSMData::MASK_SELLITEM, tagIndex);

			//记录日志:铺主上架出售物品栏物品成功
			GameplayLog::GL_ShopSetSellItem(PlayerId, LOG_SHOP_UPSELLITEM_OK, &item);
		}
		else
		{
			//判断是否操作索位位置越界
			if(srcIndex < 0 || srcIndex >= pStall->GetBuySellItems())
			{
				error = 3072;
				break;//目标选择错误
			}
			stFixedStallItem* pSrcItem = pStall->GetOneSellItem(srcIndex);
			//判断位置上是否有物品
			if(!pSrcItem || pSrcItem->ItemID != item.ItemID)
			{
				error = 3020;
				break;//目标不存在，无法操作
			}
			item = *pSrcItem;
			pSrcItem->InitData(stallId, pSrcItem->Pos);
			pStall->SetSellItemVer(srcIndex);
			pStall->UpdateSMData(StallSMData::MASK_SELLITEM, srcIndex);

			//记录日志:铺主下架出售物品栏物品成功
			GameplayLog::GL_ShopSetSellItem(PlayerId, LOG_SHOP_DOWNSELLITEM_OK, &item);
		}		
	}while (false);

	if(error != 0)
	{
		if(isUpItem)
		{
			char szMsg[1024] = {0};
			sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】物品上架失败,系统退还物品,请及时收取.</t>",
				szPlayerName, stallId+1);
			SERVER->GetMailManager()->sendMail(0, PlayerId, 0, item, "【商铺物品设置】", szMsg, 1);
			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );

			//记录日志:铺主上架出售物品栏物品失败
			GameplayLog::GL_ShopSetSellItem(PlayerId,LOG_SHOP_UPSELLITEM_FAIL, &item);
		}
	}

	SendSellItemRespond(PlayerId, szPlayerName, stallId, isUpItem, item, price, srcIndex, tagIndex, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应处理商铺出售物品栏物品上架消息
void CStallManager::SendSellItemRespond(U32 PlayerId, const char* PlayerName, U16 stallId, bool isUpItem, stItemInfo& item,
						S32 price, S32 srcIndex, S32 tagIndex, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[4096];
	Base::BitStream sendPacket(buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SELLITEM_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(srcIndex, 16);
	sendPacket.writeInt(tagIndex, 16);
	sendPacket.writeFlag(isUpItem);
	sendPacket.writeInt(price, 32);
	sendPacket.writeString(PlayerName, MAX_NAME_LENGTH);
	sendPacket.writeBits(sizeof(stItemInfo)<<3, &item);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求出售灵兽栏灵兽上下架的消息
void CStallManager::HandleSellPetRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	S16	srcIndex	= Packet->readInt(16);	//出售灵兽栏索引位置
	S16 tagIndex	= Packet->readInt(16);	//出售物品栏索引位置
	bool isUpPet	= Packet->readFlag();	//是否上传灵兽
	S32 price		= Packet->readInt(32);	//灵兽价格

	stFixedStallPet pet;
	Packet->readBits(sizeof(stPetInfo)<<3, &pet);

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;
		}

		//判断商铺是否收摊修改状态
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2308;
			break;
		}

		if(isUpPet)	//灵兽上架
		{
			if(tagIndex != -1)
			{
				//判断是否操作索位位置越界
				if(tagIndex < 0 || tagIndex >= pStall->GetBuySellPets())
				{
					error = 3023;
					break;
				}
				stFixedStallPet* pTagPet = pStall->GetOneSellPet(tagIndex);
				if(pTagPet && pTagPet->petDataId != 0)
				{
					error = 3022;
					break;//目标位置已有灵兽
				}
			}
			else
			{
				tagIndex = pStall->FindEmptyPosFromSellPet();
				if(tagIndex == -1)
				{
					error = 3432;
					break;//商铺操作无效
				}
			}
			pet.StallID = stallId;
			pet.Pos = tagIndex;
			pet.Price = price;
			pStall->SetOneSellPet(&pet);
			pStall->SetSellPetVer(tagIndex);
			pStall->UpdateSMData(StallSMData::MASK_SELLPET, tagIndex);

			//记录日志:铺主上架出售灵兽栏灵兽成功
			GameplayLog::GL_ShopSetSellPet(PlayerId, LOG_SHOP_UPSELLPET_OK, &pet);
		}
		else
		{
			//判断是否操作索位位置越界
			if(srcIndex < 0 || srcIndex >= pStall->GetBuySellPets())
			{
				error = 3072;
				break;
			}
			stFixedStallPet* pSrcPet = pStall->GetOneSellPet(srcIndex);
			//判断位置上是否有灵兽
			if(!pSrcPet || pSrcPet->petDataId != pet.petDataId)
			{
				error = 3020;
				break;
			}
			pet = *pSrcPet;
			pSrcPet->InitData(stallId, pSrcPet->Pos);
			pStall->SetSellPetVer(srcIndex);
			pStall->UpdateSMData(StallSMData::MASK_SELLPET, srcIndex);

			//记录日志:铺主下架出售灵兽栏灵兽成功
			GameplayLog::GL_ShopSetSellPet(PlayerId, LOG_SHOP_DOWNSELLPET_OK, &pet);
		}
		
	}while (false);

	if(error != 0)
	{
		if(isUpPet)
		{
			//暂时未开放邮寄灵兽功能

			//记录日志:铺主上架出售灵兽栏灵兽失败
			GameplayLog::GL_ShopSetSellPet(PlayerId, LOG_SHOP_UPSELLPET_FAIL, &pet);
		}		
	}

	SendSellPetRespond(PlayerId, stallId, isUpPet, pet, price, srcIndex, tagIndex, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应处理商铺出售灵兽栏灵兽上下架消息
void CStallManager::SendSellPetRespond(U32 PlayerId, U16 stallId, bool isUpPet, stPetInfo& pet, 
							S32 price, S32 srcIndex, S32 tagIndex, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[1024];
	Base::BitStream sendPacket(buf,sizeof(buf));

	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SELLPET_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(srcIndex, 16);
	sendPacket.writeInt(tagIndex, 16);
	sendPacket.writeFlag(isUpPet);
	sendPacket.writeInt(price, 32);
	sendPacket.writeBits(sizeof(stPetInfo)<<3, &pet);
	sendPacket.writeInt(error, 32);

	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求收购物品栏上下架的消息
void CStallManager::HandleBuyItemRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	bool isUpItem	= Packet->readFlag();	//是否上架物品
	S32	price		= Packet->readInt(32);	//物品价格
	S16	num			= Packet->readInt(16);	//数量
	S16 index		= Packet->readInt(8);	//栏位索引位置
	char szPlayerName[MAX_NAME_LENGTH];
	Packet->readString(szPlayerName, MAX_NAME_LENGTH);

	stFixedStallItem item;
	Packet->readBits(sizeof(stItemInfo)<<3, &item);

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺已不在列表中
		}

		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主，无法修改操作
		}

		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2020;
			break;//请收摊再试
		}

		if(isUpItem)	//物品上架
		{
			if(pStall->FindSameItemInBuyItem(item.ItemID))
			{
				error = 2312;
				break;//该物品已经在收购栏中
			}

			index = pStall->FindEmptyPosFromBuyItem();
			if(index == -1)
			{
				error = 2312;
				break;//收购栏位已满
			}

			item.StallID = pStall->GetStallID();
			item.Pos = index;
			item.Price = price;
			item.BuyNum = num;
			item.LeftNum = num;
			pStall->SetOneBuyItem(&item);

			//记录日志:铺主上架出售收购栏物品成功
			GameplayLog::GL_ShopSetBuyItem(PlayerId, LOG_SHOP_UPBUYITEM_OK, &item);
		}
		else
		{
			//判断是否操作索位位置越界
			if(index < 0 || index >= MAX_FIXEDSTALL_BUYITEM)
			{
				error = 3072;
				break;
			}

			stFixedStallItem* pSrcItem = pStall->GetOneBuyItem(index);
			//判断位置上是否有物品
			if(!pSrcItem || pSrcItem->ItemID != item.ItemID)
			{
				error = 3020;
				break;
			}
			item = *pSrcItem;
			pSrcItem->InitData(stallId, pSrcItem->Pos);

			//记录日志:铺主下架出售收购栏物品成功
			GameplayLog::GL_ShopSetBuyItem(PlayerId, LOG_SHOP_DOWNBUYITEM_OK, &item);
		}		
	}while (false);

	if(error == 0)
	{
        pStall->UpdateSMData(StallSMData::MASK_BUYITEM,index);
		pStall->SetBuyItemVer(index);
	}
	else
	{
		if(isUpItem)
		{
			//记录日志:铺主上架出售收购栏物品失败
			GameplayLog::GL_ShopSetBuyItem(PlayerId, LOG_SHOP_UPBUYITEM_FAIL, &item);
		}
	}

	SendBuyItemRespond(PlayerId, szPlayerName, stallId, isUpItem, item, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应处理商铺收购物品栏物品上下架消息
void CStallManager::SendBuyItemRespond(U32 PlayerId, const char* PlayerName, U16 stallId, bool isUpItem, stFixedStallItem& item, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[1024];
	Base::BitStream sendPacket( buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_BUYITEM_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeFlag(isUpItem);
	sendPacket.writeBits(sizeof(stFixedStallItem)<<3, &item);
	sendPacket.writeInt(error, 32);
	sendPacket.writeString(PlayerName, MAX_NAME_LENGTH);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求喊话道具物品上下架的消息
void CStallManager::HandleSetShoutItemRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	bool isUpItem	= Packet->readFlag();	//是否上架物品
	U32	itemId		= Packet->readInt(32);	//喊话道具ID
	S32 num			= Packet->readInt(16);	//喊话道具数量
	S32 index		= Packet->readInt(16);	//包裹栏位索引位置
	char szPlayerName[MAX_NAME_LENGTH];
	Packet->readString(szPlayerName, MAX_NAME_LENGTH);

	int error = 0;
	U32 shoutItem = 0;
	S32 shoutNum = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;
		}

		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2308;
			break;
		}

		shoutItem = pStall->GetShoutItemId();
		shoutNum = pStall->GetShoutItemNum();
		if(isUpItem)	//物品上架
		{
			if(shoutItem != 0)
			{
				error = 3022;
				break;//喊话道具栏上已经有物品
			}
			if(itemId != SHOUTITEM0 && itemId != SHOUTITEM1)
			{
				error = 3023;
				break;//无法拖物品到目标位置上
			}

			pStall->SetShoutItemId(itemId);
			pStall->SetShoutItemNum(num);

			//记录日志:铺主上架喊话道具栏物品成功
			GameplayLog::GL_ShopSetShoutItem(PlayerId, LOG_SHOP_UPSHOUTITEM_OK, stallId, itemId, num);
		}
		else
		{
			num = shoutNum;
			pStall->SetShoutItemId(0);
			pStall->SetShoutItemNum(0);

			//记录日志:铺主下架喊话道具栏物品成功
			GameplayLog::GL_ShopSetShoutItem(PlayerId, LOG_SHOP_DOWNSHOUTITEM_OK, stallId, shoutItem, shoutNum);
		}		
	}while (false);

	if(error == 0)
	{
        pStall->UpdateSMData(StallSMData::MASK_INFO);
		pStall->SetInfoVer();
	}
	else
	{
		if(isUpItem)
		{
			char szMsg[1024] = {0};
			sprintf_s(szMsg,1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】喊话道具设置操作失败，系统退还您的物品,请及时收取.</t>",
				szPlayerName, stallId + 1);
			SERVER->GetMailManager()->sendMail(0, PlayerId, 0, itemId, num, "【商铺物品设置】", szMsg, 1);
			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );

			//记录日志:铺主上架喊话道具栏物品失败
			GameplayLog::GL_ShopSetShoutItem(PlayerId, LOG_SHOP_UPSHOUTITEM_FAIL, stallId, shoutItem, shoutNum);
		}
	}

	SendSetShoutItemRespond(PlayerId, szPlayerName, stallId, isUpItem, itemId, num, index, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应处理商铺喊话道具上下架消息
void CStallManager::SendSetShoutItemRespond(U32 PlayerId, const char* PlayerName, U16 stallId, bool isUpItem, U32 itemId,
							S32 num, S32 index, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[200];
	Base::BitStream sendPacket( buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SETSHOUTITEM_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeFlag(isUpItem);
	sendPacket.writeInt(itemId, 32);
	sendPacket.writeInt(num, 16);
	sendPacket.writeInt(index, 16);
	sendPacket.writeInt(error,32);
	sendPacket.writeString(PlayerName, MAX_NAME_LENGTH);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求玩家购买出售物品或灵兽，以及卖出物品给铺主的操作的消息
void CStallManager::HandlePlayerItemRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	U8	type		= Packet->readInt(8);	//购买物品/购买灵兽/卖出物品
	S16 index		= Packet->readInt(16);	//物品槽位索引值
	S16	num			= Packet->readInt(16);	//物品数量
	S32 money		= Packet->readInt(32);	//花费的金钱
	U32 checkId		= Packet->readInt(32);	//待查的目标物品ID
	S32 checkPrice	= Packet->readInt(32);	//待查的目标物品价格
	char szPlayerName[MAX_NAME_LENGTH];
	Packet->readString(szPlayerName, MAX_NAME_LENGTH);

	stItemInfo item;
	stPetInfo pet;
	S32 srcNum = 0, tagNum = 0, price = 0;

	int error = 0;
	StallHandle pStall = GetStall(stallId);
	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;
		}

		//当前铺主交易税率
		S32 TaxRate = pStall->GetTradeTax();
		if(TaxRate <= 0 || TaxRate >= 100)
			TaxRate = TRADETAX;

		//当前铺主流动资金
		S64 flowMoney = pStall->GetFlowMoney();

		//判断是否铺主自己
		if(pStall->GetStallerID() == 0 || pStall->GetStallerID() == PlayerId)
		{
			error = 3418;
			break;
		}

		if(pStall->GetStatus() != SS_OPEN)
		{
			error = 2308;
			break;
		}

		if(type == 1)		//购买物品
		{
			if(index < 0 || index >= pStall->GetBuySellItems())
			{
				error = 3072;
				break;//物品索引位置不正确
			}

			stFixedStallItem* pItem = pStall->GetOneSellItem(index);
			if(!pItem || pItem->Quantity < num)
			{
				error = 3072;
				break;//物品数量不够
			}

			if(pItem->ItemID != checkId || pItem->Price != checkPrice)
			{
				error = 3489;
				break;//物品槽位已发生改变，需重新确认买卖
			}

			//需要花费的金钱
			S64 iNeedMoney = pItem->Price * num; 
			if(iNeedMoney <= 0)
			{
				error = 580;
				break;;	//输入的金钱有误
			}

			if(iNeedMoney > MAX_MONEY_NUM)
			{
				error = 3406;
				break;	//金钱数目太大
			}

			//计算税钱
			S64 iTax = TaxRate * iNeedMoney / 100;
			if(iTax < 1)
				iTax = 1;
			S32 srcFlowMoney = flowMoney;
			flowMoney += (iNeedMoney - iTax);
			S32 tagFlowMoney = flowMoney;
			if(flowMoney < 0)
			{
				error = 3432;//商铺操作失败
				break;
			}
			
			if(flowMoney > MAX_MONEY_NUM)
			{
				error = 3438;
				break;//铺主金钱已达上限
			}

			memcpy(&item, pItem, sizeof(stItemInfo));
			price = pItem->Price;
			if(pItem->Quantity == num)
			{
				srcNum = 0;
				pItem->InitData(stallId, pItem->Pos);
			}
			else
			{
				pItem->Quantity -= num;
				srcNum = pItem->Quantity;
			}
			tagNum = num;
			pStall->SetFlowMoney(flowMoney);
			pStall->SetInfoVer();
			pStall->SetSellItemVer(index);
            pStall->UpdateSMData(StallSMData::MASK_INFO | StallSMData::MASK_SELLITEM,index);
			
			//记录商铺铺主交易记录
			pStall->AddRecordToList(0, iNeedMoney, iTax, szPlayerName, &item, num, NULL);

			//记录日志:商铺购买出售物品栏物品成功
			GameplayLog::GL_ShopBuySellItem(PlayerId, LOG_SHOP_BUYSELLITEM_OK, pStall->GetStallID(),
				pStall->GetStallerID(),	srcFlowMoney, tagFlowMoney, iNeedMoney, iTax, 
				TaxRate, &item, index, price, num);
		}
		else if(type == 2) //购买灵兽
		{
			if(index < 0 || index >= pStall->GetBuySellPets())
			{
				error = 3072;
				break;//灵兽槽位索引位置不正确
			}

			stFixedStallPet* pPet = pStall->GetOneSellPet(index);
			if(!pPet)
			{
				error = 3072;
				break;//灵兽数据不在
			}

			if(pPet->petDataId != checkId || pPet->Price != checkPrice)
			{
				error = 3489;
				break;//物品槽位已发生改变，需重新确认买卖
			}

			//需要花费的金钱
			S64 iNeedMoney = pPet->Price; 
			//计算税钱
			S64 iTax = TaxRate * iNeedMoney / 100;
			if(iTax < 1)
				iTax = 1;
			S32 srcFlowMoney = flowMoney;
			flowMoney += (iNeedMoney - iTax);
			S32 tagFlowMoney = flowMoney;
			if(flowMoney < 0)
			{
				error = 3432;//商铺操作失败
				break;
			}
			
			if(flowMoney > MAX_MONEY_NUM)
			{
				error = 3438;
				break;//金钱已达上限
			}

			memcpy(&pet, pPet, sizeof(stPetInfo));
			price = pPet->Price;
			pStall->SetFlowMoney(flowMoney);
			pStall->SetInfoVer();
			pPet->InitData(stallId, pPet->Pos);
			pStall->SetSellPetVer(index);
            pStall->UpdateSMData(StallSMData::MASK_INFO | StallSMData::MASK_SELLPET,index);

			//记录商铺铺主交易记录
		   pStall->AddRecordToList(1, iNeedMoney, iTax, szPlayerName, NULL, 0, &pet);
			//记录日志:商铺购买出售灵兽栏灵兽成功
			GameplayLog::GL_ShopBuySellPet(PlayerId, LOG_SHOP_BUYSELLPET_OK, pStall->GetStallID(),
				pStall->GetStallerID(),	srcFlowMoney, tagFlowMoney, iNeedMoney, iTax,
				TaxRate, &pet, index, price);
		}
		else
		{
			if(index < 0 || index >= MAX_FIXEDSTALL_BUYITEM)
			{
				error = 3072;
				break;//物品索引位置不正确
			}
			stFixedStallItem* pItem = pStall->GetOneBuyItem(index);
			if(!pItem || pItem->LeftNum < num)
			{
				error = 3072;
				break;//物品已经收购完成
			}

			if(pItem->ItemID != checkId || pItem->Price != checkPrice)
			{
				error = 3489;
				break;//物品槽位已发生改变，需重新确认买卖
			}

			S64 iNeedMoney = pItem->Price * num;
			if(iNeedMoney <= 0)
			{
				error = 580;
				break;//输入的金钱有误
			}

			if(iNeedMoney > MAX_MONEY_NUM)
			{
				error = 617;
				break;//金钱已达上限
			}

			if(flowMoney < iNeedMoney)
			{
				error = 3413;
				break;//无足够的流动资金来收购物品
			}

			S32 srcFlowMoney = flowMoney;
			flowMoney -= iNeedMoney;
			S32 tagFlowMoney = flowMoney;

			memcpy(&item, pItem, sizeof(stItemInfo));
			pItem->LeftNum -= num;
			srcNum = pItem->LeftNum;
			tagNum = num;
			price = pItem->Price;
			pStall->SetFlowMoney(flowMoney);
			pStall->SetInfoVer();
			pStall->SetBuyItemVer(index);
            pStall->UpdateSMData(StallSMData::MASK_INFO | StallSMData::MASK_BUYITEM,index);

			//记录商铺铺主交易记录
			pStall->AddRecordToList(2,-iNeedMoney, 0, szPlayerName, &item, num, NULL);
			//记录日志:商铺卖出收购物品栏物品成功
			GameplayLog::GL_ShopSellBuyItem(PlayerId, LOG_SHOP_SELLBUYITEM_OK,	pStall->GetStallID(),
				pStall->GetStallerID(), srcFlowMoney, tagFlowMoney, &item, index, srcNum+tagNum, srcNum, price);
		}
	}while (false);

	if(error != 0)
	{
		if(!pStall)
		{
			g_Log.WriteError("玩家向商铺买卖操作，未查询到有效的商铺对象!");
			SendPlayerItemRespond(PlayerId, szPlayerName, stallId, type, index, price, srcNum, tagNum, NULL, NULL, 0,0,error);
			return;
		}

		if(type == 1)		//购买物品操作失败，必须退还金元给玩家
		{
			char szMsg[1024] = {0};
			sprintf_s(szMsg,1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您向【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】购买物品失败，系统退还您的金元,请及时收取.</t>",
				szPlayerName, stallId + 1);
			SERVER->GetMailManager()->sendMail(0, PlayerId, money, 0, 0, "【商铺物品购买】", szMsg, 1);
			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );
			
			//记录日志:商铺购买出售物品栏物品失败
			GameplayLog::GL_ShopBuySellItem(PlayerId, LOG_SHOP_BUYSELLITEM_FAIL, stallId,
				pStall->GetStallerID(), 0, 0, 0, 0, 0, &item, index,price, num);
		}
		else if(type == 2)	//购买灵兽操作失败必须退还金元给玩家
		{
			char szMsg[1024] = {0};
			sprintf_s(szMsg,1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您向【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】购买灵兽失败，系统退还您的金元,请及时收取.</t>",
				szPlayerName, stallId + 1);
			SERVER->GetMailManager()->sendMail(0, PlayerId, money, 0, 0, "【商铺灵兽购买】", szMsg, 1);
			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );

			//记录日志:商铺购买出售灵兽栏灵兽失败
			GameplayLog::GL_ShopBuySellPet(PlayerId, LOG_SHOP_BUYSELLPET_FAIL, stallId,
				pStall->GetStallerID(), 0, 0, 0, 0,0, &pet, index, price);
		}
		else		//出售物品给铺主操作失败，必须退还物品给玩家
		{
			char szMsg[1024] = {0};
			sprintf_s(szMsg,1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
				"    您向【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】出售物品操作失败，系统退还您的物品,请收取.</t>",
				szPlayerName, stallId + 1);
			SERVER->GetMailManager()->sendMail(0, PlayerId, 0, item, "【商铺卖出物品】", szMsg, 1);
			//向客户端通知邮件更新
			SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );

			//记录日志:商铺卖出收购物品栏物品失败
			GameplayLog::GL_ShopSellBuyItem(PlayerId, LOG_SHOP_SELLBUYITEM_FAIL, stallId,
				pStall->GetStallerID(),	0, 0, &item, index, srcNum+tagNum, srcNum, price);
		}
		SendPlayerItemRespond(PlayerId, szPlayerName, stallId, type, index, price, srcNum, tagNum,
			&item, &pet, 0, 0, error);
	}
	else
	{
		SendPlayerItemRespond(PlayerId, szPlayerName, stallId, type, index, price, srcNum, tagNum,
			&item, &pet, pStall->GetFlowMoney(), pStall->GetTradeTax(), error);
	}	
}

// ----------------------------------------------------------------------------
// 向Zone回应处理玩家购买出售物品或灵兽，以及卖出物品给铺主的消息
void CStallManager::SendPlayerItemRespond(U32 PlayerId, const char* PlayerName, U16 stallId, U8 type, S32 index, S32 price, S16 srcNum,
						S16 tagNum, stItemInfo* pItem, stPetInfo* pPet, S32 flowmoney, S32 tax, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[1024];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_PLAYERITEM_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(type, 8);
	sendPacket.writeInt(index, 16);
	sendPacket.writeInt(price, 32);
	sendPacket.writeInt(flowmoney,32);
	sendPacket.writeInt(tax, 32);
	sendPacket.writeString(PlayerName, MAX_NAME_LENGTH);
	if(type == 1 || type == 3)
	{
		sendPacket.writeInt(srcNum, 16);
		sendPacket.writeInt(tagNum, 16);
		sendPacket.writeBits(sizeof(stItemInfo)<<3, pItem);
	}
	else
		sendPacket.writeBits(sizeof(stPetInfo)<<3, pPet);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理来自Zone请求铺主修改价格的消息
void CStallManager::HandleSetStallMoneyRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId	= pHead->Id;			//玩家ID
	U16 stallId		= Packet->readInt(16);	//玩家修改商铺ID
	U8	type		= Packet->readInt(8);	//操作类型
	S16 index		= Packet->readInt(16);	//目标位置
	S32 money		= Packet->readInt(32);	//玩家修改商铺物品或灵兽的价格

	int error = 0;
	StallHandle pStall = GetStall(stallId);

	do 
	{
		if(!pStall || stallId >= GetMaxStalls())
		{
			error = 3414;
			break;//商铺不在列表中
		}

		//判断是否铺主自己
		if(pStall->GetStallerID() != PlayerId)
		{
			error = 3431;
			break;//不是铺主,不能修改
		}

		//判断状态是否正确
		if(pStall->GetStatus() != SS_MODIFY)
		{
			error = 2302;
			break;//商铺应先收摊再修改
		}

		if(money <= 0)
		{
			error = 580;
			break;
		}
		
		if(money > MAX_MONEY_NUM)
		{
			error = 617;
			break;//输入的金钱超过了上限
		}

		if(type == 1)		//出售物品栏
		{
			if(index < 0 || index >= pStall->GetBuySellItems())
			{
				error = 3072;
				break;//物品索引位置不正确
			}

			stFixedStallItem* pItem = pStall->GetOneSellItem(index);
			if(!pItem || pItem->ItemID == 0)
			{
				error = 3020;
				break;//物品不存在
			}
			pItem->Price = money;
			pStall->SetSellItemVer(index);
            pStall->UpdateSMData(StallSMData::MASK_SELLITEM, index);
		}
		else if(type == 2)		//出售灵兽栏
		{
			if(index < 0 || index >= pStall->GetBuySellPets())
			{
				error = 3072;
				break;//物品索引位置不正确
			}
			stFixedStallPet* pPet = pStall->GetOneSellPet(index);
			if(!pPet || pPet->petDataId == 0)
			{
				error = 3020;
				break;//目标不存在
			}
			pPet->Price = money;
			pStall->SetSellPetVer(index);
            pStall->UpdateSMData(StallSMData::MASK_SELLPET, index);
		}
		else
		{
			if(index < 0 || index >= MAX_FIXEDSTALL_BUYITEM)
			{
				error = 3072;
				break;//物品索引位置不正确
			}
			stFixedStallItem* pItem = pStall->GetOneBuyItem(index);
			if(!pItem || pItem->LeftNum == 0)
			{
				error = 3020;
				break;//目标不存在
			}
			pItem->Price = money;
			pStall->SetBuyItemVer(index);
            pStall->UpdateSMData(StallSMData::MASK_BUYITEM, index);
		}
	}while (false);

	SendSetStallMoneyRespond(PlayerId, stallId, type, index, money, error);
}

// ----------------------------------------------------------------------------
// 向Zone回应设置商铺流动资金消息
void CStallManager::SendSetStallMoneyRespond(U32 PlayerId, U16 stallId,	U8 type, S32 index, S32 money, S32 error)
{
	int LineId = SERVER->GetPlayerManager()->GetPlayerLineId(PlayerId);
	int srcZoneId = SERVER->GetPlayerManager()->GetPlayerZoneId(PlayerId);
	int SocketHandle = SERVER->GetServerManager()->GetGateSocket( LineId, srcZoneId );

	char buf[128];
	Base::BitStream sendPacket( buf, sizeof(buf) );
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WZ_STALL_SETSTALLMONEY_RESPOND, PlayerId, SERVICE_ZONESERVER, srcZoneId);
	sendPacket.writeInt(stallId, 16);
	sendPacket.writeInt(type, 8);
	sendPacket.writeInt(index, 16);
	sendPacket.writeInt(money, 32);
	sendPacket.writeInt(error, 32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
}

// ----------------------------------------------------------------------------
// 处理商铺每日逻辑
void CStallManager::UpdateStall()
{
	U32 curtime;
	_time32((__time32_t*)&curtime);

	DO_LOCK( Lock::Stall );

	static U32 lastDay = 0;		

	STALLMAP::iterator it = m_stallMap.begin();
	for(; it != m_stallMap.end(); ++it)
	{
		StallHandle pStall = it->second;
		//检查摊位是否开放并且有摊主
		if(pStall && pStall->GetStallerID() != 0)
		{
			bool isUpdate = false;
			//处理全服喊话
			if(pStall->GetStatus() == SS_OPEN && pStall->GetIsShoutAll())
			{
				U32 ItemId  = pStall->GetShoutItemId();
				U32 ItemNum = pStall->GetShoutItemNum();
				if(ItemNum > 0 && curtime > pStall->GetShoutLastTime() + pStall->GetShoutInterval())
				{
					stChatMessage msg;
					msg.nSender = pStall->GetStallerID();
					msg.btMessageType = CHAT_MSG_TYPE_WORLD;
					msg.nRecver = 1;
					dStrcpy(msg.szMessage, _countof(msg.szMessage), pStall->GetShoutContent());
					dStrcpy(msg.szSenderName, 32, pStall->GetStallerName());
					gHelper.SendMessageToWorld(msg);
					pStall->SetShoutLastTime(curtime);
					pStall->SetShoutItemNum(ItemNum-1);
					if(pStall->GetShoutItemNum() == 0)
						pStall->SetShoutItemId(0);
					pStall->SetInfoVer();
					isUpdate = true;

					//记录日志：商铺定时喊话消耗道具
					GameplayLog::GL_ShopCostShoutItem(pStall->GetStallerID(), pStall->GetStallID(), ItemId, ItemNum);
				}				
			}

			//处理扣除每日税
			if(curtime - pStall->GetPayTaxTime() > 24*3600)
			{
				S32 money = pStall->GetFixedMoney();
				S32 srcFixedMoney = money;
				S32 DayTax = pStall->GetStallTax();
				if(money >= DayTax)
				{
					money -= DayTax;
					pStall->SetFixedMoney(money);
					pStall->SetPayTaxTime(getZeroTime(curtime));
					pStall->SetInfoVer();
					isUpdate = true;
					if(money < 7*DayTax)
					{
						S32 days = money / DayTax;
						char szMsg[1024] = {0};
						sprintf_s(szMsg, 1024, "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
							"    您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】的固定资金"\
							"已经不足以支付</t><t c='0xff0000ff'>%d天</t><t c='0xff9900ff'>的每日税收．请您及"\
							"时为商铺贮备足够的固定资金,否则系统会在无法交付的当日回收此商铺.</t><b/>"\
							"<t c='0xff9900ff'>    敬请关注!</t>",
							pStall->GetStallerName(), pStall->GetStallID() + 1, days);
						SERVER->GetMailManager()->sendMail(0, pStall->GetStallerID(), 0, 0, 0, "【商铺交税】", szMsg, 1);
						//向客户端通知邮件更新
						SERVER->GetPlayerManager()->SendSimplePacket(pStall->GetStallerID(),WORLD_CLIENT_MailNotify );
					}
					//记录日志:商铺每日扣税
					GameplayLog::GL_ShopDayTax(pStall->GetStallerID(), pStall->GetStallID(), DayTax, pStall->GetFlourish(),
						GetTotalFlourish(), srcFixedMoney, pStall->GetFixedMoney()); 
				}
				else
				{
					char szInfo[1024] = {0};
					struct tm today;
					_localtime32_s(&today, (__time32_t*)&curtime);
					sprintf_s(szInfo, sizeof(szInfo), "<t c='0xff9900ff'>亲爱的%s:</t><b/><t c='0xff9900ff'>"\
						"    因为您的【</t><t c='0xff0000ff'>%d号商铺</t><t c='0xff9900ff'>】的固定资金不足以支付当天的税收，系统于"\
						"%d-%02d-%02d回收此商铺．</t>", pStall->GetStallerName(), pStall->GetStallID() + 1,
						today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);

					S32 PlayerId = pStall->GetStallerID();
					pStall->InitInfo();

					//因为下面一定会全部更新，所以isUpdate没必要设为true
					isUpdate = false;

					//全部数据保存
                    pStall->UpdateSMData(StallSMData::MASK_ALL);
					
					SERVER->GetMailManager()->sendMail(0, PlayerId, 0, 0, 0, "【商铺回收】", szInfo, 1);
					//向客户端通知邮件更新
					SERVER->GetPlayerManager()->SendSimplePacket(PlayerId,WORLD_CLIENT_MailNotify );

					//记录日志:系统回收商铺
					GameplayLog::GL_ShopDumpStall(PlayerId, pStall->GetStallID());
				}
			}

			if(isUpdate)
			{
                pStall->UpdateSMData(StallSMData::MASK_INFO);
			}
		}
	}

	U32 today = getZeroTime(curtime);
	if(today != lastDay && m_stallMap.size() >= INITSTALLS)
	{
		S32 iTotalFlourish = 0;
		//首先要计算一次商业指数
		STALLMAP::iterator it = m_stallMap.begin();
		for(; it != m_stallMap.end(); ++it)
		{
			StallHandle pStall = it->second;
			//检查摊位是否开放并且有摊主
			if(pStall && pStall->GetStallID() < m_MaxStalls)
			{
				iTotalFlourish += pStall->GetFlourish();
			}
		}

		//更新商业指数
		SetTotalFlourish(iTotalFlourish);
		lastDay = today;
	}
}

// ----------------------------------------------------------------------------
// 处理来自客户端发送的留言信息
void CStallManager::HandleSendChatRequest(S32 SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId = pHead->Id;
	U16 stallId = Packet->readInt(16);
	StallHandle pStall = GetStall(stallId);
	if(pStall && pStall->GetStallerID() != PlayerId &&
			pStall->GetStatus() == SS_OPEN)
	{
		char szName[MAX_NAME_LENGTH];
		char szMsg[128];
		Packet->readString(szName, 32);
		Packet->readString(szMsg, 128);
		pStall->AddChatToList(szName, szMsg);
	}
}

// ----------------------------------------------------------------------------
// 处理来自客户端发送请求清除留言信息
void CStallManager::HandleSendClearChatRequest(S32 SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId = pHead->Id;
	U16 stallId = Packet->readInt(16);
	StallHandle pStall = GetStall(stallId);
	if(pStall && pStall->GetStallerID() == PlayerId)
	{
		pStall->ClearChat();
	}
}

// ----------------------------------------------------------------------------
// 处理来自客户端发送请求查询留言或交易信息
void CStallManager::HandleQueryChatRequest(S32 SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	DO_LOCK( Lock::Stall );	
	U32 PlayerId = pHead->Id;
	U16 stallId = Packet->readInt(16);
	U8	type = Packet->readInt(8);
	U32 timePos = Packet->readInt(32);
	StallHandle pStall = GetStall(stallId);
	if(pStall)
	{
		U32 findPos = 0;
		AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerId);
		if(NULL == pAccount)
		{
			g_Log.WriteWarn("为查询商铺玩家留言信息或铺主交易历史记录获取不到在线玩家数据!(PlayerId=%d)", PlayerId);
			return;
		}

		CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM );
		Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );

		if(type == 1)		//铺主交易历史记录
		{
			if(timePos != 0)
			{
				for(size_t i = 0; i < pStall->RecordList.size(); i++)
				{
					stStallRecord* pRecord = pStall->RecordList[i];
					if(pRecord && pRecord->time > timePos)
					{
						findPos = i;
						break;
					}
				}
			}

			size_t count = pStall->RecordList.size() - findPos;

			stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_STALL_QUERYCHAT_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
			sendPacket.writeInt(stallId, 16);
			sendPacket.writeInt(type, 8);
			sendPacket.writeInt(count, 32);
			for(size_t i = 0; i < count; i++)
			{
				stStallRecord* pRecord = pStall->RecordList[findPos];
				if(sendPacket.writeFlag(pRecord != NULL))
				{
					sendPacket.writeBits(sizeof(stStallRecord)<<3, pRecord);
					if(pRecord->type == 0 || pRecord->type == 2)
					{
						if(sendPacket.writeFlag(pRecord->trade.item != NULL))
							sendPacket.writeBits(sizeof(stItemInfo)<<3, pRecord->trade.item);
					}
					else
					{
						if(sendPacket.writeFlag(pRecord->trade.pet != NULL))
							sendPacket.writeBits(sizeof(stPetInfo)<<3, pRecord->trade.pet);
					}
				}
				findPos++;
			}
			pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
		}
		else				//玩家留言记录
		{
			if(timePos != 0)
			{
				for(size_t i = 0; i < pStall->ChatList.size(); i++)
				{
					stStallChat* pChat = pStall->ChatList[i];
					if(pChat && pChat->time > timePos)
					{
						findPos = i;
						break;
					}
				}
			}

			size_t count = pStall->ChatList.size() - findPos;
			
			stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WC_STALL_QUERYCHAT_RESPOND, pAccount->AccountId, SERVICE_CLIENT);
			sendPacket.writeInt(stallId, 16);
			sendPacket.writeInt(type, 8);
			sendPacket.writeInt(count, 32);
			for(size_t i = 0; i < count; i++)
			{
				if(sendPacket.writeFlag(pStall->ChatList[findPos] != NULL))
					sendPacket.writeBits(sizeof(stStallChat)<<3, pStall->ChatList[findPos]);
				findPos++;
			}
			pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );			
		}		
		
		SERVER->GetServerSocket()->Send( pAccount->GetGateSocketId(), sendPacket );
	}
}

//==============================================================================//
// 商铺的共享数据																//
//==============================================================================//
CStall::CStall()
{
	m_pSMData = NULL;
}

CStall::CStall(U16 id)
{
	m_pSMData = NULL;
	m_Info.StallID = id;
	InitInfo();
}

CStall::~CStall()
{
	if (NULL != m_pSMData)
	{
		SERVER->GetStallManager()->GetStallSMPool()->Free(m_pSMData);
		m_pSMData = NULL;
	}
	ClearChat();
	ClearRecord();
}

bool CStall::InitSM(U32 stallId)
{
	if (NULL != m_pSMData)
	{
		assert(NULL == m_pSMData);
		return false;
	}

	m_pSMData = SERVER->GetStallManager()->GetStallSMPool()->Alloc(stallId,0);
	if (NULL == m_pSMData)
	{
		assert(NULL != m_pSMData);
		return false;
	}
	
	m_pSMData->Attach(this);
	return true;
}

bool CStall::InitSM(SM_STALL_DATA* pSMData)
{
	if (NULL == pSMData)
		return false;

	if (NULL != m_pSMData)
	{
		assert(NULL == m_pSMData);
		return false;
	}

	m_pSMData = pSMData;
	m_pSMData->Attach(this);

	//从SM里面读取数据
	if (!m_pSMData->LoadFromSM())
		return false;
    
	return true;
}

void CStall::UpdateSMData(int mask,U32 posMask)
{
	if (NULL == m_pSMData)
	{
		g_Log.WriteError("商铺[%d]SM Data为空,无法保存到share memory", GetStallID());
		return;
	}
    
    //如果之前的数据已经被保存,则清除标记
    //如果标记计算不正确，应该只是数据标识被重复设置，导致
    //相应的操作被执行多次，应该不会引起问题
    if (m_pSMData->IsSaved())
    {
        this->ClearSaveMask();
    }

    m_dirtyMask |= mask;

    //需要记录修改了哪些元素
    if (mask & StallSMData::MASK_BUYITEM)
    {
        if(posMask >= MAX_FIXEDSTALL_BUYITEM)
        {
            assert(0);
		    return;
        }

        m_maskBuyItem[posMask] = true;
    }
    
    if (mask & StallSMData::MASK_SELLPET)
    {
        if(posMask > MAX_FIXEDSTALL_SELLPET)
        {
            assert(0);
		    return;
        }

        m_maskSellPet[posMask] = true;
    }
    
    if (mask & StallSMData::MASK_SELLITEM)
    {
        if(posMask > MAX_FIXEDSTALL_SELLITEM)
        {
            assert(0);
		    return;
        }

        m_maskSellItem[posMask] = true;
    }

    m_pSMData->Save(true,false);
}

void CStall::SetInfo(stFixedStall* pInfo)
{
	if(!pInfo)
		return;

	memcpy(&m_Info, pInfo, sizeof(stFixedStall));
	SetInfoVer();
}

void CStall::InitInfo()
{
	m_Info.Init(m_Info.StallID);
	for(int i = 0; i < MAX_FIXEDSTALL_SELLITEM; i++)
	{
		m_SellItem[i].InitData(m_Info.StallID, i);
	}
	for(int i = 0; i < MAX_FIXEDSTALL_SELLPET; i++)
	{
		m_SellPet[i].InitData(m_Info.StallID, i);
	}
	for(int i = 0; i < MAX_FIXEDSTALL_BUYITEM; i++)
	{
		m_BuyItem[i].InitData(m_Info.StallID, i);
	}
	ClearChat();
	ClearRecord();
}

void CStall::SetOneSellItem(stFixedStallItem* pItem)
{
	if(!pItem || pItem->Pos >= MAX_FIXEDSTALL_SELLITEM)
		return;
	memcpy(&m_SellItem[pItem->Pos], pItem, sizeof(stFixedStallItem));
}

void CStall::SetOneBuyItem(stFixedStallItem* pItem)
{
	if(!pItem || pItem->Pos >= MAX_FIXEDSTALL_BUYITEM)
		return;
	memcpy(&m_BuyItem[pItem->Pos], pItem, sizeof(stFixedStallItem));
}

void CStall::SetOneSellPet(stFixedStallPet* pPet)
{
	if(!pPet || pPet->Pos >= MAX_FIXEDSTALL_SELLPET)
		return;
	memcpy(&m_SellPet[pPet->Pos], pPet, sizeof(stFixedStallPet));
}

stFixedStall* CStall::GetInfo()
{
	return &m_Info;
}

stFixedStallItem* CStall::GetOneSellItem(U32 pos)
{
	return pos < MAX_FIXEDSTALL_SELLITEM ? &m_SellItem[pos] : NULL;
}

stFixedStallItem* CStall::GetOneBuyItem(U32 pos)
{
	return pos < MAX_FIXEDSTALL_BUYITEM ? &m_BuyItem[pos] : NULL;
}

stFixedStallPet* CStall::GetOneSellPet(U32 pos)
{
	return pos < MAX_FIXEDSTALL_SELLPET? &m_SellPet[pos] : NULL;
}

U32 CStall::GetSellItemVer(U32 pos)
{
	return pos < MAX_FIXEDSTALL_SELLITEM ? m_SellItem[pos].DataVersion : 0;
}

void CStall::SetSellItemVer(U32 pos)
{
	if(pos < MAX_FIXEDSTALL_SELLITEM) m_SellItem[pos].DataVersion++;
}

U32 CStall::GetBuyItemVer(U32 pos)
{
	return pos < MAX_FIXEDSTALL_BUYITEM ? m_BuyItem[pos].DataVersion : 0;
}

void CStall::SetBuyItemVer(U32 pos)
{
	if(pos < MAX_FIXEDSTALL_BUYITEM) m_BuyItem[pos].DataVersion++;
}

U32 CStall::GetSellPetVer(U32 pos)
{
	return pos < MAX_FIXEDSTALL_SELLPET ? m_SellPet[pos].DataVersion : 0;
}

void CStall::SetSellPetVer(U32 pos)
{
	if(pos < MAX_FIXEDSTALL_SELLPET) m_SellPet[pos].DataVersion++;
}

// ----------------------------------------------------------------------------
// 添加玩家留言信息(最多保存最近50条)
void CStall::AddChatToList(const char* sender, const char* content)
{
	stStallChat* pChat = NULL;
	if(ChatList.size() >= MAXLEAVEWORDS)
	{
		pChat = ChatList.front();
		ChatList.pop_front();
	}
	else
		pChat = new stStallChat;
	
	_time32((__time32_t*)&pChat->time);
	dStrcpy(pChat->sender, MAX_NAME_LENGTH, sender);
	dStrcpy(pChat->content, 128, content);
	ChatList.push_back(pChat);
}

// ----------------------------------------------------------------------------
// 清除商铺玩家留言记录
void CStall::ClearChat()
{
	for(size_t i = 0; i < ChatList.size(); i++)
	{
		if(ChatList[i])
			delete ChatList[i];
	}
	ChatList.clear();
}

// ----------------------------------------------------------------------------
// 添加玩家交易信息(最多保存最近200条)
void CStall::AddRecordToList(U32 type, S32 money, S32 tax, const char* target, stItemInfo* item, S32 num, stPetInfo* pet)
{
	stStallRecord* pRecord = NULL;
	if(RecordList.size() >= MAXTRADERECORDS)
	{
		pRecord = RecordList.front();
		RecordList.pop_front();
		if(pRecord->type == 0 || pRecord->type == 2)
			delete pRecord->trade.item;
		else
			delete pRecord->trade.pet;
	}
	else
	{
		pRecord = new stStallRecord;
	}

	_time32((__time32_t*)&pRecord->time);
	pRecord->type = type;
	pRecord->money = money;
	pRecord->tax = tax;
	dStrcpy(pRecord->targetName, MAX_NAME_LENGTH, target);
	if(type == 0 || type == 2)
	{
		pRecord->trade.item = new stItemInfo;
		memcpy(pRecord->trade.item,item, sizeof(stItemInfo));
		pRecord->trade.item->Quantity = num;
	}
	else
	{
		pRecord->trade.pet = new stPetInfo;
		memcpy(pRecord->trade.pet, pet, sizeof(stPetInfo));
	}
	
	RecordList.push_back(pRecord);
}

// ----------------------------------------------------------------------------
// 清空铺主交易记录
void CStall::ClearRecord()
{
	for(size_t i = 0; i < RecordList.size(); i++)
	{
		if(RecordList[i])
			delete RecordList[i];
	}
	RecordList.clear();
}

// ============================================================================
// 帮助函数
// ============================================================================
// ----------------------------------------------------------------------------
// 商铺收购物品栏是否存在指定的同种物品
bool CStall::FindSameItemInBuyItem(U32 itemId)
{
	for(int i = 0; i < MAX_FIXEDSTALL_BUYITEM; i++)
	{
		if(m_BuyItem[i].ItemID == itemId)
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
// 商铺出售物品栏内查找一个空槽位
S16 CStall::FindEmptyPosFromSellItem()
{
	for(U16 i = 0; i < GetBuySellItems(); i++)
	{
		if(m_SellItem[i].ItemID == 0)
			return i;
	}
	return -1;
}

// ----------------------------------------------------------------------------
// 商铺出售灵兽栏内查找一个空槽位
S16 CStall::FindEmptyPosFromSellPet()
{
	for(U16 i = 0; i < GetBuySellPets(); i++)
	{
		if(m_SellPet[i].petDataId == 0)
			return i;
	}
	return -1;
}

// ----------------------------------------------------------------------------
// 商铺收购物品栏内查找一个空槽位
S16 CStall::FindEmptyPosFromBuyItem()
{
	for(U16 i = 0; i < MAX_FIXEDSTALL_BUYITEM; i++)
	{
		if(m_BuyItem[i].ItemID == 0)
			return i;
	}
	return -1;
}
*/