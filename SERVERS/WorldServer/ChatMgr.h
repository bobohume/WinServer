#ifndef CHAT_MANAGER_H
#define CHAT_MANAGER_H

#include <deque>
#include <list>

#include "Common/PacketType.h"
#include "Base/bitStream.h"
#include "Base/Locker.h"
#include "Common/MemGuard.h"
//#include "Common/Script.h"
#include "wintcp/IPacket.h"
#include "wintcp/dtServerSocket.h"

//#include "PlayerMgr.h"
//#include "ServerMgr.h"
#include "Common/ChatBase.h"
#include "Channel.h"

#include "LockCommon.h"
#include "ManagerBase.h"


#define CHAT_PENDING_TIME_NORAML		3000
#define CHAT_PENDING_TIME_PRIVATE		1000
#define CHAT_PENDING_TIME_WORLDPLUS		30000
#define CHAT_PENDING_TIME_REALCITY		5000
#define CHAT_PENDING_TIME_LINE			5000

#define GM_FLAG	'$'


class CWorldServer;

struct stPlayerChatRecord
{
	stPlayerChatRecord()
	{
		memset( this, 0, sizeof( stPlayerChatRecord ) );	
	}

	U64 nLastTime;
	U64 nPendingTime;
};

struct ChatBanInfo
{
	static const int MAX_LENGTH = 32;

	enum TargetType
	{
		TARGET_TYPE_ACC = 0,   //帐号
		TARGET_TYPE_ACTOR,     //角色
		TARGET_TYPE_IP,        //地址

		TARGET_TYPE_COUNT
	};

	enum BadWordsType
	{
		SENS = 0,			//敏感
		SUSP				//可疑
	};


	char name[MAX_LENGTH];
	int         type;
	int         chatBanExpire;
	char chatReason[MAX_LENGTH];

	ChatBanInfo()
	{
		memset(this,0,sizeof(ChatBanInfo));
	}

	void pack(Base::BitStream* pPacket)
	{
		pPacket->writeString(name,MAX_LENGTH);
		pPacket->writeInt(type,Base::Bit32);
		pPacket->writeInt(chatBanExpire,Base::Bit32);
		pPacket->writeString(chatReason,MAX_LENGTH);
	}

	void unpack(Base::BitStream* pPacket)
	{
		pPacket->readString(name,MAX_LENGTH);
		type  = pPacket->readInt(Base::Bit32);
		chatBanExpire  = pPacket->readInt(Base::Bit32);
		pPacket->readString(chatReason,MAX_LENGTH);
	}

	bool IsChatBaned(int curTime)
	{
		if (curTime >= chatBanExpire)
		{
			chatBanExpire = 0;
			return false;
		}

		return true;
	}
};


class Player;

#define MAX_CITY_CHANNEL 10000
#define MAX_FAMILY_COUNT 8
#define UseGB2312 _acp = 936

class CChatManager :  public CManagerBase< CChatManager>
{
public:
	CChatManager();
	virtual ~CChatManager();
private:
	bool IsBaned( stChatMessage& chatMessage );
	
public:
	void Initialize();

	void readFile(const char* path,int type);
	bool isSusWord(const char* pName);
	bool isSensWord(const char* pName);
    void HandleIncomingMsg( int nSocket, stPacketHead* pHead, Base::BitStream* pBitStream );
	void HandleGMMessage( stChatMessage& chatMessage );

	void SendChatResponse( int nSocket, int nAccountId, char cType, int value = 0 ,BYTE nChannel = 0);
	
	void SendMessage(stChatMessage& chatMessage,const std::vector<int>& playerList);

	void SendMessageTo( stChatMessage& chatMessage, int nPlayerId );
	void SendMessageToWorld( stChatMessage& chatMessage );
	void SendMessageToLine( stChatMessage& chatMessage, int nLineId );
	void SendMessageToChannel( stChatMessage& chatMessage, int nChannelId );
	CChannelManager* GetChannelManager();

	U64 GetPlayerChatPendingTime( int nPlayerId, char cMessageType );

	void SetPlayerChatLastTime( int nPlayerId, char cMessageType, U64 nTime );
	U64 GetPlayerChatLastTime( int nPlayerId, char cMessageType );

	void AddAutoMsg( AutoMsg& msg );
	void GetAutoMsg(int sid);
	void DelAutoMsg(AutoMsg& msg);
	void CancelAutoMsg();

    /************************************************************************/
    /* 更新聊天系统，推进自动消息的发送
    /************************************************************************/
    void Update(void);
	void HandleAddToFamily(int nSocket,stPacketHead* pHead, Base::BitStream* pBitStream );
	void HandleZoneSendMessage( int lineId, stPacketHead* pHead, Base::BitStream* pBitStream );
	void HandleJoinCity( int nSocket, stPacketHead* pHead, Base::BitStream* pBitStream );
	void initCityChannels();
	void writeLog( int playerId, int type, const char* msg, int playerIdTo = 0 );
	void writeBadWordPlayer(int playerId,char* pWord,int type);

	 std::vector<ChatBanInfo> m_banList;

#ifndef _NTJ_UNITTEST
protected:
#endif
	CChannelManager m_channelManager;

    int m_FamilyChannels[MAX_FAMILY_COUNT];

	typedef stdext::hash_map<__int64, stPlayerChatRecord> CHATREC_MAP;
    CHATREC_MAP m_playerChatMap;

    std::list<AutoMsg> m_autoMsgs;

	typedef	std::vector<std::wstring>					 VEC_WSTR;
	typedef std::vector<std::wstring>::const_iterator    VEC_WSTR_CONITER;

	VEC_WSTR	m_sensWords;                //敏感词汇
	VEC_WSTR	m_susWords;				   //可疑词汇


	int mChannelMap[MAX_CITY_CHANNEL];

	CMyLog m_chatLog;
public:
	CMyCriticalSection m_cs;
};

#endif


