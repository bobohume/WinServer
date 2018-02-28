#ifndef PLAYER_STRUCT_H
#define PLAYER_STRUCT_H

#include <memory>
#include "base/bitStream.h"
#include "quicklz/quicklz.h"
#include "sqlGenTypedef.h"
#include "Common/MultiMountBase.h"
#include <sstream>
#include "base/Log.h"
#if !defined(TCP_SERVER)
class BitStream;
#endif

const unsigned int MAX_UUID_LENGTH			= 37;					// 物品UID长度
const unsigned int MAX_NAME_LENGTH			= 32;
const unsigned int Default_Player_Top_Level = 80;					// 天劫玩家最大等级

enum enFamily
{
	Family_None        = 0,
	Family_Ren,//人
	Family_Sheng,//圣
	Family_Fo,
	Family_Xian,//仙
	Family_Jing,//金
	Family_Gui,//鬼
	Family_Guai,//怪
	Family_Yao,//妖
	Family_Mo,//魔
	Family_Shou,//兽
	Family_Long,//龙
	Family_Max,
};

static char* g_strFamily[Family_Max] = 
{
	"无",
	"人",
	"佛",
	"仙",
	"精",
	"鬼",
	"怪",
	"妖",
	"魔",
	"兽",
	"龙",
};

static char * g_strFirstClass[] = 
{
	"新手",
	"天师",
	"灵尊",
	"金刚",
	"修罗",
	"祭司",
	"飞灵",
	"剑灵",
};

// 主次职业
enum enClassesMaster
{
	_1st,
	_2nd,
	_3rd,
	_4th,
	_5th,
	MaxClassesMasters,
};

// 效果激活(APF)
enum enActivePropertyFlag
{
	EAPF_ATTACH			= 1 << 1,	// 装备标志
	EAPF_EQUIPSTRENTH   = 1 << 2,	// 装备强化升级
};

// 物品数据结构
struct stItemInfo
{
	char	UID[MAX_UUID_LENGTH];			// 物品世界唯一ID
	U32		ItemID;							// 物品模板ID
	U32		PlayerID;						// 玩家ID
	U16		Quantity;						// 物品数量(对于装备，只能为1)
	U8		Level;							// 装备等级
	U32		Exp;							// 装备经验
	stItemInfo() { InitData();}
	void InitData()
	{
		memset(this, 0, sizeof(stItemInfo));
	}

	void WriteData(Base::BitStream* stream);
	void ReadData(Base::BitStream* stream);
};

struct stPetInfo
{				
	U32			petDataId;						//模板ID

	stPetInfo() { InitData();}
	void InitData() { memset(this, 0, sizeof(stPetInfo));}

	void WriteData(Base::BitStream* stream);
	void ReadData(Base::BitStream* stream);
};

//帐号表里的status标识,用来做为这个帐号下所有玩家共享的功能标识
enum EAccountStatus
{
	EAS_OLDPLAYERREWARD = BIT(0),		//老玩家奖励
	EAS_PRECREATEREWARD = BIT(1),		//预创建玩家奖励
	EAS_OLDPLAYER_VIPACCOUNT = BIT(2),  //老玩家&vip帐号
};

//扩展基础玩家信息,这些是不需要更新到DB的
struct SimplePlayerData
{
	U32  AccountId;
	U32  PlayerId;
	char PlayerName[32];
	U8  Sex;
	U8  Level;
	S32 Family;
	S32	FirstClass;
	U16 ZoneId;
	U32 Gold;
	U32 DrawGold;
	U32 VipLv;
	U32 LastLogoutTime;
	U32 LastLoginTime;

	SimplePlayerData(void):AccountId(0), PlayerId(0), Sex(0), Level(0), Family(0), FirstClass(0), ZoneId(0), Gold(0),\
		VipLv(0), LastLoginTime(0), LastLogoutTime(0), DrawGold(0)
	{
		memset(PlayerName, 0, sizeof(PlayerName));
	}

	U32 GetId(void) const {return PlayerId;}

	void WriteData(Base::BitStream* stream)
	{
		//stream->writeInt(AccountId, Base::Bit32);
		stream->writeInt(PlayerId, Base::Bit32);
		stream->writeString(PlayerName, 32);
		/*stream->writeInt(Sex, Base::Bit8);
		stream->writeInt(Level, Base::Bit8);
		stream->writeInt(ZoneId, Base::Bit16);*/
		stream->writeInt(Gold, Base::Bit32);
		/*stream->writeInt(Family, Base::Bit8);
		stream->writeInt(FirstClass, Base::Bit8);
		stream->writeInt(VipLv, Base::Bit32);
		stream->writeInt(LastLogoutTime, Base::Bit32);
		stream->writeInt(LastLoginTime, Base::Bit32);
		stream->writeInt(LastLoginTime, Base::Bit32);*/
		//stream->writeBits(sizeof(SimplePlayerData)<<3,this);
	}

	void ReadData(Base::BitStream* stream)
	{
		//AccountId = stream->readInt(Base::Bit32);
		PlayerId = stream->readInt(Base::Bit32);
		stream->readString(PlayerName, 32);
		/*Sex = stream->readInt(Base::Bit8);
		Level = stream->readInt(Base::Bit8);
		ZoneId = stream->readInt(Base::Bit16);*/
		Gold = stream->readInt(Base::Bit32);
		/*Family = stream->readInt(Base::Bit8);
		FirstClass = stream->readInt(Base::Bit8);
		VipLv = stream->readInt(Base::Bit32);
		LastLogoutTime = stream->readInt(Base::Bit32);
		LastLoginTime = stream->readInt(Base::Bit32);
		LastLoginTime = stream->readInt(Base::Bit32);*/
		//stream->readBits(sizeof(SimplePlayerData)<<3,this);
	}
};

typedef std::tr1::shared_ptr<stItemInfo> ItemInfoRef;
#endif


