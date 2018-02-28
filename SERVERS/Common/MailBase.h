#ifndef MAIL_BASE_H
#define MAIL_BASE_H

#include "Base/BitStream.h"
#include "Common/PacketType.h"
#include "Common/UtilString.h"

#define MAX_MAIL_ITEM_LENGTH 1024
#define MAX_MAIL_ITEM_ILENGTH 512
#define MAX_MAIL_REFLUSH_TIME 10
#define MAX_PAGE_MAIL_ITEM 6
#define MAX_MAIL_ITEM_NUM 4

enum
{
	SYS_MAIL,
	TXT_MAIL,
	BLOB_MAIL,
};

struct stMailHeader
{
	int nRecver;				//收取人
	int nPage;					//当前页数
	int nCount;					//每页邮件数
	int nRecvCount;				//总邮件数
	int nNoReadCount;			//未读邮件
	int nType;					//请求类型

	stMailHeader() { memset(this, 0, sizeof(stMailHeader));}

	template< typename _Ty >
	void PackData( _Ty& packet )
	{
		packet.writeInt(nRecver,	32);
		packet.writeInt(nPage,		8);
		packet.writeInt(nCount,		8);
		packet.writeInt(nRecvCount,	16);
		packet.writeInt(nNoReadCount, 16);
		packet.writeInt(nType, 8);
	}

	template< typename _Ty >
	void UnpackData( _Ty& packet )
	{
		nRecver		= packet.readInt(32);
		nPage		= packet.readInt(8);
		nCount		= packet.readInt(8);
		nRecvCount	= packet.readInt(16);
		nNoReadCount= packet.readInt(16);
		nType		= packet.readInt(8);
	}
};

struct stMailItem
{
	int		id;
	int		nSender;
	char	szSenderName[COMMON_STRING_LENGTH];
	int		nMoney;
	int		nItemId[MAX_MAIL_ITEM_NUM];
	int		nItemCount[MAX_MAIL_ITEM_NUM];
	int		nTime;
	bool	isReaded;
	int		isSystem;
	int		nRecver;
	char	szRecverName[COMMON_STRING_LENGTH];

	bool	haveContent;
	char	title[COMMON_STRING_LENGTH];
	char	content[MAX_MAIL_ITEM_LENGTH];

	stMailItem() {	memset( this, 0, sizeof( stMailItem ) );}

	stMailItem( const stMailItem& mailItem )
	{
		id			= mailItem.id;
		nSender		= mailItem.nSender;
		nMoney		= mailItem.nMoney;
		for(auto i = 0; i < MAX_MAIL_ITEM_NUM; ++i)
		{
			nItemId[i]	= mailItem.nItemId[i];
			nItemCount[i] = mailItem.nItemCount[i];
		}
		nTime		= mailItem.nTime;
		isReaded	= mailItem.isReaded;
		isSystem	= mailItem.isSystem;
		haveContent = mailItem.haveContent;

		strcpy_s( content, sizeof( content ), mailItem.content );
		strcpy_s( szSenderName, sizeof( szSenderName ), mailItem.szSenderName );
		strcpy_s( title, sizeof( title ), mailItem.title );
		strcpy_s(szRecverName, sizeof(szRecverName), mailItem.szRecverName);
	}

	template< typename _Ty >
	void PackData( _Ty& packet, bool bSimple = true)
	{
		packet.writeInt(id,			32);
		packet.writeInt(nTime,		32);
		packet.writeInt(nSender,	32);
		packet.writeFlag(isReaded );
		packet.writeString(Util::MbcsToUtf8(szSenderName), COMMON_STRING_LENGTH * 2);
		packet.writeString(Util::MbcsToUtf8(szRecverName), COMMON_STRING_LENGTH * 2);
		packet.writeString(Util::MbcsToUtf8(title), COMMON_STRING_LENGTH * 2 );
		if (!bSimple)
		{
			packet.writeInt(MAX_MAIL_ITEM_NUM, Base::Bit16);
			for (auto i = 0; i < MAX_MAIL_ITEM_NUM; ++i)
			{
				packet.writeFlag(true);
				packet.writeInt(nItemId[i], 32);
				packet.writeInt(nItemCount[i], 16);
			}

			packet.writeString(Util::MbcsToUtf8(content), 512);
		}
	}
};

#endif