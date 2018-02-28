#ifndef __CHAT_HELPER__
#define __CHAT_HELPER__

#include "Common/ChatBase.h"
#include "LockCommon.h"
#include "ManagerBase.h"

typedef void (*RegisterChannelCallback)( int type, int channelId, void* param );

class CChatHelper :public CManagerBase< CChatHelper>
{
public:
	CChatHelper();
	virtual ~CChatHelper();

	void addAccountPlayer(int playerId,U32 accountId,std::string name,const char* accountName,int gateSocket);
	void removeAccountPlayer(int playerId);

	void clearAccount( int accountId );
	void addPlayer( int channelId, int playerId );
	void removePlayer( int channelId, int playerId );
	void registerChannel( int type, RegisterChannelCallback callback, void* param );
	void unregisterChannel( int channelId );
	void AddAutoMsg( AutoMsg& msg );
	void GetAutoMsg(int sid);
	void DelAutoMsg( AutoMsg& msg );
	void CancelAutoMsg();
	void SendMessageToWorld(stChatMessage& msg);
    void SendMessageToLine(int lineId,stChatMessage& msg);
	void SendMessageTo( stChatMessage& msg, int playerId );
	void SendMessageToChannel( stChatMessage& msg, int channelId );
	bool HandleAutoMsgResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);
	bool HandleRegisterChannelResponse(int SocketHandle, stPacketHead *pHead, Base::BitStream *Packet);
	void addPlayerToFamily( int playerId, int family );
};

extern CChatHelper gHelper;

#define CHAT_HELPER (&gHelper)


#endif