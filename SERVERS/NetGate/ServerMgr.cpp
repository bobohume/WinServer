
#include "DBLib/dbLib.h"
#include "Base/Log.h"
#include "wintcp/dtServerSocket.h"
#include "Common/PacketType.h"
#include "ServerMgr.h"
#include "NetGate.h"
#include "common/BillingBase.h"

using namespace std;

CServerManager::CServerManager()
{
	mDataAgentSocket    = 0;
    m_arenaStartTime[0] = 0;
    m_openDay           = 0;
    m_closeDay          = 0;
}

CServerManager::~CServerManager()
{
	//ClearGoods();
	//ClearDiscount();
	//ClearRecommend();
}

void CServerManager::AddServerMap(stServerInfo &ServerInfo)
{
	CLocker lock(m_cs);

	HashSocketMap::iterator sit = m_SocketMap.find(ServerInfo.SocketId);
	if(sit != m_SocketMap.end())
	{
		ReleaseServerMap( sit->second->SocketId, true );
	}

	stServerInfo *pServerInfo = new stServerInfo;
	*pServerInfo	=	ServerInfo;

	m_SocketMap[ServerInfo.SocketId]	= pServerInfo;
	m_ZoneMap[ServerInfo.ZoneId]		= pServerInfo;
}

stServerInfo* CServerManager::GetZoneInfo( int socketId )
{
	HashSocketMap::iterator sit = m_SocketMap.find(socketId);
	if(sit != m_SocketMap.end())	
		return sit->second;

	return NULL;
}

void CServerManager::ReleaseServerMap(int SocketId,bool postEvent)
{
	CLocker lock(m_cs);

	HashSocketMap::iterator sit = m_SocketMap.find(SocketId);
	if(sit == m_SocketMap.end())
		return;

	stServerInfo* pServerInfo = sit->second;

    stServerInfo* pOldConnectInfo = 0;

    HashZoneMap::iterator iter = m_ZoneMap.find(pServerInfo->ZoneId);

    if (iter != m_ZoneMap.end())
    {
        pOldConnectInfo = iter->second;
    }

    if (pOldConnectInfo && pOldConnectInfo->SocketId != SocketId)
    {
        g_Log.WriteLog("socket id missmatched id[%d]",pOldConnectInfo->SocketId);
    }
    else
    {
        m_ZoneMap.erase(pServerInfo->ZoneId);
    }

    m_SocketMap.erase(pServerInfo->SocketId);
    delete pServerInfo;

    if(postEvent)
        SERVER->GetServerSocket()->PostEvent(dtServerSocket::OP_RESTART,SocketId);
}

int CServerManager::GetZoneSocketId(int ZoneId)
{
	CLocker lock(m_cs);

	HashZoneMap::iterator slit = m_ZoneMap.find(ZoneId);
	if(slit == m_ZoneMap.end())
		return 0;

	stServerInfo *pServerInfo = slit->second;
	return pServerInfo->SocketId;
}

stServerInfo *CServerManager::GetServerInfo(int ZoneId)
{
	HashZoneMap::iterator slit = m_ZoneMap.find(ZoneId);
	if(slit == m_ZoneMap.end())
		return NULL;

	return slit->second;
}

bool CServerManager::Initialize()
{
	return true;
}

void CServerManager::UnpackZoneData(Base::BitStream *pPacket)
{
	CLocker lock(m_cs);

	stZoneInfo ZoneInfo;
	int Num = pPacket->readInt(Base::Bit16);
	for(int i=0;i<Num;i++)
	{
		ZoneInfo.ZoneId			= pPacket->readInt(Base::Bit32);
		ZoneInfo.MaxPlayerNum	= pPacket->readInt(Base::Bit16);
		ZoneInfo.ServicePort	= pPacket->readInt(Base::Bit16);
		ZoneInfo.Sequence		= pPacket->readInt(Base::Bit32);
		m_ZoneInfoList[ZoneInfo.ZoneId] = ZoneInfo;
	}
    
    pPacket->readString(m_arenaStartTime,sizeof(m_arenaStartTime));
    m_openDay  = pPacket->readInt(Base::Bit32);
    m_closeDay = pPacket->readInt(Base::Bit32);
	m_Years5Assure = pPacket->readInt(Base::Bit8);
	m_GodGivePath = pPacket->readInt(Base::Bit8);
}

bool CServerManager::GetZoneInfo(int ZoneId,stZoneInfo &Info)
{
	CLocker lock(m_cs);

	HashZoneInfo::iterator it = m_ZoneInfoList.find(ZoneId);
	if(it == m_ZoneInfoList.end())
		return false;

	Info = it->second;
	return true;
}

void CServerManager::GetArenaInfo(std::string& startTime,int& openDay,int& closeDay)
{
    CLocker lock(m_cs);

    startTime = m_arenaStartTime;
    openDay   = m_openDay;
    closeDay  = m_closeDay;
}

void CServerManager::PackZoneMgrList(Base::BitStream &Packet)
{
	CLocker lock(m_cs);

	Packet.writeInt((int)m_ZoneMap.size(),Base::Bit8);
	HashZoneMap::iterator zit = m_ZoneMap.begin();
	while(zit!=m_ZoneMap.end())
	{
		Packet.writeInt(zit->first,Base::Bit32);
		Packet.writeInt(zit->second->Ip,Base::Bit32);
		Packet.writeInt(zit->second->Port,Base::Bit16);
		Packet.writeInt(zit->second->ConnectSeq,Base::Bit32);
		Packet.writeInt(SERVER->LineId,Base::Bit16);
		zit++;
	}
}

int CServerManager::GetDataAgentSocket()
{
	CLocker lock(m_cs);

	return mDataAgentSocket;
}

void CServerManager::SetDataAgentSocket( int socketId )
{
	CLocker lock(m_cs);

	mDataAgentSocket = socketId;
}
//
//void CServerManager::AddDiscount(stDiscount* discount)
//{
//	if(discount)
//	{
//		mDiscountList.push_back(discount);
//	}
//}
//void CServerManager::ClearDiscount()
//{
//	for each(stDiscount* it in mDiscountList)
//	{
//		if(it) delete it;
//	}
//	mDiscountList.clear();
//}
//
//void CServerManager::AddGoods(stGoods* goods)
//{
//	if(goods)
//	{
//		mGoodsList.push_back(goods);
//	}
//}
//void CServerManager::ClearGoods()
//{
//	for each(stGoods* it in mGoodsList)
//	{
//		if(it) delete it;
//	}
//	mGoodsList.clear();
//}
//
//void CServerManager::AddRecommend(stRecommend* recommend)
//{
//	if(recommend)
//	{
//		mRecommendList.push_back(recommend);
//	}
//}
//void CServerManager::ClearRecommend()
//{
//	for each(stRecommend* it in mRecommendList)
//	{
//		if(it) delete it;
//	}
//	mRecommendList.clear();
//}
//
//// ----------------------------------------------------------------------------
//// 向Zone回应发送商城数据
//void CServerManager::SendGZBillingResponse(int SocketHandle, int type)
//{
//	if(DataType == 0)			//商品数据
//	{
//		const U32 MAX_BLOCK = 512;
//		size_t counts = mGoodsList.size() / MAX_BLOCK;
//		if(mGoodsList.size() % MAX_BLOCK != 0)
//			counts ++;
//
//		CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM );
//		Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );
//		for(size_t i = 0; i < counts; i++)
//		{
//			sendPacket.setPosition(0);
//			stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WG_BILLING_RESPOND, 0, SERVICE_ZONESERVER);
//			sendPacket.writeInt(DataType, 8);
//			size_t count = MAX_BLOCK;
//			if( i == counts - 1)
//				count = mGoodsList.size() - i * MAX_BLOCK;
//
//			sendPacket.writeInt(count, Base::Bit16);
//			for(size_t j = i * MAX_BLOCK; j < (i * MAX_BLOCK + count); j++)
//			{
//				sendPacket.writeBits(sizeof(stGoods)<<3, mGoodsList[j]);
//			}
//			pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
//			SERVER->GetServerSocket()->Send( SocketHandle, sendPacket);
//		}
//	}
//	else if (DataType == 1)		//折扣配置数据
//	{
//		const U32 MAX_BLOCK = 1024;
//		size_t counts = mDiscountList.size() / MAX_BLOCK;
//		if(mDiscountList.size() % MAX_BLOCK != 0)
//			counts ++;
//
//		CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM );
//		Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );
//		for(size_t i = 0; i < counts; i++)
//		{
//			sendPacket.setPosition(0);
//			stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WG_BILLING_RESPOND, 0, SERVICE_ZONESERVER);
//			sendPacket.writeInt(DataType, 8);
//			size_t count = MAX_BLOCK;
//			if( i == counts - 1)
//				count = mDiscountList.size() - i * MAX_BLOCK;
//
//			sendPacket.writeInt(count, Base::Bit16);
//			for(size_t j = i * MAX_BLOCK; j < (i * MAX_BLOCK + count); j++)
//			{
//				sendPacket.writeBits(sizeof(stDiscount)<<3, mDiscountList[j]);
//			}
//			pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
//			SERVER->GetServerSocket()->Send( SocketHandle, sendPacket );
//		}
//	}
//	else						//推荐配置数据
//	{
//		const U32 MAX_BLOCK = 1024;
//		size_t counts = mRecommendList.size() / MAX_BLOCK;
//		if(mRecommendList.size() % MAX_BLOCK != 0)
//			counts ++;
//
//		CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM );
//		Base::BitStream sendPacket( buf.get(), MAX_PACKET_SIZE );
//		for(size_t i = 0; i < counts; i++)
//		{
//			sendPacket.setPosition(0);
//			stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WG_BILLING_RESPOND, 0, SERVICE_ZONESERVER);
//			sendPacket.writeInt(DataType, 8);
//			size_t count = MAX_BLOCK;
//			if( i == counts - 1)
//				count = mRecommendList.size() - i * MAX_BLOCK;
//
//			sendPacket.writeInt(count, Base::Bit16);
//			for(size_t j = i * MAX_BLOCK; j < (i * MAX_BLOCK + count); j++)
//			{
//				sendPacket.writeBits(sizeof(stRecommend)<<3, mRecommendList[j]);
//			}
//			pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
//			SERVER->GetServerSocket()->Send( SocketHandle, sendPacket );
//		}
//	}
//}