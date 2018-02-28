
#include "Common/MemGuard.h"
#include "worldserver.h"
#include "wintcp/AsyncSocket.h"
#include "ServerMgr.h"
#include "ChatHelper.h"
#include "AccountHandler.h"
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"

CChatHelper gHelper;

CChatHelper::CChatHelper()
{
	registerEvent( "WORLD_CHAT_AutoMsgResponse", &CChatHelper::HandleAutoMsgResponse );
	registerEvent( "WORLD_CHAT_RegisterChannelResponse", &CChatHelper::HandleRegisterChannelResponse);
}

CChatHelper::~CChatHelper()
{

}

bool CChatHelper::HandleAutoMsgResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
{
	AutoMsg msg;
	msg.ReadData(Packet);
	return true;
}

bool CChatHelper::HandleRegisterChannelResponse(int SocketHandle, stPacketHead *pHead, Base::BitStream *Packet)
{
	RegisterChannelCallback	callback = (RegisterChannelCallback)pHead->Id;
	int channelId = pHead->DestZoneId;
	void* param = (void*)pHead->SrcZoneId;
	int type = pHead->LineServerId;

	if (callback)
		callback(type, channelId, param);

	return true;
}
//************************************
// Method:    clearAccount
// FullName:  CChatHelper::clearAccount
// Access:    public 
// Returns:   void
// Qualifier: 把该帐号下的所有角色排出聊天系统
// Parameter: int accountId
//************************************
void CChatHelper::clearAccount( int accountId )
{
/*	MAKE_PACKET( sendPacket, WORLD_CHAT_ClearAccount, accountId, SERVICE_CHATSERVER );

	int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

	if(svrSocket > 0)
	{
		SEND_PACKET( sendPacket, svrSocket );
	}*/
}

//************************************
// Method:    addPlayer
// FullName:  CChatHelper::addPlayer
// Access:    public 
// Returns:   void
// Qualifier: 把玩家加入到制定频道
// Parameter: int channelId
// Parameter: int playerId
//************************************
void CChatHelper::addPlayer( int channelId, int playerId )
{
	MAKE_PACKET( sendPacket, "WORLD_CHAT_ChannelAddPlayer", playerId, SERVICE_CHATSERVER, channelId );

	int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

	if(svrSocket > 0)
	{
		SEND_PACKET( sendPacket, svrSocket );
	}
}

//************************************
// Method:    removePlayer
// FullName:  CChatHelper::removePlayer
// Access:    public 
// Returns:   void
// Qualifier: 把玩家从制定频道移除
// Parameter: int channelId
// Parameter: int playerId
//************************************
void CChatHelper::removePlayer( int channelId, int playerId )
{
	MAKE_PACKET( sendPacket, "WORLD_CHAT_ChannelRemovePlayer", playerId, SERVICE_CHATSERVER, channelId );

	int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

	if(svrSocket > 0)
	{
		SEND_PACKET( sendPacket, svrSocket );
	}
}

//************************************
// Method:    registerChannel
// FullName:  CChatHelper::registerChannel
// Access:    public 
// Returns:   void
// Qualifier: 注册一个频道
// Parameter: int type [类型]
// Parameter: RegisterChannelCallback callback [回调函数]
// Parameter: void * param [自定义参数]
//************************************
void CChatHelper::registerChannel( int type, RegisterChannelCallback callback, void* param )
{
	MAKE_PACKET( sendPacket, "WORLD_CHAT_RegisterChannelRequest", type, SERVICE_CHATSERVER, (int)param, (int)callback );

	CServerManager* pServerMgr = SERVER->GetServerManager();
	if (pServerMgr)
	{
		int svrSocket = pServerMgr->GetServerSocket( SERVICE_CHATSERVER );

		if(svrSocket > 0)
		{
			SEND_PACKET( sendPacket, svrSocket );
		}
	}
}

//************************************
// Method:    unregisterChannel
// FullName:  CChatHelper::unregisterChannel
// Access:    public 
// Returns:   void
// Qualifier: 删除一个频道
// Parameter: int channelId
//************************************
void CChatHelper::unregisterChannel( int channelId )
{
	MAKE_PACKET( sendPacket, "WORLD_CHAT_UnregisterChannelRequest", channelId, SERVICE_CHATSERVER );

	CServerManager* pServerMgr = SERVER->GetServerManager();
	if (pServerMgr)
	{
		int svrSocket = pServerMgr->GetServerSocket( SERVICE_CHATSERVER );

		if(svrSocket > 0)
		{
			SEND_PACKET( sendPacket, svrSocket );
		}
	}
}

//************************************
// Method:    AddAutoMsg
// FullName:  CChatHelper::AddAutoMsg
// Access:    public 
// Returns:   void
// Qualifier: 添加一个自动发送消息
// Parameter: AutoMsg & msg
//************************************
void CChatHelper::AddAutoMsg( AutoMsg& msg )
{
    MAKE_PACKET( sendPacket, "WORLD_CHAT_AddAutoMsg",0,SERVICE_CHATSERVER );
    msg.WriteData(&sendPacket);

    int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

    if(svrSocket > 0)
    {
        SEND_PACKET( sendPacket, svrSocket );
    }
}
void CChatHelper::GetAutoMsg(int sid)
{
	MAKE_PACKET( sendPacket, "WORLD_CHAT_GetAutoMsg",sid,SERVICE_CHATSERVER);

	int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

	if(svrSocket > 0)
	{
		SEND_PACKET( sendPacket, svrSocket );
	}
}

void CChatHelper::DelAutoMsg( AutoMsg& msg )
{
	MAKE_PACKET( sendPacket, "WORLD_CHAT_DelAutoMsg",0,SERVICE_CHATSERVER );
	msg.WriteData(&sendPacket);

	int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

	if(svrSocket > 0)
	{
		SEND_PACKET( sendPacket, svrSocket );
	}
}

void CChatHelper::CancelAutoMsg()
{
	MAKE_PACKET( sendPacket, "WORLD_CHAT_CancelAutoMsg",0,SERVICE_CHATSERVER );

	int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

	if(svrSocket > 0)
	{
		SEND_PACKET( sendPacket, svrSocket );
	}
}


//************************************
// Method:    SendMessageToWorld
// FullName:  CChatHelper::SendMessageToWorld
// Access:    public 
// Returns:   void
// Qualifier: 给全世界发送聊天消息
// Parameter: stChatMessage & msg
//************************************
void CChatHelper::SendMessageToWorld( stChatMessage& msg )
{
    MAKE_PACKET( sendPacket, "WORLD_CHAT_SendMessageToWorld",0,SERVICE_CHATSERVER );
    stChatMessage::PackMessage(msg,sendPacket);

    int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

    if(svrSocket > 0)
    {
        SEND_PACKET( sendPacket, svrSocket );
    }
}

void CChatHelper::SendMessageToLine(int lineId,stChatMessage& msg)
{
/*    MAKE_PACKET(sendPacket,WORLD_CHAT_SendMessageToLine,lineId,SERVICE_CHATSERVER );
    stChatMessage::PackMessage(msg,sendPacket);

    int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

    if(svrSocket > 0)
    {
        SEND_PACKET( sendPacket, svrSocket );
    }*/
}

//************************************
// Method:    SendMessageTo
// FullName:  CChatHelper::SendMessageTo
// Access:    public 
// Returns:   void
// Qualifier: 给制定的人发聊天消息
// Parameter: stChatMessage & msg
// Parameter: int playerId
//************************************
void CChatHelper::SendMessageTo( stChatMessage& msg, int playerId )
{
    MAKE_PACKET( sendPacket, "WORLD_CHAT_SendMessageToPlayer",playerId,SERVICE_CHATSERVER );
    stChatMessage::PackMessage(msg,sendPacket);

    int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

    if(svrSocket > 0)
    {
        SEND_PACKET( sendPacket, svrSocket );
    }
}

//************************************
// Method:    SendMessageToChannel
// FullName:  CChatHelper::SendMessageToChannel
// Access:    public 
// Returns:   void
// Qualifier: 给特定的频道发送聊天消息
// Parameter: stChatMessage & msg
// Parameter: int channelId
//************************************
void CChatHelper::SendMessageToChannel( stChatMessage& msg, int channelId )
{
/*    MAKE_PACKET( sendPacket, WORLD_CHAT_SendMessageToChannel,channelId,SERVICE_CHATSERVER );
    stChatMessage::PackMessage(msg,sendPacket);

    int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );
    
    if(svrSocket > 0)
    {
        SEND_PACKET( sendPacket, svrSocket );
    }*/
}

//************************************
// Method:    addAccountPlayer
// FullName:  CChatHelper::addAccountPlayer
// Access:    public 
// Returns:   void
// Qualifier: 通知聊天服务器新进玩家
// Parameter: int playerId
// Parameter: int accountId
// Parameter: std::string name
//************************************

void CChatHelper::addAccountPlayer(int playerId,U32 accountId,std::string name,const char* accountName,int gateSocket)
{
	AccountRef pAccountInfo = SERVER->GetPlayerManager()->GetAccount(accountId);
	
    ChatPlayerInfo info;
    info.accountId   = accountId;
    info.playerId    = playerId;
    dStrcpy(info.playerName,sizeof(info.playerName),name.c_str());
    dStrcpy(info.accountName,sizeof(info.accountName),accountName);
	if (pAccountInfo)
		dStrcpy(info.loginIp,sizeof(info.loginIp),pAccountInfo->loginIP);

    
    if (!SERVER->GetServerManager()->GetGateInfoBySocket(gateSocket,info.lineId,info.gateId))
    {
        g_Log.WriteError("无法把玩家[%d]添加到聊天服务器,无法获取gate信息",playerId);
        return;
    }
    
    char buf[256];
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket,"WORLD_CHAT_AddPlayer",playerId,SERVICE_CHATSERVER,info.accountId);

    sendPacket.writeBits(8 * sizeof(ChatPlayerInfo),&info);

    pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);

	int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

	if(svrSocket > 0)
	{
        SERVER->GetServerSocket()->Send(svrSocket,sendPacket);
	}
}

//************************************
// Method:    removeAccountPlayer
// FullName:  CChatHelper::removeAccountPlayer
// Access:    public 
// Returns:   void
// Qualifier: 通知聊天服务器玩家离开
// Parameter: int playerId
//************************************
void CChatHelper::removeAccountPlayer(int accountId)
{
    if (0 == accountId)
        return;

    char buf[64];
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket,"WORLD_CHAT_RemovePlayer",accountId,SERVICE_CHATSERVER);
    pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);

	int svrSocket = SERVER->GetServerManager()->GetServerSocket(SERVICE_CHATSERVER);

	if(svrSocket > 0)
	{
		SERVER->GetServerSocket()->Send(svrSocket,sendPacket);
		g_Log.WriteLog("Notify chatserver delete account[%d].", accountId);
	}
}

void CChatHelper::addPlayerToFamily( int playerId, int family )
{
/*	MAKE_PACKET( sendPacket, ZONE_CHAT_AddToFamily, playerId, SERVICE_CHATSERVER, family );

	int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

	if(svrSocket > 0)
	{
		SEND_PACKET( sendPacket, svrSocket );
	}*/
}
