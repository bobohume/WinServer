#ifndef CHAT_BASE_H
#define CHAT_BASE_H

//#include "Base/Log.h"
#ifndef _WINDOWS_
#include <WinSock2.h>
#include <windows.h>
#endif
#pragma once
#include <iostream>
#include <sstream>
#include "Base/bitStream.h"
#include "Common/PlayerStruct.h"
#include "Common/ScriptVariable.h"
#include "Common/PacketType.h"
#include "Common/UtilString.h"
#include "BASE/base64.h"

const int CHAT_MESSAGE_MAX_LENGTH = 512;
const int CHAT_MESSAGE_MAX_NAME_LENGTH = 32;

using namespace std;

enum eSendChatResponseType
{
	SEND_CHAT_ACK_UNKNOWN = 0,
	SEND_CHAT_ACK_SUCCESSED,
	SEND_CHAT_ACK_TOOFAST,
	SEND_CHAT_ACK_REFUSED,
	SEND_CHAT_ACK_ERROR,
	SEND_CHAT_ACK_NOPLAYER,
	SEND_CHAT_ACK_ILLEGAL,
};

enum enChatType
{
	CHAT_TYPE_NORMAL        = BIT(0),//正常聊天
	CHAT_TYPE_ITEM          = BIT(1),//带物品的聊天
};

#define CHATBEGIN(name, script) SCRIPTVARTYPES8(name, script, 0); const int CHAT_VALUE_BEING = __LINE__;
#define CHATDEFINE(name, script) SCRIPTVARTYPES8(name, script, __LINE__-CHAT_VALUE_BEING);
enum
{
	CHAT_MSG_TYPE_SYSTEM,
	CHAT_MSG_TYPE_WORLDPLUS,
	CHAT_MSG_TYPE_WORLD,
	CHAT_MSG_TYPE_LINE,
	CHAT_MSG_TYPE_RACE,
	CHAT_MSG_TYPE_ORG,
	CHAT_MSG_TYPE_SQUAD,
	CHAT_MSG_TYPE_TEAM,
	CHAT_MSG_TYPE_GROUP,
	CHAT_MSG_TYPE_REALCITY,
	CHAT_MSG_TYPE_NEARBY,
	CHAT_MSG_TYPE_PRIVATE,
	CHAT_MSG_TYPE_P2P,
	CHAT_MSG_TYPE_GM,
	CHAT_MSG_TYPE_PERSON,
	CHAT_MSG_TYPE_PATH,
	CHAT_MSG_TYPE_BATTLE,
	CHAT_MSG_TYPE_RECORD,
	CHAT_MSG_TYPE_SYSPLUS,
	CHAT_MSG_TYPE_LEAGUE,
	CHAT_MSG_TYPE_WORLD_2,
	CHAT_MSG_TYPE_SUPER_WORLD,
	CHAT_MSG_TYPE_BFCORPS,
	CHAT_MSG_TYPE_MSGBOX,
	CHAT_MSG_TYPE_CHALLENGE,
	CHAT_MSG_TYPE_AWARD,
	CHAT_MSG_TYPE_SUPER_WORLD2,
	CHAT_MSG_TYPE_UNKNOWN,
	CHAT_MSG_TYPE_COUNT,
};

/*CHATBEGIN(CHAT_MSG_TYPE_SYSTEM,		"chatMsg_System")		// 系统
CHATDEFINE(CHAT_MSG_TYPE_WORLDPLUS,	"chatMsg_WordPlus")		// 跑马
CHATDEFINE(CHAT_MSG_TYPE_WORLD,		"chatMsg_Word")			// 跨服
CHATDEFINE(CHAT_MSG_TYPE_LINE,		"chatMsg_Line")			// 全服
CHATDEFINE(CHAT_MSG_TYPE_RACE,		"chatMsg_Race")			// 门宗
CHATDEFINE(CHAT_MSG_TYPE_ORG,		"chatMsg_Org")			// 帮会
CHATDEFINE(CHAT_MSG_TYPE_SQUAD,		"chatMsg_Squad")		// 团
CHATDEFINE(CHAT_MSG_TYPE_TEAM,		"chatMsg_Team")			// 队伍
CHATDEFINE(CHAT_MSG_TYPE_GROUP,		"chatMsg_Group")		// 群消息
CHATDEFINE(CHAT_MSG_TYPE_REALCITY,	"chatMsg_RealCity")		// 同城
CHATDEFINE(CHAT_MSG_TYPE_NEARBY,	"chatMsg_Nearby")		// 附近
CHATDEFINE(CHAT_MSG_TYPE_PRIVATE,	"chatMsg_Private")		// 私聊
CHATDEFINE(CHAT_MSG_TYPE_P2P,		"chatMsg_P2P")			// 私聊窗口
CHATDEFINE(CHAT_MSG_TYPE_GM,		"chatMsg_GM")			// GM消息
CHATDEFINE(CHAT_MSG_TYPE_PERSON,	"chatMsg_Person")		// 个人
CHATDEFINE(CHAT_MSG_TYPE_PATH,		"chatMsg_Path")			// 寻径
CHATDEFINE(CHAT_MSG_TYPE_BATTLE,	"chatMsg_Battle")		// 战斗
CHATDEFINE(CHAT_MSG_TYPE_RECORD,	"chatMsg_Record")		// 留言
CHATDEFINE(CHAT_MSG_TYPE_SYSPLUS,	"chatMsg_SysPlus")		// 系统跑马频道
CHATDEFINE(CHAT_MSG_TYPE_LEAGUE,	"chatMsg_League")		// 联盟
CHATDEFINE(CHAT_MSG_TYPE_WORLD_2,   "chatMsg_World2")	    // 客户端左下角的全服广播
CHATDEFINE(CHAT_MSG_TYPE_SUPER_WORLD,"chatMsg_SuperWorld")  // 超级跑马
CHATDEFINE(CHAT_MSG_TYPE_BFCORPS,   "chatMsg_BFCorps")		// 战场团队
CHATDEFINE(CHAT_MSG_TYPE_MSGBOX,	"chatMsg_MsgBox")		// 全服弹框
CHATDEFINE(CHAT_MSG_TYPE_CHALLENGE,	"chatMsg_Challenge")	// 竞技场全服公告
CHATDEFINE(CHAT_MSG_TYPE_AWARD,     "chatMsg_MadDialAward")	// 疯狂大转盘获奖信息记录
CHATDEFINE(CHAT_MSG_TYPE_SUPER_WORLD2,"chatMsg_SuperWorld2")// 千里传音界面的超级跑马
CHATDEFINE(CHAT_MSG_TYPE_UNKNOWN,	"chatMsg_Unknown")		// 预留
CHATDEFINE(CHAT_MSG_TYPE_COUNT,		"chatMsg_Count")	*/


//enum eChatMessageType
//{
//	CHAT_MSG_TYPE_SYSTEM,		// 系统
//	CHAT_MSG_TYPE_WORLDPLUS,	// 跑马
//	CHAT_MSG_TYPE_WORLD,		// 跨服
//	CHAT_MSG_TYPE_LINE,			// 全服
//	//----------------------------------------
//	CHAT_MSG_TYPE_RACE,			// 门宗
//	CHAT_MSG_TYPE_ORG,			// 帮会
//	CHAT_MSG_TYPE_SQUAD,		// 团
//	CHAT_MSG_TYPE_TEAM,			// 队伍
//	CHAT_MSG_TYPE_GROUP,		// 群消息
//	CHAT_MSG_TYPE_REALCITY,		// 同城
//	CHAT_MSG_TYPE_NEARBY,		// 附近
//	//----------------------------------------
//	CHAT_MSG_TYPE_PRIVATE,		// 私聊
//	CHAT_MSG_TYPE_P2P,			// 私聊窗口
//	CHAT_MSG_TYPE_GM,			// GM消息
//	CHAT_MSG_TYPE_PERSON,		// 个人
//	CHAT_MSG_TYPE_PATH,			// 寻径
//	CHAT_MSG_TYPE_BATTLE,		// 战斗
//	CHAT_MSG_TYPE_RECORD ,	    // 留言
//	CHAT_MSG_TYPE_SYSPLUS,		// 系统跑马频道
//	CHAT_MSG_TYPE_LEAGUE,		// 联盟
//	CHAT_MSG_TYPE_UNKNOWN,		// 预留
//
//    CHAT_MSG_TYPE_COUNT
//};

static char* strChatMessageType[CHAT_MSG_TYPE_COUNT] = 
{
	"系统",
	"跑马",
	"跨服",
	"全服",
	//----------------------------------------
	"门宗",
	"帮会",
    "团",
	"队伍",
    "群消息",
	"同城",
	"附近",
	//----------------------------------------
	"私聊",
	"私聊窗口",
	"GM消息",
	"个人",
	"寻径",
	"战斗",
	"留言",
	"跑马",
	"联盟",
	"..."
};

struct stChatMessage
{
	int	nSender;	// 发送者
	int nRecver;	// 接收者 
	int nSenderLineID;		//发送者的线程id

	BYTE btMessageType;
	char szMessage[CHAT_MESSAGE_MAX_LENGTH];
	char szSenderName[CHAT_MESSAGE_MAX_NAME_LENGTH];
	char szRecverName[CHAT_MESSAGE_MAX_NAME_LENGTH];
	BYTE btPrivilegeFlagType; //特权类型 

	stChatMessage()
	{
		memset( this, 0, sizeof( stChatMessage ) );
		nSenderLineID = -1;
	}

	static void PackMessage( stChatMessage& chatMessage, Base::BitStream& stream )
	{
		static size_t buffSize = 0;
		stream.writeInt( chatMessage.nSender, Base::Bit32 );
		stream.writeInt( chatMessage.nRecver, Base::Bit32 );
		stream.writeInt( chatMessage.nSenderLineID, Base::Bit32 );

		stream.writeInt( chatMessage.btMessageType, Base::Bit8 );
		stream.writeString(chatMessage.szSenderName, CHAT_MESSAGE_MAX_NAME_LENGTH );
		stream.writeString( chatMessage.szRecverName, CHAT_MESSAGE_MAX_NAME_LENGTH );
		stream.writeString(chatMessage.szMessage, CHAT_MESSAGE_MAX_LENGTH);
		stream.writeInt( chatMessage.btPrivilegeFlagType, Base::Bit8 );
	}

	static void PackMessage1(stChatMessage& chatMessage, Base::BitStream& stream)
	{
		static size_t buffSize = 0;
		stream.writeInt(chatMessage.nSender, Base::Bit32);
		stream.writeInt(chatMessage.nRecver, Base::Bit32);
		stream.writeInt(chatMessage.nSenderLineID, Base::Bit32);

		stream.writeInt(chatMessage.btMessageType, Base::Bit8);
		stream.writeString(Util::MbcsToUtf8(chatMessage.szSenderName), CHAT_MESSAGE_MAX_NAME_LENGTH);
		stream.writeString(Util::MbcsToUtf8(chatMessage.szRecverName), CHAT_MESSAGE_MAX_NAME_LENGTH);
		//stream.writeString(Util::MbcsToUtf8(chatMessage.szMessage), CHAT_MESSAGE_MAX_LENGTH);
		std::string str = Util::MbcsToUtf8(chatMessage.szMessage);
		stream.writeString(base64_encode((unsigned char *)str.c_str(), str.length()), CHAT_MESSAGE_MAX_LENGTH);
		stream.writeInt(chatMessage.btPrivilegeFlagType, Base::Bit8);
	}

	static void UnpackMessage( stChatMessage& chatMessage, Base::BitStream& stream )
	{
		static size_t buffSize = 0;
		chatMessage.nSender = (int)stream.readInt( Base::Bit32 );
		chatMessage.nRecver = (int)stream.readInt( Base::Bit32 );
		chatMessage.nSenderLineID = (int)stream.readInt( Base::Bit32 );

		chatMessage.btMessageType = (BYTE)stream.readInt( Base::Bit8 );
		stream.readString( chatMessage.szSenderName, CHAT_MESSAGE_MAX_NAME_LENGTH );
		stream.readString( chatMessage.szRecverName, CHAT_MESSAGE_MAX_NAME_LENGTH );
		stream.readString(chatMessage.szMessage, CHAT_MESSAGE_MAX_LENGTH);
		chatMessage.btPrivilegeFlagType = (BYTE)stream.readInt( Base::Bit8 );
	}

	static void UnpackMessage1(stChatMessage& chatMessage, Base::BitStream& stream)
	{
		static size_t buffSize = 0;
		chatMessage.nSender = (int)stream.readInt(Base::Bit32);
		chatMessage.nRecver = (int)stream.readInt(Base::Bit32);
		chatMessage.nSenderLineID = (int)stream.readInt(Base::Bit32);

		chatMessage.btMessageType = (BYTE)stream.readInt(Base::Bit8);
		stream.readString(chatMessage.szSenderName, CHAT_MESSAGE_MAX_NAME_LENGTH);
		stream.readString(chatMessage.szRecverName, CHAT_MESSAGE_MAX_NAME_LENGTH);
		//stream.readString(chatMessage.szMessage, CHAT_MESSAGE_MAX_LENGTH);
		std::string str = stream.readString(CHAT_MESSAGE_MAX_LENGTH);
		std::string str1 = base64_decode(str);
		dMemcpy(chatMessage.szMessage, str1.c_str(), str1.length());
		chatMessage.btPrivilegeFlagType = (BYTE)stream.readInt(Base::Bit8);
	}

};

struct AutoMsg
{
	AutoMsg(void) : playerId(0),channel(0),startTime(0),endTime(0),lastSendTime(0),freq(-1),times(-1) {}

	stChatMessage msg;          //消息
	int           playerId;     //如果是针对玩家的则不为空
	int           channel;      //频道
	__time64_t    startTime;    //发送的开始时间
	__time64_t    endTime;      //消息结束时间
	__time64_t    lastSendTime; //上次发发送的时间
	int           freq;         //多少s发送1次
	int           times;        //总共发送多少次

	template< typename _Ty >
	void ReadData( _Ty* packet )
	{
        stChatMessage::UnpackMessage(msg,*packet);
        playerId = packet->readInt(Base::Bit32);
        channel  = packet->readInt(Base::Bit32);
        packet->readBits(sizeof(startTime) * 8,&startTime);
        packet->readBits(sizeof(endTime) * 8,&endTime);
        packet->readBits(sizeof(lastSendTime) * 8,&lastSendTime);
        freq  = packet->readInt(Base::Bit32);
        times = packet->readInt(Base::Bit32);
	}

	template< typename _Ty >
	void WriteData( _Ty* packet )
	{
        stChatMessage::PackMessage(msg,*packet);
        packet->writeInt(playerId,Base::Bit32);
        packet->writeInt(channel,Base::Bit32);
        packet->writeBits(sizeof(startTime) * 8,&startTime);
        packet->writeBits(sizeof(endTime)  * 8,&endTime);
        packet->writeBits(sizeof(lastSendTime) * 8,&lastSendTime);
        packet->writeInt(freq,Base::Bit32);
        packet->writeInt(times,Base::Bit32);
	}
};

//注册的玩家信息
struct ChatPlayerInfo
{
    ChatPlayerInfo(void)
    {
        memset(this,0,sizeof(ChatPlayerInfo));
    }
    
    U32  playerId;
    U32  accountId;
    char playerName[32];
    char accountName[ACCOUNT_NAME_LENGTH];
	 char loginIp[32];

    //当前所属的gate
    U32  lineId;
    U32  gateId;
};

#endif
