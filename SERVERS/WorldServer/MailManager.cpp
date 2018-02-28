#include "dblib/dbLib.h"
#include "MailManager.h"
#include "WorldServer.h"
#include "Common/MailBase.h"
#include "DBLayer\Data\TBLMailList.h"
#include "Common\dbStruct.h"
#include "PlayerMgr.h"
#include "WINTCP/MessageCode.h"
#include "DB_MailList.h"
#include "DB_SendMail.h"
#include "DB_ReadMail.h"
#include "DB_DeleteAllMail.h"
#include "DB_DeleteMail.h"
#include "DB_TryGetMailItem.h"
#include "DB_GetMailItem.h"
#include "Logic/ItemData.h"
#include "Script/lua_tinker.h"

CMailManager::CMailManager()
{
	REGISTER_EVENT_METHOD("CW_MailListRequest",		this,	&CMailManager::HandleMailListRequest );
	REGISTER_EVENT_METHOD("CW_ReadMailRequest",		this,	&CMailManager::HandleReadMailRequest );
	REGISTER_EVENT_METHOD("CW_GetMailItemRequest",  this,   &CMailManager::HandleMailTryGetItemRequest);
}

CMailManager::~CMailManager()
{
}

CMailManager* CMailManager::Instance()
{
	static CMailManager s_Mgr;
	return &s_Mgr;
}
// ----------------------------------------------------------------------------
// 发送邮件给客户端
int CMailManager::sendMail( int nRecver, stMailItem &mailItem )
{
    DB_SendMail* pDBHandle = new DB_SendMail;

    pDBHandle->m_nRecver  = nRecver;
    pDBHandle->m_mailItem = mailItem;
  
    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
    return DBERR_NONE;
}

// ----------------------------------------------------------------------------
// 发送邮件给客户端
int CMailManager::sendMail(int nSender, int nRecver, int money, int itemId[MAX_MAIL_ITEM_NUM], int itemNum[MAX_MAIL_ITEM_NUM], const char* title, const char* content, bool isSystem)
{
	stMailItem mailItem;
	mailItem.nSender	= nSender;
	mailItem.nRecver	= nRecver;
	for (auto i = 0,j = 0; i < MAX_MAIL_ITEM_NUM; ++i)
	{
		if (itemId[i] != 0)
		{
			mailItem.nItemId[j] = itemId[i];
			mailItem.nItemCount[j] = itemNum[i];
			++j;
		}
	}

	mailItem.nMoney		= money;
	mailItem.isSystem	= isSystem;
	mailItem.nTime		= time(NULL);
	dStrcpy(mailItem.title,COMMON_STRING_LENGTH, title);
	dStrcpy(mailItem.content,MAX_MAIL_ITEM_LENGTH,content);

	return sendMail(nRecver, mailItem);
}

// ----------------------------------------------------------------------------
// 发送邮件给客户端
int CMailManager::sendMail(int nSender, int nRecver, const char* title, const char* content, bool isSystem) 
{
	S32 nItemID[4] = { 0, 0, 0, 0 };
	S32 nItemNum[4] = { 0, 0, 0, 0 };
	return sendMail(nSender, nRecver, 0, nItemID, nItemNum, title, content, isSystem);
}
// ----------------------------------------------------------------------------
// 发送邮件给客户端
/*int CMailManager::sendMailEx( int nRecver, stMailItem &mailItem, stItemInfo ItemInfoEx)
{
	U16 MaxOverNum = 0;
	CItemData* pItemData = g_ItemDataMgr->getData(mailItem.nItemId);
	if(pItemData)
	{
		MaxOverNum = pItemData->m_MaxDie;
	}

	if(MaxOverNum == 0)
		MaxOverNum = 1;

	//物品数量如果大于叠加数量,分多个邮件发送
	int currentMails = 1;
	int totalMails = mailItem.nItemCount/MaxOverNum + ((mailItem.nItemCount%MaxOverNum)?1:0);
	int itemCount = mailItem.nItemCount;
	char szTitle[COMMON_STRING_LENGTH];
	strcpy_s(szTitle, COMMON_STRING_LENGTH,mailItem.title);
	int error = DBERR_NONE;
	while(itemCount>0)
	{
		if(itemCount>MaxOverNum)
		{
			mailItem.nItemCount = MaxOverNum;
			ItemInfoEx.Quantity = MaxOverNum;
		}
		else
		{
			mailItem.nItemCount = itemCount;
			ItemInfoEx.Quantity = itemCount;
		}

		if(totalMails>1)
		{
			sprintf_s(mailItem.title, COMMON_STRING_LENGTH,"%s(%d/%d)",szTitle,currentMails,totalMails);
			error = sendMail(nRecver,mailItem);
		}
		else
			error = sendMail(nRecver,mailItem);

		if(error != DBERR_NONE)
			break;

		itemCount -= mailItem.nItemCount;
		currentMails++;
	}
	return error;
}*/

// ----------------------------------------------------------------------------
// 发送邮件给客户端
/*int CMailManager::sendMailEx(int nSender, int nRecver, int money, int itemId[MAX_MAIL_ITEM_NUM], int itemNum[MAX_MAIL_ITEM_NUM], const char* title, const char* content, bool isSystem)
{
	U16 MaxOverNum = 0;
	CItemData* pItemData = g_ItemDataMgr->getData(itemId);
	if(pItemData)
	{
		MaxOverNum = pItemData->m_MaxDie;
	}

	if(MaxOverNum == 0)
		MaxOverNum = 1;

	//物品数量如果大于叠加数量,分多个邮件发送
	int currentMails = 1;
	int totalMails = itemNum/MaxOverNum + ((itemNum%MaxOverNum)?1:0);
	char szTitle[COMMON_STRING_LENGTH];
	int error = DBERR_NONE;
	while(itemNum>0)
	{
		int count = itemNum;
		if(itemNum>MaxOverNum)
			count = MaxOverNum;

		if(totalMails>1)
		{
			sprintf_s(szTitle, COMMON_STRING_LENGTH,"%s(%d/%d)",title,currentMails,totalMails);
			error = sendMail(nSender, nRecver, money, itemId, count, szTitle, content, isSystem);
		}
		else
			error = sendMail(nSender, nRecver, money, itemId, count, title, content, isSystem);

		if(error != DBERR_NONE)
			break;

		itemNum -= count;
		currentMails++;
	}
	return error;
}*/
// ----------------------------------------------------------------------------
// 发送邮件给客户端
/*int CMailManager::sendMail(int nSender, int nRecver, int money, stItemInfo& item, const char* title, const char* content, bool isSystem)
{
	stMailItem mailItem;

	mailItem.nSender	= nSender;
	mailItem.nRecver	= nRecver;
	mailItem.nItemId    = item.ItemID;
	mailItem.nItemCount = item.Quantity;
	mailItem.nMoney		= money;
	mailItem.isSystem	= isSystem;
	dStrcpy(mailItem.title,COMMON_STRING_LENGTH, title);
	dStrcpy(mailItem.content,MAX_MAIL_ITEM_LENGTH,content);

	return sendMail(nRecver, mailItem);
}*/
//--------------------------------------------------------------------------------
//删除邮件
int CMailManager::deleteMail(int playerId, int mailId, U32 accountId, int SocketHandle)
{
	std::vector<int> mailList;
	mailList.push_back(mailId);
	DB_DeleteMail* pDBHandle = new DB_DeleteMail;	
	pDBHandle->m_socketHandle = SocketHandle;
	pDBHandle->m_accountId = accountId;
	pDBHandle->m_nRecver = playerId;
	pDBHandle->m_idlist = mailList;

	SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
	return DBERR_NONE;
}

// 处理来自客户端的邮件请求
bool CMailManager::HandleReadMailRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	int AccountId = pHead->Id;
	int mailId = Packet->readInt(Base::Bit32);

	AccountRef pInfo = SERVER->GetPlayerManager()->GetAccount(AccountId);

	if(!pInfo)
		return false;

	DB_ReadMail* pDBHandle = new DB_ReadMail;

    pDBHandle->m_socketHandle = SocketHandle;
    pDBHandle->m_mailId	      = mailId;
	pDBHandle->m_accountId    = pInfo->GetAccountId();
    pDBHandle->m_playerId	  = pInfo->GetPlayerId();

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
	return true;
}

// 处理来自Zone查询每页邮件的请求
bool CMailManager::HandleMailListRequest( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
{
	int AccountId = pHead->Id;
	AccountRef pInfo = SERVER->GetPlayerManager()->GetAccount(AccountId);

	if(!pInfo)
		return false;

    DB_MailList* pDBHandle = new DB_MailList;

    pDBHandle->m_socketHandle = SocketHandle;
    pDBHandle->m_accountId	  = pInfo->GetAccountId();
    pDBHandle->m_playerId     = pInfo->GetPlayerId();

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
	return true;
}

// 处理来自Zone查询每页邮件的请求
bool CMailManager::HandleMailTryGetItemRequest(int SocketHandle, stPacketHead *pHead, Base::BitStream *Packet)
{
	int AccountId = pHead->Id;
	int mailId = Packet->readInt(Base::Bit32);

	AccountRef pInfo = SERVER->GetPlayerManager()->GetAccount(AccountId);

	if (!pInfo)
		return false;

	DB_TryGetMailItem* pDBHandle = new DB_TryGetMailItem;

	pDBHandle->m_socketHandle = SocketHandle;
	pDBHandle->m_nMailId = mailId;
	pDBHandle->m_accountId = pInfo->GetAccountId();
	pDBHandle->m_nPlayerId = pInfo->GetPlayerId();

	SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
	return true;
}

void ___sendmail(int nSender, int nRecver, int money,const char* title, const char* content, bool isSystem)
{
	int itemid[MAX_MAIL_ITEM_NUM]  = { 600076, 600077,600078,600079 };
	int itemnum[MAX_MAIL_ITEM_NUM] = { 1, 2,3,5 };
	MAILMGR->sendMail(nSender, nRecver, money, itemid, itemnum, title, content, isSystem);


	AccountRef pInfo = SERVER->GetPlayerManager()->GetOnlinePlayer(nRecver);

	if (!pInfo)
		return;
	DB_MailList* pDBHandle = new DB_MailList;

	pDBHandle->m_socketHandle = pInfo->GetGateSocketId();
	pDBHandle->m_accountId = pInfo->GetAccountId();
	pDBHandle->m_playerId = nRecver;

	SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}

void CMailManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "sendmail", &___sendmail);
}