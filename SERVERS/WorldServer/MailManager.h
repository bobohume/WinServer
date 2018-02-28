#ifndef MAIL_MANAGER_H
#define MAIL_MANAGER_H


struct stPacketHead;

#include "base/bitStream.h"
#include "common/PacketType.h"
#include "LockCommon.h"
#include "Common/MailBase.h"
#include "common/PlayerStruct.h"

class CMailManager
{
public:
	CMailManager();
	virtual ~CMailManager();

	static CMailManager* Instance();
	static void Export(struct lua_State* L);

	bool HandleMailListRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	bool HandleReadMailRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet );
	bool HandleMailTryGetItemRequest(int SocketHandle, stPacketHead *pHead, Base::BitStream *Packet);

	int sendMail( int nRecver, stMailItem &mailItem );
	int sendMail(int nSender, int nRecver, int money, int itemid[MAX_MAIL_ITEM_NUM], int itemnum[MAX_MAIL_ITEM_NUM], const char* title, const char* content, bool isSystem);
	int sendMail(int nSender, int nRecver, const char* title, const char* content, bool isSystem);
	//int sendMail(int nSender, int nRecver, int money, stItemInfo& item, const char* title, const char* content, bool isSystem);
	//int sendMailEx(int nSender, int nRecver, int money, int itemid[MAX_MAIL_ITEM_NUM], int itemnum[MAX_MAIL_ITEM_NUM], const char* title,  const char* content, bool isSystem);
	//int sendMailEx( int nRecver, stMailItem &mailItem, stItemInfo ItemInfoEx);
	int deleteMail(int playerId, int mailId, U32 accountId, int SocketHandle);
};

#define  MAILMGR CMailManager::Instance()
#endif