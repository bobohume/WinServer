#ifndef CHAT_BASE_H
#define CHAT_BASE_H

#include "Base/Log.h"

const int CHAT_MESSAGE_MAX_LENGTH = 128;

using namespace std;

using namespace stdext;

enum eChatMessageType
{
	CHAT_MSG_TYPE_CHANNEL,		// 频道
	CHAT_MSG_TYPE_PRIVATE,		// 私聊
	CHAT_MSG_TYPE_GM,			// GM消息
	CHAT_MSG_TYPE_UNKNOWN,		// 预留
};

struct stChatMessage
{
	BYTE btMessageType;
	UINT btChannelID;
	char szMessage[CHAT_MESSAGE_MAX_LENGTH];
};

enum eChatChannelType
{
	CHAT_CHANNEL_TYPE_SYSTEM,		// 系统
	CHAT_CHANNEL_TYPE_AREA,			// 区域
	CHAT_CHANNEL_TYPE_GROUP,		// 人群
};

#endif