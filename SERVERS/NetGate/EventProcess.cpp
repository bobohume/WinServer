#include "DBLib/dbLib.h"
#include "WinTCP/IPacket.h"
#include "WinTCP/dtServerSocket.h"
#include "Common/MemGuard.h"
#include "base/WorkQueue.h"
#include "Common/PacketType.h"
#include "Common/CommonClient.h"
#include "Common/MemoryShare.h"
#include "Common/DataCheck.h"
#include "Common/ChatBase.h"
#include "Common/Script.h"
#include "common/BillingBase.h"

#include "NetGate.h"
#include "EventProcess.h"
//#include "ServerMgr.h"
#include "PlayerMgr.h"

using namespace std;


void SERVER_CLASS_NAME::SendToDataAgent( WorkQueueItemStruct * pItem, stPacketHead* pHead )
{
}

//监控参数
stdext::hash_map<U32, U32> g_EventNetFNListTotalFreq;

int SERVER_CLASS_NAME::EventProcess(LPVOID Param)
{
	WorkQueueItemStruct *pItem = (WorkQueueItemStruct*)Param;

	switch(pItem->opCode)
	{
	case WQ_CONNECT:
		g_Log.WriteLog( "收到一个地图连接 Socket:%d IP:%s ",pItem->Id , pItem->Buffer );
		break;
	case WQ_DISCONNECT:
		{
			g_Log.WriteError( "disconnect zoneServer Socket:%d ZoneId:Unknow ",pItem->Id);
		}
		break;
	case WQ_PACKET:
		{
			stPacketHead* pHead = (stPacketHead*)pItem->Buffer;

            g_EventNetFNListTotalFreq[pHead->Message]++;
			Base::BitStream switchPacket(pItem->Buffer,pItem->size);
			switchPacket.setPosition(pItem->size);

			//SERVER->SendToDataAgent(pItem, pHead);
			//SERVER->GetDataAgentHelper()->Send( 0, SERVICE_GATESERVER, switchPacket );

			if(pHead->DestServerType == SERVICE_CLIENT)
				EventFn::SwitchSendToClient(pHead,switchPacket);
			/*else if(pHead->DestServerType == SERVICE_ZONESERVER && pHead->DestZoneId!=0)
				EventFn::SwitchSendToZone(pHead,switchPacket);*/
			else if(pHead->DestServerType == SERVICE_WORLDSERVER || pHead->DestServerType == SERVICE_REMOTESERVER)
				EventFn::SwitchSendToWorld(pHead,switchPacket);
			/*else if(pHead->DestServerType == SERVICE_DATAAGENT)
				EventFn::SwitchSendToDataAgent(pHead,switchPacket);*/
			else if(pHead->DestServerType == SERVICE_CHATSERVER)
				EventFn::SwitchSendToChat(pHead,switchPacket);
			else if(ISVALID_EVENT(pHead->Message))
			{
				char *pData	= (char *)(pHead) + IPacket::GetHeadSize();
				Base::BitStream RecvPacket(pData,pItem->size-IPacket::GetHeadSize());
				return !TRIGGER_EVENT(pItem->Id,pHead,RecvPacket);
				//return fnList[pHead->Message](pItem->Id,pHead,&RecvPacket);
			}
		}
		break;
	case  WQ_TIMER:
		{
		}
		break;
	}

	return false;
}

void DumpRecvPackets(void)
{
    for (stdext::hash_map<U32, U32>::iterator itr = g_EventNetFNListTotalFreq.begin(); itr != g_EventNetFNListTotalFreq.end(); ++itr)
    {
        if (0 == itr->second)
            continue;

        g_Log.WriteLog("收到服务端消息[%d] 次数[%d]",itr->first,itr->second);

        itr->second = 0;
    }
}

namespace EventFn
{
	void Initialize()
	{
		REGISTER_EVENT_FUNCTION("COMMON_RegisterRequest", &EventFn::HandleRegisterRequest);
		REGISTER_EVENT_FUNCTION("COMMON_RegisterResponse", &EventFn::HandleRegisterResponse);
		REGISTER_EVENT_FUNCTION("GATE_WORLD_ClientLoginResponse", &EventFn::HandleClientLoginResponse);
		REGISTER_EVENT_FUNCTION("GATE_WORLD_ClientReLoginResponse", &EventFn::HandleClientReLoginResponse);
		REGISTER_EVENT_FUNCTION("GATE_WORLD_ClientLogoutResponse", &EventFn::HandleClientLogoutResponse);
		REGISTER_EVENT_FUNCTION("SERVER_CHAT_SendMessageAll", &EventFn::HandleServerChatSendMessageAll);
		REGISTER_EVENT_FUNCTION("SERVER_CHAT_SendMessage", &EventFn::HandleServerChatSendMessage);
	}

	void SwitchSendToClient(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType)
	{
		int SocketId = SERVER->GetPlayerManager()->GetAccountSocket(pHead->Id);
		if(SocketId)
		{
			SERVER->GetUserSocket()->Send(SocketId,switchPacket,ctrlType);
		}
		else
		{
			g_Log.WriteWarn( "未能发现客户端[%d], msg:%d", pHead->Id, pHead->Message );
		}
	}

	void SwitchSendToZone(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType)
	{
		return;
		/*
		int ZoneSocketId = SERVER->GetServerManager()->GetZoneSocketId(pHead->DestZoneId);
		if(ZoneSocketId)
		{
			SERVER->GetServerSocket()->Send(ZoneSocketId,switchPacket,ctrlType);
		}
		else
		{
			g_Log.WriteError( "未能发现地图[%d] msg:%d, id:%d", pHead->DestZoneId, pHead->Message, pHead->Id );
		}*/
	}

	void SwitchSendToChat(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType)
	{
		SERVER->GetChatSocket()->Send(switchPacket,ctrlType);
	}

	void SwitchSendToWorld(stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType)
	{
		SERVER->GetWorldSocket()->Send(switchPacket,ctrlType);
	}

	void SwitchSendToDataAgent( stPacketHead* pHead, Base::BitStream &switchPacket,int ctrlType)
	{
		return;
		//SERVER->GetServerSocket()->Send(SERVER->GetServerManager()->GetDataAgentSocket(), switchPacket,ctrlType );
	}

	bool HandleClientLoginResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		int SocketId = SERVER->GetPlayerManager()->GetAccountSocket(pHead->Id);
		
		g_Log.WriteLog( "收到世界服务器登陆网关响应%d", SocketId );

		if(SocketId)
		{
			int Error = Packet->readInt(Base::Bit16);
			bool isGM = Packet->readFlag();

			if(isGM)
				SERVER->GetUserSocket()->SetConnectionType(SocketId, ISocket::SERVER_CONNECT);

			CMemGuard Buffer(64 MEM_GUARD_PARAM);
			Base::BitStream retPacket(Buffer.get(),64);
			stPacketHead *pSendHead = IPacket::BuildPacketHead(retPacket,"CLIENT_GATE_LoginResponse");
			retPacket.writeInt(Error,Base::Bit16);
			pSendHead->PacketSize = retPacket.getPosition()-IPacket::GetHeadSize();
			SERVER->GetUserSocket()->Send(SocketId,retPacket);
			return true;
		}

		return false;
	}

	bool HandleClientLogoutResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		int SocketId = SERVER->GetPlayerManager()->GetAccountSocket(pHead->Id);

		g_Log.WriteLog( "收到世界服务器登陆网关响应%d", SocketId );

		if(SocketId)
		{
			CMemGuard Buffer(64 MEM_GUARD_PARAM);
			Base::BitStream retPacket(Buffer.get(),64);
			stPacketHead *pSendHead = IPacket::BuildPacketHead(retPacket,"CLIENT_GATE_LogoutResponse");
			pSendHead->PacketSize = retPacket.getPosition()-IPacket::GetHeadSize();
			SERVER->GetUserSocket()->Send(SocketId,retPacket);
			//重启这个连接
			SERVER->GetPlayerManager()->ReleaseSocketMap(SocketId, true);
			return true;
		}

		return false;
	}

	bool HandleClientReLoginResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		int SocketId = SERVER->GetPlayerManager()->GetAccountSocket(pHead->Id);

		g_Log.WriteLog( "收到世界服务器登陆网关响应%d", SocketId );

		if(SocketId)
		{
			int Error = Packet->readInt(Base::Bit16);

			CMemGuard Buffer(64 MEM_GUARD_PARAM);
			Base::BitStream retPacket(Buffer.get(),64);
			stPacketHead *pSendHead = IPacket::BuildPacketHead(retPacket,"CLIENT_GATE_ReLoginResponse");
			retPacket.writeInt(Error,Base::Bit16);
			pSendHead->PacketSize = retPacket.getPosition()-IPacket::GetHeadSize();
			SERVER->GetUserSocket()->Send(SocketId,retPacket);
			return true;
		}

		return false;
	}

	bool HandleRegisterRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		return true;
		/*stServerInfo ServerInfo;
		ServerInfo.SocketId			= SocketHandle;
		int ServerType = Packet->readInt(Base::Bit8);

		if( ServerType == SERVICE_DATAAGENT )
		{
			CMemGuard Buffer(64 MEM_GUARD_PARAM);
			Base::BitStream retPacket(Buffer.get(),64);
			stPacketHead *pSendHead = IPacket::BuildPacketHead(retPacket,COMMON_RegisterResponse);
			pSendHead->PacketSize = retPacket.getPosition() - sizeof( stPacketHead );
			SERVER->GetServerSocket()->Send( SocketHandle, retPacket );
			SERVER->GetServerManager()->SetDataAgentSocket( SocketHandle );
			return false;
		}

		if( ServerType != SERVICE_ZONESERVER)
			return false;
		Packet->readInt(Base::Bit8);	//无用id
		ServerInfo.ZoneId			= Packet->readInt(Base::Bit32);

		stZoneInfo Info;

		char Buffer[256];
		Base::BitStream retPacket(Buffer,sizeof(Buffer));

		stPacketHead *pSendHead = IPacket::BuildPacketHead(retPacket,COMMON_RegisterResponse );

		if(SERVER->GetServerManager()->GetZoneInfo(ServerInfo.ZoneId,Info))
		{
			ServerInfo.ConnectSeq		= Packet->readInt(Base::Bit32);
			if(Packet->readFlag())
			{
				ServerInfo.Ip			= Packet->readInt(Base::Bit32);
				Packet->readInt(Base::Bit16);					//ZoneServer发上来的服务端口不使用
				ServerInfo.Port			= Info.ServicePort;		//使用数据库里设置的端口来启动服务
			}
			SERVER->GetServerManager()->AddServerMap(ServerInfo);

			g_Log.WriteLog( "ZoneServer incoming Socket:%d ZoneId:%d ",SocketHandle, ServerInfo.ZoneId);

			//反馈给地图服务器

			retPacket.writeInt(SERVER->LineId,Base::Bit16);
			retPacket.writeInt(SERVER->GateId,Base::Bit16);

			retPacket.writeInt(Info.MaxPlayerNum,Base::Bit16);
			retPacket.writeInt(Info.ServicePort	,Base::Bit16);
			retPacket.writeInt(Info.Sequence, Base::Bit16);
            
            std::string arenaOpenTime;
            int openDay = 0;
            int closeDay = 0;
            SERVER->GetServerManager()->GetArenaInfo(arenaOpenTime,openDay,closeDay);

            retPacket.writeString(arenaOpenTime.c_str());
            retPacket.writeInt(openDay,Base::Bit32);
            retPacket.writeInt(closeDay,Base::Bit32);
			retPacket.writeInt(SERVER->GetServerManager()->GetYears5Assure(), Base::Bit8);
			retPacket.writeInt(SERVER->GetServerManager()->GetGodGivePath(), Base::Bit8);

            pSendHead->PacketSize = retPacket.getPosition()-IPacket::GetHeadSize();
			SERVER->GetServerSocket()->Send(SocketHandle,retPacket);

			//发送消息到世界服务器
			Base::BitStream wldPacket(Buffer,sizeof(Buffer));
			pSendHead = IPacket::BuildPacketHead(wldPacket,GATE_WORLD_ZoneRegisterRequest);
			wldPacket.writeInt(1,Base::Bit8);
			wldPacket.writeInt(ServerInfo.ZoneId,Base::Bit32);
			wldPacket.writeInt(ServerInfo.Ip,Base::Bit32);
			wldPacket.writeInt(ServerInfo.Port,Base::Bit16);
			wldPacket.writeInt(ServerInfo.ConnectSeq,Base::Bit32);
			wldPacket.writeInt(SERVER->LineId,Base::Bit16);
			pSendHead->PacketSize = wldPacket.getPosition()-IPacket::GetHeadSize();
			SERVER->GetWorldSocket()->Send(wldPacket,PT_GUARANTEED|PT_SYS);
		}
		else
		{

			pSendHead->DestZoneId = -1;
			pSendHead->PacketSize = retPacket.getPosition() - sizeof( stPacketHead );
			SERVER->GetServerSocket()->Send( SocketHandle, retPacket );
			
			g_Log.WriteError( "ZoneServer register Fail！ Socket:%d ZoneId:%d ",SocketHandle, ServerInfo.ZoneId);
		}

		return true;*/
	}

	bool HandleRegisterResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		if( SocketHandle == -1 )
		{
			//来自WorldServer对自己注册的反馈
			//SERVER->GetServerManager()->UnpackZoneData(Packet);
			SERVER->OnServerStart();

			//发送消息到世界服务器，同步一下已有的所有地图管理
			/*CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
			Base::BitStream wldPacket(Buffer.get(),MAX_PACKET_SIZE);
			stPacketHead *pSendHead = IPacket::BuildPacketHead(wldPacket,GATE_WORLD_ZoneRegisterRequest);
			SERVER->GetServerManager()->PackZoneMgrList(wldPacket);
			pSendHead->PacketSize = wldPacket.getPosition()-IPacket::GetHeadSize();
			SERVER->GetWorldSocket()->Send(wldPacket,PT_GUARANTEED|PT_SYS);*/
		}

		if( SocketHandle == -2 )
		{
			// 来自ChatServer对自己的注册反馈
		}

		//{
		//	if(SERVER->GetServerManager()->IsNoGoods())
		//	{
		//		//若注册成功,再向World请求发送商城商品数据
		//		CMemGuard Buffer(255 MEM_GUARD_PARAM);
		//		Base::BitStream wldPacket(Buffer.get(), 255);
		//		stPacketHead *pSendHead = IPacket::BuildPacketHead(wldPacket,GW_BILLING_REQUEST);
		//		wldPacket.writeInt(0, 8);
		//		pSendHead->PacketSize = wldPacket.getPosition()-IPacket::GetHeadSize();
		//		SERVER->GetWorldSocket()->Send(wldPacket);
		//	}

		//	if(SERVER->GetServerManager()->IsNoDiscount())
		//	{
		//		//若注册成功,再向World请求发送商城折扣配置数据
		//		CMemGuard Buffer(255 MEM_GUARD_PARAM);
		//		Base::BitStream wldPacket(Buffer.get(), 255);
		//		stPacketHead *pSendHead = IPacket::BuildPacketHead(wldPacket,GW_BILLING_REQUEST);
		//		wldPacket.writeInt(1, 8);
		//		pSendHead->PacketSize = wldPacket.getPosition()-IPacket::GetHeadSize();
		//		SERVER->GetWorldSocket()->Send(wldPacket);
		//	}

		//	if(SERVER->GetServerManager()->IsNoRecommend())
		//	{
		//		//若注册成功,再向World请求发送商城推荐配置数据
		//		CMemGuard Buffer(255 MEM_GUARD_PARAM);
		//		Base::BitStream wldPacket(Buffer.get(), 255);
		//		stPacketHead *pSendHead = IPacket::BuildPacketHead(wldPacket,GW_BILLING_REQUEST);
		//		wldPacket.writeInt(2, 8);
		//		pSendHead->PacketSize = wldPacket.getPosition()-IPacket::GetHeadSize();
		//		SERVER->GetWorldSocket()->Send(wldPacket);
		//	}
		//}

		return true;
	}

	//// ----------------------------------------------------------------------------
	//// 处理来自World回应发送商城数据
	//bool HandleWGBillingResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	//{
	//	int DataType = Packet->readInt(8);
	//	if(DataType == 0)			//商品数据
	//	{
	//		int count = Packet->readInt(Base::Bit16);
	//		for(int i = 0; i < count; i++)
	//		{
	//			stGoods* goods = new stGoods;
	//			Packet->readBits(sizeof(stGoods)<<3, goods);
	//			SERVER->GetServerManager()->AddGoods(goods);
	//		}
	//	}
	//	else if (DataType == 1)		//折扣配置数据
	//	{
	//		int count = Packet->readInt(Base::Bit16);
	//		for(int i = 0; i < count; i++)
	//		{
	//			stDiscount* discount = new stDiscount;
	//			Packet->readBits(sizeof(stDiscount)<<3, discount);
	//			SERVER->GetServerManager()->AddDiscount(discount);
	//		}			
	//	}
	//	else						//推荐配置数据
	//	{
	//		int count = Packet->readInt(Base::Bit16);
	//		for(int i = 0; i < count; i++)
	//		{
	//			stRecommend* recommend = new stRecommend;
	//			Packet->readBits(sizeof(stRecommend)<<3, recommend);
	//			SERVER->GetServerManager()->AddRecommend(recommend);
	//		}
	//	}
	//	return true;
	//}

	//bool HandleZGBillingRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	//{
	//	int DataType = Packet->readInt(8);
	//	SERVER->GetServerManager()->SendGZBillingResponse(SocketHandle, DataType);
	//	return true;
	//}

	bool HandleServerChatSendMessageAll(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		OLD_DO_LOCK( SERVER->GetPlayerManager()->m_cs );

		stChatMessage chatMessage;
		stChatMessage::UnpackMessage( chatMessage, *Packet );

		CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream SendPacket( buf.get(), MAX_PACKET_SIZE );
		stPacketHead* pSendHead = IPacket::BuildPacketHead( SendPacket, "SERVER_CHAT_SendMessage_NOTIFY" );
		pSendHead->DestServerType = SERVICE_CLIENT;
		pSendHead->Id = 0;
		stChatMessage::PackMessage1( chatMessage, SendPacket );
		pSendHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetPlayerManager()->Broadcast(SendPacket,PT_DROP);

		g_Log.WriteWarn("全线广播大小为%d聊天包",SendPacket.getPosition());
		return false;
	}

	bool HandleServerChatSendMessage(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		OLD_DO_LOCK( SERVER->GetPlayerManager()->m_cs );
		
		std::list<int> playerList;

		unsigned short nSize = (unsigned short)Packet->readInt( Base::Bit16 );
		for( int i = 0; i < nSize; i++ )
		{
			playerList.push_back( (int)Packet->readInt( Base::Bit32 ) );
		}
		stChatMessage chatMessage;
		stChatMessage::UnpackMessage( chatMessage, *Packet );

		CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream SendPacket( buf.get(), MAX_PACKET_SIZE );
		stPacketHead* pSendHead = IPacket::BuildPacketHead( SendPacket, "SERVER_CHAT_SendMessage", 0, SERVICE_CLIENT );

		stChatMessage::PackMessage( chatMessage, SendPacket );
		pSendHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );

		std::list<int>::iterator it;
		int nSocket;
		for( it = playerList.begin(); it != playerList.end(); it++ )
		{
			nSocket = SERVER->GetPlayerManager()->GetAccountSocket( *it );
			
			if( nSocket != 0 )
				SERVER->GetUserSocket()->Send( nSocket, SendPacket ,PT_DROP);
		}

		g_Log.WriteWarn("对%d个玩家发送大小为%d聊天包,这次发送总计%d字节",nSize,SendPacket.getPosition(),nSize*SendPacket.getPosition());

		return false;
	}
}